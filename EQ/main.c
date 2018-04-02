//////////////////////////////////////////////////////////////////////////////
// *
// * Predmetni projekat iz predmeta OAiS DSP 2
// * Godina: 2017
// *
// * Zadatak: Ekvalizacija audio signala
// * Autor:
// *                                                                          
// *                                                                          
/////////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "ezdsp5535.h"
#include "ezdsp5535_i2c.h"
#include "aic3204.h"
#include "ezdsp5535_aic3204_dma.h"
#include "ezdsp5535_i2s.h"
#include "ezdsp5535_sar.h"
#include "print_number.h"
#include "math.h"

#include "iir.h"
#include "processing.h"

/* Frekvencija odabiranja */
#define SAMPLE_RATE 8000L

#define PI 3.14159265

/* Niz za smestanje ulaznih i izlaznih odbiraka */
#pragma DATA_ALIGN(sampleBufferL,4)
Int16 sampleBufferL[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(sampleBufferR,4)
Int16 sampleBufferR[AUDIO_IO_SIZE];

Int16 diracBuff[AUDIO_IO_SIZE];
Int16 izlaz[AUDIO_IO_SIZE];

Int16 outputShelvingHP[AUDIO_IO_SIZE];
Int16 outputShelvingLP[AUDIO_IO_SIZE];
Int16 outputPeek[AUDIO_IO_SIZE];

Int16 outputEQ_L[AUDIO_IO_SIZE];
Int16 outputEQ_R[AUDIO_IO_SIZE];

Int16 outputShelvingHP_Coeffs[4];
Int16 outputShelvingLP_Coeffs[4];
Int16 outputPeek_Coeffs[6];
Int16 outputPeek_Coeffs1[6];
Int16 outputPeek_Coeffs2[6];

// baferi za levu stranu
Int16 history_x_LP_L[1] = {0};
Int16 history_y_LP_L[1] = {0};
Int16 history_x_HP_L[1] = {0};
Int16 history_y_HP_L[1] = {0};
Int16 history_x_Peek1_L[2] = {0, 0};
Int16 history_y_Peek1_L[2] = {0, 0};
Int16 history_x_Peek2_L[2] = {0, 0};
Int16 history_y_Peek2_L[2] = {0, 0};

// baferi za desnu stranu
Int16 history_x_LP_R[1] = {0};
Int16 history_y_LP_R[1] = {0};
Int16 history_x_HP_R[1] = {0};
Int16 history_y_HP_R[1] = {0};
Int16 history_x_Peek1_R[2] = {0, 0};
Int16 history_y_Peek1_R[2] = {0, 0};
Int16 history_x_Peek2_R[2] = {0, 0};
Int16 history_y_Peek2_R[2] = {0, 0};

Int16 freqs[6] = {280, 870, 410, 3030, 1290, 4800};
Int16 k[4] = {8192, 8192, 8192, 8192};
int k_i = 0;

//	koeficijenti su normalizovani deljenjem sa 16000

//						280/16000		 410/16000		    1290/16000		 	4800/16000
float alphaBuff[4] = {0.895674708483692, 0.850690591767436, 0.706215304473532, -0.158384439220583};
//					  870/16000			3030/16000
float betaBuff[2] = {0.942203731595556, 0.371772929816393};

Uint16 getKey()
{
    static Uint16 old = NoKey;
    Uint16 key = EZDSP5535_SAR_getKey();
    if (key == old) {
        return NoKey;
    } else {
        old = key;
        return key;
    }
}

void printNewValue() {
	setWritePointerToFirstChar();
	/* Indeks broja k */
	printChar('0' + k_i);
	printChar(' ');
	/* Vrednost broja k */
	if (k[k_i] == 32767) {
		printChar('1');
		printChar('.');
		printChar('0');
	} else {
		printChar('0');
		printChar('.');
		printChar('1' + k[k_i] / 3277);
	}
}

void main(void) {
	int i;

	/* Inicijalizaija razvojne ploce */
	EZDSP5535_init();

	/* Inicijalizacija kontrolera za ocitavanje vrednosti pritisnutog dugmeta*/
	EZDSP5535_SAR_init();

	/* Inicijalizacija LCD kontrolera */
	initPrintNumber();

	printf("\n Ekvalizacija audio signala \n");

	/* Inicijalizacija veze sa AIC3204 kodekom (AD/DA) */
	aic3204_hardware_init();

	/* Inicijalizacija AIC3204 kodeka */
	aic3204_init();

	aic3204_dma_init();

	diracBuff[0] = 16000;

	for (i = 1; i < AUDIO_IO_SIZE; i++) {
		diracBuff[i] = 0;
	}

	/* Postavljanje vrednosti frekvencije odabiranja i pojacanja na kodeku */
	set_sampling_frequency_and_gain(SAMPLE_RATE, 0);

	//-------------------------------------------------------------------------------
	// koeficijenti Shelving-a po PDF-u
	//calculateShelvingCoeff(0.3, outputShelvingLP_Coeffs);
	//calculateShelvingCoeff(-0.3, outputShelvingHP_Coeffs);
	//calculatePeekCoeff(0.7, 0, outputPeek_Coeffs);
	//-------------------------------------------------------------------------------

	// koeficijenti Shelcing-a mog alpha
	calculateShelvingCoeff(alphaBuff[0], outputShelvingLP_Coeffs);
	calculateShelvingCoeff(alphaBuff[3], outputShelvingHP_Coeffs);


	// koeficijenti Peek-a
	calculatePeekCoeff(alphaBuff[1], betaBuff[0], outputPeek_Coeffs1);
	calculatePeekCoeff(alphaBuff[2], betaBuff[1], outputPeek_Coeffs2);


	clearLCD();

	while (1) {
		aic3204_read_block(sampleBufferL, sampleBufferR);

		/* Your code here */
		//-------------------------------------------------------------------------------
		// za testiranje koeficijenata za date alpha i beta
		/*for (i = 0; i < AUDIO_IO_SIZE; i++) {
			outputShelvingHP[i] = shelvingHP(diracBuff[i],
					outputShelvingHP_Coeffs, history_x, history_y, 8192);
		}

		for (i = 0; i < AUDIO_IO_SIZE; i++) {
			outputShelvingLP[i] = shelvingLP(diracBuff[i],
					outputShelvingLP_Coeffs, history_x, history_y, 8192);
		}

		for (i = 0; i < AUDIO_IO_SIZE; i++){
			outputPeek[i] = shelvingPeek(diracBuff[i], outputPeek_Coeffs, history_x, history_y, 8192);
		}*/
		//-------------------------------------------------------------------------------

    	Uint16 key = getKey();

    	switch (key) {
    	    case SW1:
    	        (k_i + 1 == 4) ? k_i = 0 : ++k_i;
    	        printNewValue();
    	        break;
    	    case SW2:
    	        k[k_i] -= 3277;
    	        if (k[k_i] < 0) {
    	            k[k_i] = 32767;
    	        }
    	        printNewValue();
    	        break;
    	}

    	setWritePointerToFirstChar();

		for (i = 0; i < AUDIO_IO_SIZE; i++){
			outputEQ_L[i] = shelvingLP(sampleBufferL[i], outputShelvingLP_Coeffs, history_x_LP_L, history_y_LP_L, k[0]);
			outputEQ_L[i] = shelvingPeek(outputEQ_L[i], outputPeek_Coeffs1, history_x_Peek1_L, history_y_Peek1_L, k[1]);
			outputEQ_L[i] = shelvingPeek(outputEQ_L[i], outputPeek_Coeffs2, history_x_Peek2_L, history_y_Peek2_L, k[2]);
			outputEQ_L[i] = shelvingHP(outputEQ_L[i], outputShelvingHP_Coeffs, history_x_HP_L, history_y_HP_L, k[3]);

			outputEQ_R[i] = shelvingLP(sampleBufferR[i], outputShelvingLP_Coeffs, history_x_LP_R, history_y_LP_R, k[0]);
			outputEQ_R[i] = shelvingPeek(outputEQ_R[i], outputPeek_Coeffs1, history_x_Peek1_R, history_y_Peek1_R, k[1]);
			outputEQ_R[i] = shelvingPeek(outputEQ_R[i], outputPeek_Coeffs2, history_x_Peek2_R, history_y_Peek2_R, k[2]);
			outputEQ_R[i] = shelvingHP(outputEQ_R[i], outputShelvingHP_Coeffs, history_x_HP_R, history_y_HP_R, k[3]);
		}

		aic3204_write_block(sampleBufferR, sampleBufferR);
	}

	/* Prekid veze sa AIC3204 kodekom */
	aic3204_disable();

	printf("\n***Kraj programa***\n");
	SW_BREAKPOINT
	;
}
