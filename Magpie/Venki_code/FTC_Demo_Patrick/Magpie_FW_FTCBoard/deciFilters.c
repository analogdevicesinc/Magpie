
#include "arm_math.h"

void decimate_16x_iirHB( // takes 1.7ms, new design (7/28/24) with 50dB
	q31_t * pSrc,
	q31_t * pDst,
	uint32_t len) // note len is the final decimated output length, = input length/8
{
	uint32_t k;
	static q31_t state_stg0_zm1=0; // 1st decimation stg
	static q31_t state_stg0_zm0=0;

	static q31_t state_stg1_zm1=0; // 2nd decimation stg
	static q31_t state_stg1_zm0=0;

	// the non-delayed Allpass state-var paths are designated with _A_
	// the delayed Allpass state-var paths are designated with _B_

	static q31_t state_stg2_A_zm1=0;
	static q31_t state_stg2_A_zm0=0;
	static q31_t state_stg2_B_zm1=0;
	static q31_t state_stg2_B_zm0=0;


	static q31_t state_stg3_A_zm2=0;
	static q31_t state_stg3_A_zm1=0;
	static q31_t state_stg3_A_zm0=0;
	static q31_t state_stg3_B_zm1=0;
	static q31_t state_stg3_B_zm0=0;



// outputs
	static q31_t deci_stg0_out0 = 0;
	static q31_t deci_stg0_out1 = 0;
	static q31_t deci_stg0_out2 = 0;
	static q31_t deci_stg0_out3 = 0;
	static q31_t deci_stg0_out4 = 0;
	static q31_t deci_stg0_out5 = 0;
	static q31_t deci_stg0_out6 = 0;
	static q31_t deci_stg0_out7 = 0;

	static q31_t deci_stg1_out0 = 0;
	static q31_t deci_stg1_out1 = 0;
	static q31_t deci_stg1_out2 = 0;
	static q31_t deci_stg1_out3 = 0;

	static q31_t deci_stg2_out0 = 0;
	static q31_t deci_stg2_out1 = 0;

	static q31_t coeff_d2_n0_A_stg3 = 0x0D9C6C2D;
	static q31_t coeff_d1_n1_A_stg3 = 0x77233802;
	static q31_t coeff_d1_n1_B_stg3 = 0x385BACD3;
	static q31_t mult_temp1;
	static q31_t mult_temp2;
	static q31_t allpass_A,allpass_B;

	static q31_t in0,in1,in2,in3,in4,in5,in6,in7;
	static q31_t in8,in9,in10,in11,in12,in13,in14,in15;

	static q31_t deci_out;



	k = len;
	//j0 = 1; // allpass starts with odd inputs and skips by 2



	//pSrc+=1; // start looking st position 1 in the array (newer)
	while(k > 0) { // each loop produces 2 outputs at 2X rate, so use DMALEN/8

		// input is divided by 16, then gain by 2*2*2, then gain by (1 + 1/2), 1/2*3/2 = 3/4 = -2.49 dB
		in0=(*pSrc++ >> 5);
		in1=(*pSrc++ >> 5);
		in2=(*pSrc++ >> 5);
		in3=(*pSrc++ >> 5);
		in4=(*pSrc++ >> 5);
		in5=(*pSrc++ >> 5);
		in6=(*pSrc++ >> 5);
		in7=(*pSrc++ >> 5);
		in8=(*pSrc++ >> 5);
		in9=(*pSrc++ >> 5);
		in10=(*pSrc++ >> 5);
		in11=(*pSrc++ >> 5);
		in12=(*pSrc++ >> 5);
		in13=(*pSrc++ >> 5);
		in14=(*pSrc++ >> 5);
		in15=(*pSrc++ >> 5);


		// ***** 1st filter ****
		// 1st stage comes from 3rd order elliptc prototype, 1st-order in non-delayed branch
		// ***** 1st 2:1 decimator, gain = 2 ***

		// 1st shift the zm1 to zm0 based on the current zm1 value
		state_stg0_zm1 = state_stg0_zm0;
		state_stg0_zm0 = in1 - (state_stg0_zm1 >> 2) - (state_stg0_zm1 >> 4) - (state_stg0_zm1 >> 5); // scale input by 1/2 so the output does not need to be scaled by 1/2
		deci_stg0_out0 = state_stg0_zm1 + (state_stg0_zm0 >> 2) + (state_stg0_zm0 >> 4) + (state_stg0_zm0 >> 5 )+ in0;

		state_stg0_zm1 = state_stg0_zm0;
		state_stg0_zm0 = in3 - (state_stg0_zm1 >> 2) - (state_stg0_zm1 >> 4) - (state_stg0_zm1 >> 5); // scale input by 1/2 so the output does not need to be scaled by 1/2
		deci_stg0_out1 = state_stg0_zm1 + (state_stg0_zm0 >> 2) + (state_stg0_zm0 >> 4)+ (state_stg0_zm0 >> 5) + in2;


		state_stg0_zm1 = state_stg0_zm0;
		state_stg0_zm0 = in5 - (state_stg0_zm1 >> 2) - (state_stg0_zm1 >> 4)- (state_stg0_zm1 >> 5); // scale input by 1/2 so the output does not need to be scaled by 1/2
		deci_stg0_out2 = state_stg0_zm1 + (state_stg0_zm0 >> 2) + (state_stg0_zm0 >> 4)+ (state_stg0_zm0 >> 5)  + in4;


		state_stg0_zm1 = state_stg0_zm0;
		state_stg0_zm0 = in7 - (state_stg0_zm1 >> 2) - (state_stg0_zm1 >> 4)- (state_stg0_zm1 >> 5); // scale input by 1/2 so the output does not need to be scaled by 1/2
		deci_stg0_out3 = state_stg0_zm1 + (state_stg0_zm0 >> 2) + (state_stg0_zm0 >> 4)+ (state_stg0_zm0 >> 5)  + in6;

		state_stg0_zm1 = state_stg0_zm0;
		state_stg0_zm0 = in9 - (state_stg0_zm1 >> 2) - (state_stg0_zm1 >> 4)- (state_stg0_zm1 >> 5); // scale input by 1/2 so the output does not need to be scaled by 1/2
		deci_stg0_out4 = state_stg0_zm1 + (state_stg0_zm0 >> 2) + (state_stg0_zm0 >> 4)+ (state_stg0_zm0 >> 5)  + in8;

		state_stg0_zm1 = state_stg0_zm0;
		state_stg0_zm0 = in11 - (state_stg0_zm1 >> 2) - (state_stg0_zm1 >> 4)- (state_stg0_zm1 >> 5); // scale input by 1/2 so the output does not need to be scaled by 1/2
		deci_stg0_out5 = state_stg0_zm1 + (state_stg0_zm0 >> 2) + (state_stg0_zm0 >> 4)+ (state_stg0_zm0 >> 5)  + in10;

		state_stg0_zm1 = state_stg0_zm0;
		state_stg0_zm0 = in13 - (state_stg0_zm1 >> 2) - (state_stg0_zm1 >> 4)- (state_stg0_zm1 >> 5); // scale input by 1/2 so the output does not need to be scaled by 1/2
		deci_stg0_out6 = state_stg0_zm1 + (state_stg0_zm0 >> 2) + (state_stg0_zm0 >> 4)+ (state_stg0_zm0 >> 5)  + in12;

		state_stg0_zm1 = state_stg0_zm0;
		state_stg0_zm0 = in15 - (state_stg0_zm1 >> 2) - (state_stg0_zm1 >> 4)- (state_stg0_zm1 >> 5); // scale input by 1/2 so the output does not need to be scaled by 1/2
		deci_stg0_out7 = state_stg0_zm1 + (state_stg0_zm0 >> 2) + (state_stg0_zm0 >> 4)+ (state_stg0_zm0 >> 5)  + in14;


		//***** 2nd filter, 8 in, 4 out, same structires as 1st filter

		// ***** 2nd 2:1 decimator, same structure as 1st filter ***
		state_stg1_zm1 = state_stg1_zm0;
		state_stg1_zm0 = deci_stg0_out1 - (state_stg1_zm1 >> 2) - (state_stg1_zm1 >> 4) - (state_stg1_zm1 >> 5); // scale input by 1/2 so the output does not need to be scaled by 1/2
		deci_stg1_out0 = state_stg1_zm1 + (state_stg1_zm0 >> 2) + (state_stg1_zm0 >> 4) + (state_stg1_zm0 >> 5 )+ deci_stg0_out0;

		state_stg1_zm1 = state_stg1_zm0;
		state_stg1_zm0 = deci_stg0_out3 - (state_stg1_zm1 >> 2) - (state_stg1_zm1 >> 4) - (state_stg1_zm1 >> 5); // scale input by 1/2 so the output does not need to be scaled by 1/2
		deci_stg1_out1 = state_stg1_zm1 + (state_stg1_zm0 >> 2) + (state_stg1_zm0 >> 4) + (state_stg1_zm0 >> 5 )+ deci_stg0_out2;

		state_stg1_zm1 = state_stg1_zm0;
		state_stg1_zm0 = deci_stg0_out5 - (state_stg1_zm1 >> 2) - (state_stg1_zm1 >> 4) - (state_stg1_zm1 >> 5); // scale input by 1/2 so the output does not need to be scaled by 1/2
		deci_stg1_out2 = state_stg1_zm1 + (state_stg1_zm0 >> 2) + (state_stg1_zm0 >> 4) + (state_stg1_zm0 >> 5 )+ deci_stg0_out4;

		state_stg1_zm1 = state_stg1_zm0;
		state_stg1_zm0 = deci_stg0_out7 - (state_stg1_zm1 >> 2) - (state_stg1_zm1 >> 4) - (state_stg1_zm1 >> 5); // scale input by 1/2 so the output does not need to be scaled by 1/2
		deci_stg1_out3 = state_stg1_zm1 + (state_stg1_zm0 >> 2) + (state_stg1_zm0 >> 4) + (state_stg1_zm0 >> 5 )+ deci_stg0_out6;


		// ***** 3rd filter ***
		// 1st order allpass stg 1, 4 in, 2 out
		// 1st one is non-delayed
		// gain = 2
		state_stg2_A_zm1 = state_stg2_A_zm0;
		state_stg2_A_zm0 = deci_stg1_out1 - (state_stg2_A_zm1 >> 3);
		allpass_A = state_stg2_A_zm1 + (state_stg2_A_zm0 >> 3);
			// 1st order allpass for the delayed branch
		state_stg2_B_zm1 = state_stg2_B_zm0;
		state_stg2_B_zm0 = deci_stg1_out0  - (state_stg2_B_zm1 >> 1) - (state_stg2_B_zm1 >> 4);
		allpass_B =  state_stg2_B_zm1 + (state_stg2_B_zm0 >> 1) + (state_stg2_B_zm0 >> 4);
		deci_stg2_out0 = allpass_B+allpass_A;

		state_stg2_A_zm1 = state_stg2_A_zm0;
		state_stg2_A_zm0 = deci_stg1_out3 - (state_stg2_A_zm1 >> 3);
		allpass_A = state_stg2_A_zm1 + (state_stg2_A_zm0 >> 3);
			// 1st order allpass for the delayed branch
		state_stg2_B_zm1 = state_stg2_B_zm0;
		state_stg2_B_zm0 = deci_stg1_out2 -(state_stg2_B_zm1 >> 1) - (state_stg2_B_zm1 >> 4);
		allpass_B = state_stg2_B_zm1 + (state_stg2_B_zm0 >> 1) + (state_stg2_B_zm0 >> 4);
		deci_stg2_out1 = allpass_B+allpass_A;


		// at this point I've processed 8 inputs from the dma array and produced 2 outputs
		// now for the final stg, which has a 2nd-order allpass in the
		//un-delayed branch and a 1st-order allpass in the delayed branch (7th order elliptic)

		// ***** 4th 2:1 decimator, 2 in, 1 out ***
		// gain 2
			// 2nd order allpass
		state_stg3_A_zm2 = state_stg3_A_zm1; // non-delayed branch
		state_stg3_A_zm1 = state_stg3_A_zm0;
		// faster to shift mult result right by 32 and then left by 1
		mult_temp1 = ((q31_t)  ( ((q63_t) state_stg3_A_zm1 * coeff_d1_n1_A_stg3) >> 32)) << 1;
		mult_temp2 = ((q31_t)  ( ((q63_t) state_stg3_A_zm2 * coeff_d2_n0_A_stg3) >> 32)) << 1;
		state_stg3_A_zm0 = deci_stg2_out1 - mult_temp1 - mult_temp2;
		mult_temp2 = ((q31_t)  ( ((q63_t) state_stg3_A_zm0 * coeff_d2_n0_A_stg3) >> 32)) << 1;
		allpass_A = mult_temp1 + mult_temp2 + state_stg3_A_zm2;

			// 1st order allpass for the delayed branch
		state_stg3_B_zm1 = state_stg3_B_zm0;
		mult_temp1 = ((q31_t)  ( ((q63_t) state_stg3_B_zm1 * coeff_d1_n1_B_stg3) >> 32)) << 1;
		state_stg3_B_zm0 = deci_stg2_out0 - mult_temp1;
		mult_temp1 = ((q31_t)  ( ((q63_t) state_stg3_B_zm0 * coeff_d1_n1_B_stg3) >> 32)) << 1;
		allpass_B = mult_temp1 + state_stg3_B_zm1;

		deci_out = allpass_B+allpass_A; // this has a gain of 1/4 from the input
		*pDst++ =  (deci_out >> 1) + deci_out; // 20*log10((1/4)*(2 + 1)) = -2.49 dB


		//debugBuff[j1++] = allpass_B+allpass_A;





		k--;

	}
}





void decimate_8x_iirHB( // takes 1.7ms, new design (7/28/24) with 50dB
	q31_t * pSrc,
	q31_t * pDst,
	uint32_t len) // note len is the final decimated output length, = input length/8
{
	uint32_t k;
	static q31_t state_stg0_zm1=0; // 1st number os decimation stg
	static q31_t state_stg0_zm0=0;

	// the non-delayed Allpass state-var paths are designated with _A_
	// the delayed Allpass state-var paths are designated with _B_

	static q31_t state_stg1_A_zm1=0;
	static q31_t state_stg1_A_zm0=0;
	static q31_t state_stg1_B_zm1=0;
	static q31_t state_stg1_B_zm0=0;

	static q31_t state_stg2_A_zm2=0;
	static q31_t state_stg2_A_zm1=0;
	static q31_t state_stg2_A_zm0=0;
	static q31_t state_stg2_B_zm1=0;
	static q31_t state_stg2_B_zm0=0;

	static q31_t deci_stg0_out0 = 0;
	static q31_t deci_stg0_out1 = 0;
	static q31_t deci_stg0_out2 = 0;
	static q31_t deci_stg0_out3 = 0;

	static q31_t deci_stg1_out0 = 0;
	static q31_t deci_stg1_out1 = 0;

	static q31_t coeff_d2_n0_A_stg2 = 0x0D9C6C2D;
	static q31_t coeff_d1_n1_A_stg2 = 0x77233802;
	static q31_t coeff_d1_n1_B_stg2 = 0x385BACD3;
	static q31_t mult_temp1;
	static q31_t mult_temp2;
	static q31_t allpass_A,allpass_B;

	static q31_t in0,in1,in2,in3,in4,in5,in6,in7;
	static q31_t deci_out;



	k = len;
	//j0 = 1; // allpass starts with odd inputs and skips by 2



	//pSrc+=1; // start looking st position 1 in the array (newer)
	while(k > 0) { // each loop produces 2 outputs at 2X rate, so use DMALEN/8

		// input is divided by 16, then gain by 2*2*2, then gain by (1 + 1/2), 1/2*3/2 = 3/4 = -2.49 dB
		in0=(*pSrc++ >> 4);
		in1=(*pSrc++ >> 4);
		in2=(*pSrc++ >> 4);
		in3=(*pSrc++ >> 4);
		in4=(*pSrc++ >> 4);
		in5=(*pSrc++ >> 4);
		in6=(*pSrc++ >> 4);
		in7=(*pSrc++ >> 4);


		// 1st stage comes from 3rd order elliptc prototype, 1st-order in non-delayed branch
		// ***** 1st 2:1 decimator, gain = 2 ***

		// 1st shift the zm1 to zm0 based on the current zm1 value
		state_stg0_zm1 = state_stg0_zm0;
		state_stg0_zm0 = in1 - (state_stg0_zm1 >> 2) - (state_stg0_zm1 >> 4) - (state_stg0_zm1 >> 5); // scale input by 1/2 so the output does not need to be scaled by 1/2
		deci_stg0_out0 = state_stg0_zm1 + (state_stg0_zm0 >> 2) + (state_stg0_zm0 >> 4) + (state_stg0_zm0 >> 5 )+ in0;

		state_stg0_zm1 = state_stg0_zm0;
		state_stg0_zm0 = in3 - (state_stg0_zm1 >> 2) - (state_stg0_zm1 >> 4) - (state_stg0_zm1 >> 5); // scale input by 1/2 so the output does not need to be scaled by 1/2
		deci_stg0_out1 = state_stg0_zm1 + (state_stg0_zm0 >> 2) + (state_stg0_zm0 >> 4)+ (state_stg0_zm0 >> 5) + in2;


		state_stg0_zm1 = state_stg0_zm0;
		state_stg0_zm0 = in5 - (state_stg0_zm1 >> 2) - (state_stg0_zm1 >> 4)- (state_stg0_zm1 >> 5); // scale input by 1/2 so the output does not need to be scaled by 1/2
		deci_stg0_out2 = state_stg0_zm1 + (state_stg0_zm0 >> 2) + (state_stg0_zm0 >> 4)+ (state_stg0_zm0 >> 5)  + in4;


		state_stg0_zm1 = state_stg0_zm0;
		state_stg0_zm0 = in7 - (state_stg0_zm1 >> 2) - (state_stg0_zm1 >> 4)- (state_stg0_zm1 >> 5); // scale input by 1/2 so the output does not need to be scaled by 1/2
		deci_stg0_out3 = state_stg0_zm1 + (state_stg0_zm0 >> 2) + (state_stg0_zm0 >> 4)+ (state_stg0_zm0 >> 5)  + in6;



		// at this point I've processed 8 inputs from the dma array and produced 4 outputs
		// now take 4 outputs and use the same filter as above to produce 2 outputs
		// this stage has a 1st-order allpass in both delayed and non-delayed branches (from 5th-order elliptic prototype)

		// ***** 2nd 2:1 decimator ***

		// instance 1
		// 1st order allpass stg 1, 2 instances to produce 2 outputs
		// 1st one is non-delayed
		// gain = 2
		state_stg1_A_zm1 = state_stg1_A_zm0;
		state_stg1_A_zm0 = deci_stg0_out1 - (state_stg1_A_zm1 >> 3);
		allpass_A = state_stg1_A_zm1 + (state_stg1_A_zm0 >> 3);
			// 1st order allpass for the delayed branch
		state_stg1_B_zm1 = state_stg1_B_zm0;
		state_stg1_B_zm0 = deci_stg0_out0  - (state_stg1_B_zm1 >> 1) - (state_stg1_B_zm1 >> 4);
		allpass_B =  state_stg1_B_zm1 + (state_stg1_B_zm0 >> 1) + (state_stg1_B_zm0 >> 4);
		deci_stg1_out0 = allpass_B+allpass_A;

		// instance 2
		state_stg1_A_zm1 = state_stg1_A_zm0;
		state_stg1_A_zm0 = deci_stg0_out3 - (state_stg1_A_zm1 >> 3);
		allpass_A = state_stg1_A_zm1 + (state_stg1_A_zm0 >> 3);
			// 1st order allpass for the delayed branch
		state_stg1_B_zm1 = state_stg1_B_zm0;
		state_stg1_B_zm0 = deci_stg0_out2 -(state_stg1_B_zm1 >> 1) - (state_stg1_B_zm1 >> 4);
		allpass_B = state_stg1_B_zm1 + (state_stg1_B_zm0 >> 1) + (state_stg1_B_zm0 >> 4);
		deci_stg1_out1 = allpass_B+allpass_A;


		// at this point I've processed 8 inputs from the dma array and produced 2 outputs
		// now for the final stg, which has a 2nd-order allpass in the
		//un-delayed branch and a 1st-order allpass in the delayed branch (7th order elliptic)

		// ***** 3rd 2:1 decimator ***
		// gain 2
			// 2nd order allpass
		state_stg2_A_zm2 = state_stg2_A_zm1; // non-delayed branch
		state_stg2_A_zm1 = state_stg2_A_zm0;
		// faster to shift mult result right by 32 and then left by 1
		mult_temp1 = ((q31_t)  ( ((q63_t) state_stg2_A_zm1 * coeff_d1_n1_A_stg2) >> 32)) << 1;
		mult_temp2 = ((q31_t)  ( ((q63_t) state_stg2_A_zm2 * coeff_d2_n0_A_stg2) >> 32)) << 1;
		state_stg2_A_zm0 = deci_stg1_out1 - mult_temp1 - mult_temp2;
		mult_temp2 = ((q31_t)  ( ((q63_t) state_stg2_A_zm0 * coeff_d2_n0_A_stg2) >> 32)) << 1;
		allpass_A = mult_temp1 + mult_temp2 + state_stg2_A_zm2;

			// 1st order allpass for the delayed branch
		state_stg2_B_zm1 = state_stg2_B_zm0;
		mult_temp1 = ((q31_t)  ( ((q63_t) state_stg2_B_zm1 * coeff_d1_n1_B_stg2) >> 32)) << 1;
		state_stg2_B_zm0 = deci_stg1_out0 - mult_temp1;
		mult_temp1 = ((q31_t)  ( ((q63_t) state_stg2_B_zm0 * coeff_d1_n1_B_stg2) >> 32)) << 1;
		allpass_B = mult_temp1 + state_stg2_B_zm1;

		deci_out = allpass_B+allpass_A; // this has a gain of 1/4 from the input
		*pDst++ =  (deci_out >> 1) + deci_out; // 20*log10((1/4)*(2 + 1)) = -2.49 dB
		//debugBuff[j1++] = allpass_B+allpass_A;





		k--;

	}
}





void decimate_4x_iirHB( // takes 1.6ms, new design (7/28/24) with 50dB
	q31_t * pSrc,
	q31_t * pDst,
	uint32_t len) // note len is the final decimated output length, = input length/8
{
	uint32_t k;

	// the non-delayed Allpass state-var paths are designated with _A_
	// the delayed Allpass state-var paths are designated with _B_

	static q31_t state_stg1_A_zm1=0;
	static q31_t state_stg1_A_zm0=0;
	static q31_t state_stg1_B_zm1=0;
	static q31_t state_stg1_B_zm0=0;

	static q31_t state_stg2_A_zm2=0;
	static q31_t state_stg2_A_zm1=0;
	static q31_t state_stg2_A_zm0=0;
	static q31_t state_stg2_B_zm1=0;
	static q31_t state_stg2_B_zm0=0;


	static q31_t deci_stg1_out0 = 0;
	static q31_t deci_stg1_out1 = 0;
	static q31_t coeff_d2_n0_A_stg2 = 0x0D9C6C2D;
	static q31_t coeff_d1_n1_A_stg2 = 0x77233802;
	static q31_t coeff_d1_n1_B_stg2 = 0x385BACD3;

	static q31_t mult_temp1;
	static q31_t mult_temp2;
	static q31_t allpass_A,allpass_B;

	static q31_t in0,in1,in2,in3;
	static q31_t deci_out;



	k = len;
	//j0 = 1; // allpass starts with odd inputs and skips by 2



	//pSrc+=1; // start looking st position 1 in the array (newer)
	while(k > 0) { // each loop produces 2 outputs at 2X rate, so use DMALEN/8

		// input/8 *2*2*(3/2)
		in0=(*pSrc++ >> 3);
		in1=(*pSrc++ >> 3);
		in2=(*pSrc++ >> 3);
		in3=(*pSrc++ >> 3);

		// ***** 1st 2:1 decimator ***

		// instance 1
		// 1st order allpass stg 1, 2 instances to produce 2 outputs
		// 1st one is non-delayed
		// gain 2
		state_stg1_A_zm1 = state_stg1_A_zm0;
		state_stg1_A_zm0 = in1 - (state_stg1_A_zm1 >> 3);
		allpass_A = state_stg1_A_zm1 + (state_stg1_A_zm0 >> 3);
			// 1st order allpass for the delayed branch
		state_stg1_B_zm1 = state_stg1_B_zm0;
		state_stg1_B_zm0 = in0 - (state_stg1_B_zm1 >> 1) - (state_stg1_B_zm1 >> 4);
		allpass_B =  state_stg1_B_zm1 + (state_stg1_B_zm0 >> 1) + (state_stg1_B_zm0 >> 4);
		deci_stg1_out0 = allpass_B+allpass_A;

		// instance 2
		state_stg1_A_zm1 = state_stg1_A_zm0;
		state_stg1_A_zm0 = in3 - (state_stg1_A_zm1 >> 3);
		allpass_A = state_stg1_A_zm1 + (state_stg1_A_zm0 >> 3);
			// 1st order allpass for the delayed branch
		state_stg1_B_zm1 = state_stg1_B_zm0;
		state_stg1_B_zm0 = in2 -(state_stg1_B_zm1 >> 1) - (state_stg1_B_zm1 >> 4);
		allpass_B = state_stg1_B_zm1 + (state_stg1_B_zm0 >> 1) + (state_stg1_B_zm0 >> 4);
		deci_stg1_out1 = allpass_B+allpass_A;


		// at this point I've processed 4 inputs from the dma array and produced 2 outputs
		// now for the final stg, which has a 2nd-order allpass in the
		//un-delayed branch and a 1st-order allpass in the delayed branch (7th order elliptic)

		// ***** 3rd 2:1 decimator, gain 2 ***



			// 2nd order allpass
		state_stg2_A_zm2 = state_stg2_A_zm1; // non-delayed branch
		state_stg2_A_zm1 = state_stg2_A_zm0;
		mult_temp1 = ((q31_t)  ( ((q63_t) state_stg2_A_zm1 * coeff_d1_n1_A_stg2) >> 32)) << 1;
		mult_temp2 = ((q31_t)  ( ((q63_t) state_stg2_A_zm2 * coeff_d2_n0_A_stg2) >> 32)) << 1;
		state_stg2_A_zm0 = deci_stg1_out1 - mult_temp1 - mult_temp2;
		mult_temp2 = ((q31_t)  ( ((q63_t) state_stg2_A_zm0 * coeff_d2_n0_A_stg2) >> 32)) << 1;
		allpass_A = mult_temp1 + mult_temp2 + state_stg2_A_zm2;

			// 1st order allpass for the delayed branch
		state_stg2_B_zm1 = state_stg2_B_zm0;
		mult_temp1 = ((q31_t)  ( ((q63_t) state_stg2_B_zm1 * coeff_d1_n1_B_stg2) >> 32)) << 1;
		state_stg2_B_zm0 = deci_stg1_out0 - mult_temp1;
		mult_temp1 = ((q31_t)  ( ((q63_t) state_stg2_B_zm0 * coeff_d1_n1_B_stg2) >> 32)) << 1;
		allpass_B = mult_temp1 + state_stg2_B_zm1;

		deci_out = allpass_B+allpass_A; // this has a gain of 1/2 from the input
		*pDst++ =  (deci_out >> 1) + deci_out; //  -2.49 dB
		//debugBuff[j1++] = allpass_B+allpass_A;





		k--;

	}
}




void decimate_2x_iirHB( // takes 3.2ms, new design (7/28/24) with 50dB
	q31_t * pSrc,
	q31_t * pDst,
	uint32_t len) // note len is the final decimated output length, = input length/8
{
	uint32_t k;

	// the non-delayed Allpass state-var paths are designated with _A_
	// the delayed Allpass state-var paths are designated with _B_



	static q31_t state_stg2_A_zm2=0;
	static q31_t state_stg2_A_zm1=0;
	static q31_t state_stg2_A_zm0=0;
	static q31_t state_stg2_B_zm1=0;
	static q31_t state_stg2_B_zm0=0;

	static q31_t coeff_d2_n0_A_stg2 = 0x0D9C6C2D;
	static q31_t coeff_d1_n1_A_stg2 = 0x77233802;
	static q31_t coeff_d1_n1_B_stg2 = 0x385BACD3;

	static q31_t mult_temp1;
	static q31_t mult_temp2;
	static q31_t allpass_A,allpass_B;

	static q31_t in0,in1;
	static q31_t deci_out;

	k = len;
	//j0 = 1; // allpass starts with odd inputs and skips by 2



	//pSrc+=1; // start looking st position 1 in the array (newer)
	while(k > 0) { // each loop produces 2 outputs at 2X rate, so use DMALEN/8

		// input /4 *2*(3/2)
		in0=(*pSrc++ >> 2);
		in1=(*pSrc++ >> 2);

			// 2nd order allpass
		state_stg2_A_zm2 = state_stg2_A_zm1; // non-delayed branch
		state_stg2_A_zm1 = state_stg2_A_zm0;
		mult_temp1 = ((q31_t)  ( ((q63_t) state_stg2_A_zm1 * coeff_d1_n1_A_stg2) >> 32)) << 1;
		mult_temp2 = ((q31_t)  ( ((q63_t) state_stg2_A_zm2 * coeff_d2_n0_A_stg2) >> 32)) << 1;
		state_stg2_A_zm0 = in1 - mult_temp1 - mult_temp2;
		mult_temp2 = ((q31_t)  ( ((q63_t) state_stg2_A_zm0 * coeff_d2_n0_A_stg2) >> 32)) << 1;
		allpass_A = mult_temp1 + mult_temp2 + state_stg2_A_zm2;

			// 1st order allpass for the delayed branch
		state_stg2_B_zm1 = state_stg2_B_zm0;
		mult_temp1 = ((q31_t)  ( ((q63_t) state_stg2_B_zm1 * coeff_d1_n1_B_stg2) >> 32)) << 1;
		state_stg2_B_zm0 = in0 - mult_temp1;
		mult_temp1 = ((q31_t)  ( ((q63_t) state_stg2_B_zm0 * coeff_d1_n1_B_stg2) >> 32)) << 1;
		allpass_B = mult_temp1 + state_stg2_B_zm1;

		deci_out = allpass_B+allpass_A; // this has a gain of 1/2
		*pDst++ =  (deci_out >> 1) + deci_out; // -2.49 dB
		//debugBuff[j1++] = allpass_B+allpass_A;





		k--;

	}
}




