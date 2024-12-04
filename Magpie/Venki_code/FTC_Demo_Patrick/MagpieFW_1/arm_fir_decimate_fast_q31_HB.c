
#include "arm_math.h"

/**    
 * @ingroup groupFilters    
 */

/**    
 * @addtogroup FIR_decimate    
 * @{    
 */

/**    
 * @brief Processing function for the Q31 FIR decimator (fast variant) for Cortex-M3 and Cortex-M4.    
 * @param[in] *S points to an instance of the Q31 FIR decimator structure.    
 * @param[in] *pSrc points to the block of input data.    
 * @param[out] *pDst points to the block of output data    
 * @param[in] blockSize number of input samples to process per call.    
 * @return none    
 *    
 * <b>Scaling and Overflow Behavior:</b>    
 *    
 * \par    
 * This function is optimized for speed at the expense of fixed-point precision and overflow protection.    
 * The result of each 1.31 x 1.31 multiplication is truncated to 2.30 format.    
 * These intermediate results are added to a 2.30 accumulator.    
 * Finally, the accumulator is saturated and converted to a 1.31 result.    
 * The fast version has the same overflow behavior as the standard version and provides less precision since it discards the low 32 bits of each multiplication result.    
 * In order to avoid overflows completely the input signal must be scaled down by log2(numTaps) bits (where log2 is read as log to the base 2).    
 *    
 * \par    
 * Refer to the function <code>arm_fir_decimate_q31()</code> for a slower implementation of this function which uses a 64-bit accumulator to provide higher precision.    
 * Both the slow and the fast versions use the same instance structure.    
 * Use the function <code>arm_fir_decimate_init_q31()</code> to initialize the filter structure.    
 */

void arm_fir_decimate_fast_q31_HB( // ***** RESTRICTIONS, Number of taps must be 4N+3, and the center tap must be 0.5. Not checked
  arm_fir_decimate_instance_q31 * S,
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
  q31_t *pState = S->pState;                     /* State pointer */
  q31_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q31_t *pStateCurnt;                            /* Points to the current sample of the state */
  q31_t x0, c0;                                  /* Temporary variables to hold state and coefficient values */
  q31_t *px;                                     /* Temporary pointers for state buffer */
  q31_t *pb;                                     /* Temporary pointers for coefficient buffer */
  q31_t sum0;                                    /* Accumulator */
  uint32_t numTaps = S->numTaps;                 /* Number of taps, for halfband must be ODD */
  // uint32_t i, tapCnt, blkCnt, outBlockSize = blockSize / S->M;  /* Loop counters */
  uint32_t i, tapCnt, blkCnt, outBlockSize = blockSize / 2;  /* Loop counters , s->M must be 2 for HB*/

  uint32_t blkCntN2;
  q31_t x1;
  q31_t acc0, acc1;
  q31_t *px0, *px1;

  uint32_t centerTapIndex = (numTaps-1)/2; // ** numTaps must be of form 4N+3

  /* S->pState buffer contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = S->pState + (numTaps - 1u);

  /* Total number of output samples to be computed */

  blkCnt = outBlockSize / 2; // == blocksize/4 for HB
  blkCntN2 = outBlockSize - (2 * blkCnt); // == blocksize/2 - outBlockSize = blocksize/2 - blocksize/2 = 0

  while(blkCnt > 0u) // blkCnt = out_size/4, but 2 samples at a time = out_size/2 total output samples
  {
    /* Copy decimation factor number of new input samples into the state buffer */
    // i = 2 * S->M;
    i = 4; // for halfband, S->M must be 2


//************* // copy 4 sample of input buff into state buff offset by numTaps-1 ******/
    do
    {
      *pStateCurnt++ = *pSrc++; // pstate gets incremented by 4 every big loop

    } while(--i);

    /* Set accumulator to zero */
    acc0 = 0;
    acc1 = 0;

    /* Initialize state pointer */
    px0 = pState;
    //px1 = pState + S->M;
    px1 = pState + 2; // HB, only 2 allowed


    /* Initialize coeff pointer */
    pb = pCoeffs;

    /* Loop unrolling.  Process 4 taps at a time. */
    tapCnt = numTaps >> 2;

    //  Loop over the number of taps.  Unroll by a factor of 4.
    // Repeat until we've computed numTaps-4 coefficients.
    // orig; 4 taps at a time, but 2 consectutive outputs at a time
    // due to HB, only 2 mults at  time, but 2 consecutive outputs at a time

    while(tapCnt > 0u)
    {
      /* Read the b[numTaps-1] coefficient */
      c0 = *(pb);

      /* Read x[n-numTaps-1] for sample 0 sample 1 */
      x0 = *(px0);
      x1 = *(px1);

      /* Perform the multiply-accumulate */
      acc0 = (q31_t) ((((q63_t) acc0 << 32) + ((q63_t) x0 * c0)) >> 32);
      acc1 = (q31_t) ((((q63_t) acc1 << 32) + ((q63_t) x1 * c0)) >> 32);

      // /* Read the b[numTaps-2] coefficient */
      // c0 = *(pb + 1u);

      // /* Read x[n-numTaps-2]  for sample 0 sample 1  */
      // x0 = *(px0 + 1u);
      // x1 = *(px1 + 1u);

      // /* Perform the multiply-accumulate */
      // acc0 = (q31_t) ((((q63_t) acc0 << 32) + ((q63_t) x0 * c0)) >> 32);
      // acc1 = (q31_t) ((((q63_t) acc1 << 32) + ((q63_t) x1 * c0)) >> 32);

      /* Read the b[numTaps-3] coefficient */
      c0 = *(pb + 2u);

      /* Read x[n-numTaps-3]  for sample 0 sample 1 */
      x0 = *(px0 + 2u);
      x1 = *(px1 + 2u);
      pb += 4u; // ??

      /* Perform the multiply-accumulate */
      acc0 = (q31_t) ((((q63_t) acc0 << 32) + ((q63_t) x0 * c0)) >> 32);
      acc1 = (q31_t) ((((q63_t) acc1 << 32) + ((q63_t) x1 * c0)) >> 32);

      /* Read the b[numTaps-4] coefficient */
      // c0 = *(pb - 1u);

      // /* Read x[n-numTaps-4] for sample 0 sample 1 */
      // x0 = *(px0 + 3u);
      // x1 = *(px1 + 3u);


      // /* Perform the multiply-accumulate */
      // acc0 = (q31_t) ((((q63_t) acc0 << 32) + ((q63_t) x0 * c0)) >> 32);
      // acc1 = (q31_t) ((((q63_t) acc1 << 32) + ((q63_t) x1 * c0)) >> 32);

      /* update state pointers */
      px0 += 4u;
      px1 += 4u;

      /* Decrement the loop counter */
      tapCnt--;
    } // ** if length is multiple of 4, you are finished computing 2 samples here

    // since the filter length is 4N+3, and we have completed the 4N here,
    // we need 3 more, however the middle coeff= 0, so skip it
    /* Read coefficients */
	 c0 = *(pb);
	 /* Fetch 1 state variable */
	 x0 = *(px0);
	 x1 = *(px1);
	 /* Perform the multiply-accumulate */
	 acc0 = (q31_t) ((((q63_t) acc0 << 32) + ((q63_t) x0 * c0)) >> 32);
	 acc1 = (q31_t) ((((q63_t) acc1 << 32) + ((q63_t) x1 * c0)) >> 32);
	 // advance pointers by 2 because the next coeff is 0 (skip)
	 pb+=2;
	 px0+=2;
	 px1+=2;

	 /* Read last coefficient */
	 c0 = *(pb);
	 /* Fetch 1 state variable */
	 x0 = *(px0);
	 x1 = *(px1);
	 /* Perform the multiply-accumulate */
	 acc0 = (q31_t) ((((q63_t) acc0 << 32) + ((q63_t) x0 * c0)) >> 32);
	 acc1 = (q31_t) ((((q63_t) acc1 << 32) + ((q63_t) x1 * c0)) >> 32);


// **** haffband edit, add x * center_coeff here , coeff is assumed to be 0.5
    x0 = *(pState  + centerTapIndex); // data aligned with center tap for 1st sample
    x1 = *(pState + 2 + centerTapIndex); // data aligned with centertap for 2nd sample
    acc0 = (q31_t) acc0 + (x0 >> 2);
    acc1 = (q31_t) acc1 + (x1 >> 2);







    /* Advance the state pointer by the decimation factor       
     * to process the next group of decimation factor number samples */
    //pState = pState + S->M * 2;
    pState = pState + 4; // HB M=2

    /* The result is in the accumulator, store in the destination buffer. */
    /** write the 2 compted outputs **/
    *pDst++ = (q31_t) (acc0 << 1);
    *pDst++ = (q31_t) (acc1 << 1);

    /* Decrement the loop counter */
    blkCnt--;
  } 

  // ************ done entire block **************



  /* Processing is complete.       
   ** Now copy the last numTaps - 1 samples to the start of the state buffer.
   ** This prepares the state buffer for the next function call. */

  /* Points to the start of the state buffer */
  pStateCurnt = S->pState;

  i = (numTaps - 1u) >> 2u;

  /* copy data */
  while(i > 0u)
  {
    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    i--;
  }

  i = (numTaps - 1u) % 0x04u;

  /* copy data */
  while(i > 0u)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    i--;
  }


} // end of function definition

/**    
 * @} end of FIR_decimate group    
 */
