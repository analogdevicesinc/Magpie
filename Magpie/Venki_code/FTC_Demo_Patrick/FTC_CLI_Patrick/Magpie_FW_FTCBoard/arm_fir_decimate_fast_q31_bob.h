
#include "arm_math.h"
//#define UNALIGNED_SUPPORT_DISABLE



void arm_fir_decimate_fast_q31_bob(
  const arm_fir_decimate_instance_q31 * S,
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize);




