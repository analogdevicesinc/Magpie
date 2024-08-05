
/* Private includes --------------------------------------------------------------------------------------------------*/

#include "data_converters.h"

/* Public function definitions ---------------------------------------------------------------------------------------*/

void data_converters_i24_swap_endianness(uint8_t *src, uint8_t *dest, uint32_t len_in_bytes)
{
    /**
     * Swap the endianness of an array of 24 bit samples four samples at a time.
     * Each chunk: 12 bytes in, 12 bytes out.
     * This is faster than using a simple for-loop. An equivalent for-loop is shown below (commented out) for clarity.
     */

    // for (uint32_t i = 0; i < len_in_bytes; i += 3)
    // {
    //     const uint8_t b0 = src[i];
    //     const uint8_t b1 = src[i + 1];
    //     const uint8_t b2 = src[i + 2];

    //     dest[i] = b2;
    //     dest[i + 1] = b1;
    //     dest[i + 2] = b0;
    // }

    // loop unrolling, compute 4 outputs at at time (three 32 bit input words comprise four 24 bit samples)
    uint32_t block_count = len_in_bytes / DATA_CONVERTERS_Q31_AND_I24_LCM_IN_BYTES;

    while (block_count > 0)
    {
        // four 24 bit input samples fit into three 32 bit words
        q31_t in1, in2, in3;
        q31_t out1, out2, out3;

        in1 = *__SIMD32(src)++;
        in2 = *__SIMD32(src)++;
        in3 = *__SIMD32(src)++;

        out1 = ((in2 << 16) & 0xFF000000) | ((in1 << 16) & 0x00FF0000) | (in1 & 0x0000FF00) | ((in1 >> 16) & 0x000000FF);
        out2 = (in2 & 0xFF0000FF) | ((in3 << 16) & 0x00FF0000) | ((in1 >> 16) & 0x0000FF00);
        out3 = ((in3 << 16) & 0xFF000000) | (in3 & 0x00FF0000) | ((in3 >> 16) & 0x0000FF00) | ((in2 >> 16) & 0x000000FF);

        *__SIMD32(dest)++ = out1;
        *__SIMD32(dest)++ = out2;
        *__SIMD32(dest)++ = out3;

        block_count--;
    }
}

uint32_t data_converters_i24_to_q31_with_endian_swap(uint8_t *src, q31_t *dest, uint32_t src_len_in_bytes)
{
    /**
     * Convert an array of 24 bit samples into an array of q31's by processing chunks of 4 samples at a time.
     * Each chunk: 12 bytes in, 16 bytes out.
     * This is more efficient than a naive for-loop. An equivalent for-loop is shown below (commented out) for clarity.
     */

    // uint32_t dest_len = 0;
    // for (uint32_t i = 0; i < src_len_in_bytes;)
    // {
    //     const uint8_t ms_byte = src[i++];
    //     const uint8_t mid_byte = src[i++];
    //     const uint8_t ls_byte = src[i++];

    //     dest[dest_len++] = (q31_t)((ms_byte << 24) | (mid_byte << 16) | (ls_byte << 8));
    // }

    // loop unrolling, compute 4 outputs at at time (three 32 bit input words comprise four 24 bit samples)
    uint32_t block_count = src_len_in_bytes / DATA_CONVERTERS_Q31_AND_I24_LCM_IN_BYTES;

    // cast the input to an array of 32 bit q31's so we can loop through it as chunks more easily
    q31_t *src_cast = (q31_t *)src;

    while (block_count > 0)
    {
        // four 24 bit input samples fit into three 32 bit words, but they need four 32 bit words after being expanded into q31's
        q31_t in1, in2, in3;
        q31_t out1, out2, out3, out4;

        in1 = *src_cast++;
        in2 = *src_cast++;
        in3 = *src_cast++;

        out1 = ((in1 << 24) & 0xFF000000) | ((in1 << 8) & 0x0FF0000) | ((in1 >> 8) & 0x0000FF00); // samp1, all 3 relevant bytes from in1
        out2 = (in1 & 0xFF000000) | ((in2 << 16) & 0x0FF0000) | (in2 & 0x0000FF00);               // samp2, ms byte from in1, 2 ls bytes from in2
        out3 = ((in2 << 8) & 0xFF000000) | ((in2 >> 8) & 0x0FF0000) | ((in3 << 8) & 0x0000FF00);  // samp3, 2 ms bytes from in2, ls byte from in3
        out4 = ((in3 << 16) & 0xFF000000) | (in3 & 0x0FF0000) | ((in3 >> 16) & 0x0000FF00);       // samp4, all 3 relevant bytes from in3

        *__SIMD32(dest)++ = out1;
        *__SIMD32(dest)++ = out2;
        *__SIMD32(dest)++ = out3;
        *__SIMD32(dest)++ = out4;

        block_count--;
    }

    return (src_len_in_bytes * DATA_CONVERTERS_Q31_SIZE_IN_BYTES) / DATA_CONVERTERS_I24_SIZE_IN_BYTES;
}

uint32_t data_converters_i24_to_q15(uint8_t *src, q15_t *dest, uint32_t src_len_in_bytes)
{
    /**
     * Convert an array of 24 bit samples into an array of q15's by processing chunks of 4 samples at a time.
     * Each chunk: 12 bytes in, 8 bytes out.
     * This is more efficient than a naive for-loop. An equivalent for-loop is shown below (commented out) for clarity.
     */

    // uint32_t dest_len = 0;
    // for (uint32_t i = 0; i < src_len_in_bytes;)
    // {
    //     i++;
    //     const uint8_t ls_byte = src[i++];
    //     const uint8_t ms_byte = src[i++];

    //     dest[dest_len++] = (q15_t)((ms_byte << 8) | ls_byte);
    // }

    // cast the input to an array of 32 bit q31's so we can loop through it as chunks more easily
    q31_t *src_cast = (q31_t *)src;

    // loop unrolling, compute 4 outputs at at time (three 32 bit input words comprise four 24 bit samples)
    uint32_t block_count = src_len_in_bytes / DATA_CONVERTERS_Q31_AND_I24_LCM_IN_BYTES;

    while (block_count > 0)
    {
        q31_t in1, in2, in3;
        q31_t out1, out2;

        in1 = *src_cast++;
        in2 = *src_cast++;
        in3 = *src_cast++;

        out1 = ((in2 << 16) & 0xFFFF0000) | ((in1 >> 8) & 0x0000FFFF);
        out2 = (in3 & 0xFFFF0000) | ((in3 << 8) & 0x0000FF00) | ((in2 >> 24) & 0x000000FF);

        *__SIMD32(dest)++ = out1;
        *__SIMD32(dest)++ = out2;

        block_count--;
    }

    return (src_len_in_bytes * DATA_CONVERTERS_Q15_SIZE_IN_BYTES) / DATA_CONVERTERS_I24_SIZE_IN_BYTES;
}

uint32_t data_converters_q31_to_i24(q31_t *src, uint8_t *dest, uint32_t src_len_in_samps)
{
    /**
     * Convert an array of q31's into an array of 24 bit integers by processing chunks of 4 samples at a time.
     * Each chunk: 16 bytes in, 12 bytes out.
     * This is more efficient than a naive for-loop. An equivalent for-loop is shown below (commented out) for clarity.
     */

    // for (uint32_t i = 0, j = 0; i < src_len_in_samps; i++)
    // {
    //     const uint8_t ls_byte = src[i] >> 8;
    //     const uint8_t mid_byte = src[i] >> 16;
    //     const uint8_t ms_byte = src[i] >> 24;

    //     dest[j++] = ls_byte;
    //     dest[j++] = mid_byte;
    //     dest[j++] = ms_byte;
    // }

    // four 32 bit samples can be stored in three 32 bit words after truncation to 24 bits
    q31_t in1, in2, in3, in4;
    q31_t out1, out2, out3;

    // loop unrolling, compute 4 outputs at at time
    uint32_t block_count = src_len_in_samps >> 2;

    while (block_count > 0)
    {
        in1 = *src++;
        in2 = *src++;
        in3 = *src++;
        in4 = *src++;

        out1 = ((in1 >> 8) & 0x00FFFFFF) | ((in2 << 16) & 0xFF000000); // all 3 bytes of samp1, ls byte of samp 2
        out2 = ((in2 >> 16) & 0x0000FFFF) | ((in3 << 8) & 0xFFFF0000); // 2 ms bytes of samp2, 2 ls bytes of samp3
        out3 = ((in3 >> 24) & 0x000000FF) | ((in4 << 0) & 0xFFFFFF00); // ms byte of samp3, al 3 bytes of samp4

        *__SIMD32(dest)++ = out1;
        *__SIMD32(dest)++ = out2;
        *__SIMD32(dest)++ = out3;

        block_count--;
    }

    return src_len_in_samps * DATA_CONVERTERS_I24_SIZE_IN_BYTES;
}

uint32_t data_converters_q31_to_q15(q31_t *src, q15_t *dest, uint32_t src_len_in_samps)
{
    /**
     * Convert an array of q31's into an array of 16 bit integers by processing chunks of 4 samples at a time.
     * Each chunk: 16 bytes in, 8 bytes out.
     * This is more efficient than a naive for-loop. An equivalent for-loop is shown below (commented out) for clarity.
     */

    // for (uint32_t i = 0; i < src_len_in_samps; i++)
    // {
    //     dest[i] = src[i] >> 16;
    // }

    q31_t in1, in2, in3, in4;
    q31_t out1, out2;

    uint32_t block_count = src_len_in_samps >> 2;

    while (block_count > 0)
    {
        in1 = *src++;
        in2 = *src++;
        in3 = *src++;
        in4 = *src++;

        out1 = (in2 & 0xFFFF0000) | ((in1 >> 16) & 0x0000FFFF);
        out2 = (in4 & 0xFFFF0000) | ((in3 >> 16) & 0x0000FFFF);

        *__SIMD32(dest)++ = out1;
        *__SIMD32(dest)++ = out2;

        block_count--;
    }

    return src_len_in_samps * DATA_CONVERTERS_Q15_SIZE_IN_BYTES;
}
