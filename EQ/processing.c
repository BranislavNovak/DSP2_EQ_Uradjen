#include "processing.h"
#include "iir.h"

void calculateShelvingCoeff(float c_alpha, Int16* output) {
	/* Your code here */
	Int16 k1, k2;
	k1 = c_alpha * 32767;
	k2  = -32768*(c_alpha);

	if(output[0] > 32767) {
		output[0] = 32767;
	}
	else {
		output[0] = k1;
	}

	output[1] = -32768;
	output[2] = 32767;

	if(output[3]  < -32768){
		output[3] = -32768;
	}
	else {
		output[3] = k2;
	}

	/*output[0] = c_alpha * 32767;
	output[1] = -32768;
	output[2] = 32767;
	output[3] = -c_alpha * 32768;

	// klipovanje pozitivnog dela
	if (output[0] > 32767) output[0] = 32767;

	// klipovanje negativnog dela
	if (output[3] < -32768) output[3] = -32768;
	*/
}

void calculatePeekCoeff(float c_alpha, float c_beta, Int16* output) {
	/* Your code here */
	output[0]=c_alpha*32767;
	output[1]=-c_beta*(1+c_alpha)*16384;
	output[2]=32767;
	output[3]=32767;
	output[4]=-c_beta*(1+c_alpha)*16384;
	output[5]=c_alpha*32767;
}

Int16 shelvingHP(Int16 input, Int16* coeff, Int16* x_history, Int16* y_history, Int16 k)
{

	Int16 input_a1;
	Int32 HPoutput;																	// 32-bitni kako bi se klipovanje uspesno izvrsilo

	input_a1 = first_order_IIR(input, coeff, x_history, y_history);

	HPoutput = _smpy((input>>1) + (input_a1>>1), k) + (Int32)((input>>1) - (input_a1>>1));			// klipovanje vrednosti, gde kastom na 32 dozvoljavam da predjem max vrednost Int16 pa ga posle klipujem

	if(HPoutput > 32767) {
		HPoutput = 32767;
	}

	if(HPoutput < -32768) {
		HPoutput = -32768;
	}

	return (Int16)HPoutput;
}

Int16 shelvingLP(Int16 input, Int16* coeff, Int16* x_history, Int16* y_history, Int16 k)
{
	Int16 input_a1;
	Int32 LP_output;																	// 32-bitni kako bi se klipovanje uspesno izvrsilo

	input_a1 = first_order_IIR(input, coeff, x_history, y_history);

	LP_output = _smpy((input>>1) - (input_a1>>1), k) + (Int32)((input>>1) + (input_a1>>1));		// klipovanje vrednosti, gde kastom na 32 dozvoljavam da predjem max vrednost Int16 pa ga posle klipujem

	if(LP_output > 32767) {
		LP_output = 32767;
	}

	if(LP_output < -32768) {
		LP_output = -32768;
	}

	return (Int16)LP_output;
}

Int16 shelvingPeek(Int16 input, Int16* coeff, Int16* x_history, Int16* y_history, Int16 k)
{
	Int16 input_a2;
	Int32 Peek_output;																	// 32-bitni kako bi se klipovanje uspesno izvrsilo

	input_a2 = second_order_IIR(input, coeff, x_history, y_history);

	Peek_output = _smpy((input>>1) - (input_a2>>1), k) + (Int32)((input>>1) + (input_a2>>1));		// klipovanje vrednosti, gde kastom na 32 dozvoljavam da predjem max vrednost Int16 pa ga posle klipujem

	if(Peek_output > 32767) {
		Peek_output = 32767;
	}

	if(Peek_output < -32768) {
		Peek_output = -32768;
	}

	return (Int16)Peek_output;
}
