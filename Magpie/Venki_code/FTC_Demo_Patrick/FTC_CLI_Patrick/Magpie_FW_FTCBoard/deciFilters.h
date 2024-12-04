
void decimate_16x_iirHB( // takes 1.7ms, new design (7/28/24) with 50dB
	q31_t * pSrc,
	q31_t * pDst,
	uint32_t len); // note len is the final decimated output length

void decimate_8x_iirHB( // takes 1.7ms, new design (7/28/24) with 50dB
	q31_t * pSrc,
	q31_t * pDst,
	uint32_t len); 


void decimate_4x_iirHB( // takes 1.7ms, new design (7/28/24) with 50dB
	q31_t * pSrc,
	q31_t * pDst,
	uint32_t len); 


void decimate_2x_iirHB( // takes 1.7ms, new design (7/28/24) with 50dB
	q31_t * pSrc,
	q31_t * pDst,
	uint32_t len); 