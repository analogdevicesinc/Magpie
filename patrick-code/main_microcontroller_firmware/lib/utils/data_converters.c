
/* Private includes --------------------------------------------------------------------------------------------------*/

#include "data_converters.h"

/* Public function definitions ---------------------------------------------------------------------------------------*/

uint32_t data_converters_i24_swap_endianness(const uint8_t *src, uint8_t *dest, uint32_t src_len_in_samps)
{
    // return early on bad inputs, we can only work on multiples of 4
    // since four 24 bit samples fits evenly into three 32 bit words
    if (src_len_in_samps & 0b11) // mask of 3 is equiv to mod(4)
    {
        return 0;
    }

    // Swap the endianness of an array of 24 bit samples four samples at a time.
    // Each chunk: 12 bytes in, 12 bytes out.
    // This is faster than using a simple for-loop. An equivalent for-loop is shown below (commented out) for clarity.

    // for (uint32_t i = 0; i < src_len_in_samps*3; i += 3)
    // {
    //     const uint8_t b0 = src[i];
    //     const uint8_t b1 = src[i + 1];
    //     const uint8_t b2 = src[i + 2];

    //     dest[i] = b2;
    //     dest[i + 1] = b1;
    //     dest[i + 2] = b0;
    // }

    // loop unrolling, compute 4 outputs at at time (three 32 bit input words comprise four 24 bit samples)
    uint32_t block_count = src_len_in_samps >> 2;

    // typecast so we can process 4 bytes at a time
    q31_t *src_cast = (q31_t *)src;
    q31_t *dest_cast = (q31_t *)dest;

    while (block_count > 0)
    {
        // four 24 bit input samples fit into three 32 bit words
        q31_t in1, in2, in3;
        q31_t out1, out2, out3;

        in1 = *src_cast++;
        in2 = *src_cast++;
        in3 = *src_cast++;

        out1 = ((in2 << 16) & 0xFF000000) | ((in1 << 16) & 0x00FF0000) | (in1 & 0x0000FF00) | ((in1 >> 16) & 0x000000FF);
        out2 = (in2 & 0xFF0000FF) | ((in3 << 16) & 0x00FF0000) | ((in1 >> 16) & 0x0000FF00);
        out3 = ((in3 << 16) & 0xFF000000) | (in3 & 0x00FF0000) | ((in3 >> 16) & 0x0000FF00) | ((in2 >> 16) & 0x000000FF);

        *dest_cast++ = out1;
        *dest_cast++ = out2;
        *dest_cast++ = out3;

        block_count--;
    }

    // no need to handle any remaining samples, there can't be any because this function only works on multiples of 4

    return src_len_in_samps * DATA_CONVERTERS_I24_SIZE_IN_BYTES;
}

uint32_t data_converters_i24_to_q31_with_endian_swap(const uint8_t *src, q31_t *dest, uint32_t src_len_in_samps)
{
    // return early on bad inputs, we can only work on multiples of 4
    // since four 24 bit samples fits evenly into three 32 bit words
    if (src_len_in_samps & 0b11) // mask of 3 is equiv to mod(4)
    {
        return 0;
    }

    // Convert an array of 24 bit samples into an array of q31's by processing chunks of 4 samples at a time.
    // Each chunk: 12 bytes in, 16 bytes out.
    // This is more efficient than a naive for-loop. An equivalent for-loop is shown below (commented out) for clarity.

    // uint32_t dest_len = 0;
    // for (uint32_t i = 0; i < src_len_in_bytes*3;)
    // {
    //     const uint8_t ms_byte = src[i++];
    //     const uint8_t mid_byte = src[i++];
    //     const uint8_t ls_byte = src[i++];

    //     dest[dest_len++] = (q31_t)((ms_byte << 24) | (mid_byte << 16) | (ls_byte << 8));
    // }

    // loop unrolling, compute 4 outputs at at time (three 32 bit input words comprise four 24 bit samples)
    uint32_t block_count = src_len_in_samps >> 2;

    // typecast so we can process 4 bytes at a time
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

        *dest++ = out1;
        *dest++ = out2;
        *dest++ = out3;
        *dest++ = out4;

        block_count--;
    }

    // no need to handle any remaining samples, there can't be any because this function only works on multiples of 4

    return src_len_in_samps * DATA_CONVERTERS_Q31_SIZE_IN_BYTES;
}

uint32_t data_converters_i24_to_q15_with_endian_swap(const uint8_t *src, q15_t *dest, uint32_t src_len_in_samps)
{
    // return early on bad inputs, we can only work on multiples of 4
    // since four 24 bit samples fits evenly into three 32 bit words
    if (src_len_in_samps & 0b11) // mask of 3 is equiv to mod(4)
    {
        return 0;
    }

    // Convert an array of 24 bit samples into an array of q15's by processing chunks of 4 samples at a time.
    // Each chunk: 12 bytes in, 8 bytes out.
    // This is more efficient than a naive for-loop. An equivalent for-loop is shown below (commented out) for clarity.

    // uint32_t dest_len = 0;
    // for (uint32_t i = 0; i < src_len_in_samps * 3; i += 3)
    // {
    //     const uint8_t ms_byte = src[i];
    //     const uint8_t ls_byte = src[i + 1];

    //     dest[dest_len++] = (q15_t)((ms_byte << 8) | ls_byte);
    // }

    // typecast so we can process 4 bytes at a time
    q31_t *src_cast = (q31_t *)src;
    q31_t *dest_cast = (q31_t *)dest;

    // loop unrolling, compute 4 outputs at at time (three 32 bit input words comprise four 24 bit samples)
    uint32_t block_count = src_len_in_samps >> 2;

    while (block_count > 0)
    {
        q31_t in1, in2, in3;
        q31_t out1, out2;

        in1 = *src_cast++;
        in2 = *src_cast++;
        in3 = *src_cast++;

        out1 = ((in1 << 0) & 0xFF000000) | ((in2 << 16) & 0x00FF0000) | ((in1 << 8) & 0x0000FF00) | ((in1 >> 8) & 0x000000FF);
        out2 = ((in3 << 16) & 0xFF000000) | ((in3 << 0) & 0x00FF0000) | ((in2 >> 8) & 0x0000FF00) | ((in2 >> 24) & 0x000000FF);

        *dest_cast++ = out1;
        *dest_cast++ = out2;

        block_count--;
    }

    // no need to handle any remaining samples, there can't be any because this function only works on multiples of 4

    return src_len_in_samps * DATA_CONVERTERS_Q15_SIZE_IN_BYTES;
}

uint32_t data_converters_q31_to_i24(const q31_t *src, uint8_t *dest, uint32_t src_len_in_samps)
{
    // return early on bad inputs, we can only work on multiples of 4
    // since four 24 bit samples fits evenly into three 32 bit words
    if (src_len_in_samps & 0b11) // mask of 3 is equiv to mod(4)
    {
        return 0;
    }

    // Convert an array of q31's into an array of 24 bit integers by processing chunks of 4 samples at a time.
    // Each chunk: 16 bytes in, 12 bytes out.
    // This is more efficient than a naive for-loop. An equivalent for-loop is shown below (commented out) for clarity.

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

    // typecast so we can process 4 bytes at a time
    q31_t *dest_cast = (q31_t *)dest;

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

        *dest_cast++ = out1;
        *dest_cast++ = out2;
        *dest_cast++ = out3;

        block_count--;
    }

    // no need to handle any remaining samples, there can't be any because this function only works on multiples of 4

    return src_len_in_samps * DATA_CONVERTERS_I24_SIZE_IN_BYTES;
}

uint32_t data_converters_q31_to_q15(const q31_t *src, q15_t *dest, uint32_t src_len_in_samps)
{
    // Convert an array of q31's into an array of 16 bit integers by processing chunks of 4 samples at a time.
    // Each chunk: 16 bytes in, 8 bytes out.
    // This is more efficient than a naive for-loop. An equivalent for-loop is shown below (commented out) for clarity.

    // for (uint32_t i = 0; i < src_len_in_samps; i++)
    // {
    //     dest[i] = src[i] >> 16;
    // }

    q31_t in1, in2, in3, in4;
    q31_t out1, out2;

    uint32_t block_count = src_len_in_samps >> 2;

    // typecast so we can process 4 bytes at a time
    q31_t *dest_cast = (q31_t *)dest;

    while (block_count > 0)
    {
        in1 = *src++;
        in2 = *src++;
        in3 = *src++;
        in4 = *src++;

        out1 = (in2 & 0xFFFF0000) | ((in1 >> 16) & 0x0000FFFF);
        out2 = (in4 & 0xFFFF0000) | ((in3 >> 16) & 0x0000FFFF);

        *dest_cast++ = out1;
        *dest_cast++ = out2;

        block_count--;
    }

    // Handle any remaining samples, this function can work on arbitrary numbers of samples at a time
    uint32_t remaining_samples = src_len_in_samps & 0x3;
    while (remaining_samples > 0)
    {
        *dest++ = (q15_t)(*src++ >> 16);
        remaining_samples--;
    }

    return src_len_in_samps * DATA_CONVERTERS_Q15_SIZE_IN_BYTES;
}

uint32_t data_converters_interleave_2_q31_to_q15(const q31_t *src0, const q31_t *src1, q15_t *dest, uint32_t src_len_in_samps)
{
    // Interleave two arrays of q31_t samples into one array of q15_t samples.
    // Process 4 samples from each source at a time (8 q31_t input samples, 8 q15_t output samples).
    // This is more efficient than a naive for-loop. An equivalent for-loop is shown below (commented out) for clarity.

    // for (uint32_t i = 0; i < src_len_in_samps; i++)
    // {
    //     dest[i * 2] = (q15_t)(src0[i] >> 16);
    //     dest[i * 2 + 1] = (q15_t)(src1[i] >> 16);
    // }

    // Loop unrolling, compute 8 outputs at a time (4 from each source)
    uint32_t block_count = src_len_in_samps >> 2;

    // typecast so we can process 4 bytes at a time
    q31_t *dest_cast = (q31_t *)dest;

    while (block_count > 0)
    {
        q31_t in1, in2, in3, in4, in5, in6, in7, in8;
        q31_t out1, out2, out3, out4;

        // Load 4 samples from each source
        in1 = *src0++;
        in2 = *src0++;
        in3 = *src0++;
        in4 = *src0++;
        in5 = *src1++;
        in6 = *src1++;
        in7 = *src1++;
        in8 = *src1++;

        // Interleave and pack into q15_t
        out1 = ((in1 >> 16) & 0x0000FFFF) | (in5 & 0xFFFF0000);
        out2 = ((in2 >> 16) & 0x0000FFFF) | (in6 & 0xFFFF0000);
        out3 = ((in3 >> 16) & 0x0000FFFF) | (in7 & 0xFFFF0000);
        out4 = ((in4 >> 16) & 0x0000FFFF) | (in8 & 0xFFFF0000);

        // Store the results
        *dest_cast++ = out1;
        *dest_cast++ = out2;
        *dest_cast++ = out3;
        *dest_cast++ = out4;

        block_count--;
    }

    // Handle any remaining samples, this function can work on arbitrary numbers of samples at a time
    uint32_t remaining_samples = src_len_in_samps & 0x3;
    while (remaining_samples > 0)
    {
        *dest++ = (q15_t)(*src0++ >> 16);
        *dest++ = (q15_t)(*src1++ >> 16);
        remaining_samples--;
    }

    return src_len_in_samps * 2 * sizeof(q15_t);
}

uint32_t data_converters_interleave_2_q31_to_i24(const q31_t *src0, const q31_t *src1, uint8_t *dest, uint32_t src_len_in_samps)
{
    // return early on bad inputs, we can only work on multiples of 4
    // since four 24 bit samples fits evenly into three 32 bit words
    if (src_len_in_samps & 0b11) // mask of 3 is equiv to mod(4)
    {
        return 0;
    }

    // Interleave two arrays of q31_t samples into one array of i24 samples.
    // Process 4 samples from each source at a time (8 q31_t input samples, 24 i24 output bytes).
    // This is more efficient than a naive for-loop. An equivalent for-loop is shown below (commented out) for clarity.

    // for (uint32_t i = 0; i < src_len_in_samps; i++)
    // {
    //     dest[i * 6] = (uint8_t)(src0[i] >> 8);
    //     dest[i * 6 + 1] = (uint8_t)(src0[i] >> 16);
    //     dest[i * 6 + 2] = (uint8_t)(src0[i] >> 24);
    //     dest[i * 6 + 3] = (uint8_t)(src1[i] >> 8);
    //     dest[i * 6 + 4] = (uint8_t)(src1[i] >> 16);
    //     dest[i * 6 + 5] = (uint8_t)(src1[i] >> 24);
    // }

    // Loop unrolling, compute 24 output bytes at a time (4 from each source)
    uint32_t block_count = src_len_in_samps >> 2;

    // typecast so we can process 4 bytes at a time
    q31_t *dest_cast = (q31_t *)dest;

    while (block_count > 0)
    {
        q31_t in1, in2, in3, in4, in5, in6, in7, in8;
        q31_t out1, out2, out3, out4, out5, out6;

        // Load 4 samples from each source
        in1 = *src0++;
        in2 = *src0++;
        in3 = *src0++;
        in4 = *src0++;
        in5 = *src1++;
        in6 = *src1++;
        in7 = *src1++;
        in8 = *src1++;

        // Interleave and pack into i24
        out1 = ((in1 >> 8) & 0x00FFFFFF) | ((in5 << 16) & 0xFF000000);
        out2 = ((in5 >> 16) & 0x0000FFFF) | ((in2 << 8) & 0xFFFF0000);
        out3 = ((in2 >> 24) & 0x000000FF) | ((in6 << 0) & 0xFFFFFF00);
        out4 = ((in3 >> 8) & 0x00FFFFFF) | ((in7 << 16) & 0xFF000000);
        out5 = ((in7 >> 16) & 0x0000FFFF) | ((in4 << 8) & 0xFFFF0000);
        out6 = ((in4 >> 24) & 0x000000FF) | ((in8 << 0) & 0xFFFFFF00);

        // Store the results
        *dest_cast++ = out1;
        *dest_cast++ = out2;
        *dest_cast++ = out3;
        *dest_cast++ = out4;
        *dest_cast++ = out5;
        *dest_cast++ = out6;

        block_count--;
    }

    // no need to handle any remaining samples, there can't be any because this function only works on multiples of 4

    return src_len_in_samps * 2 * 3; // 3 bytes per i24 sample, 2 samples per iteration
}

uint32_t data_converters_interleave_2_i24_and_swap_endianness(const uint8_t *src0, const uint8_t *src1, uint8_t *dest, uint32_t src_len_in_samps)
{
    // return early on bad inputs, we can only work on multiples of 4
    // since four 24 bit samples fits evenly into three 32 bit words
    if (src_len_in_samps & 0b11) // mask of 3 is equiv to mod(4)
    {
        return 0;
    }

    // TODO add comment description of inputs/outputs and add equivalent for-loop for clarity (commented out)

    // Process 4 samples from each source at a time (12 bytes per source)
    uint32_t block_count = src_len_in_samps >> 2;

    // typecast so we can process 4 bytes at a time
    q31_t *src0_cast = (q31_t *)src0;
    q31_t *src1_cast = (q31_t *)src1;
    q31_t *dest_cast = (q31_t *)dest;

    while (block_count > 0)
    {
        // Load 4 samples from each source (12 bytes each)
        q31_t in1, in2, in3, in4, in5, in6;
        q31_t out1, out2, out3, out4, out5, out6;

        // Load source 0
        in1 = *src0_cast++;
        in2 = *src0_cast++;
        in3 = *src0_cast++;

        // Load source 1
        in4 = *src1_cast++;
        in5 = *src1_cast++;
        in6 = *src1_cast++;

        out1 = ((in4 << 8) & 0xFF000000) | ((in1 << 16) & 0x00FF0000) | ((in1 << 0) & 0x0000FF00) | ((in1 >> 16) & 0x000000FF);
        out2 = ((in2 << 24) & 0xFF000000) | ((in2 << 8) & 0x00FF0000) | ((in4 << 8) & 0x0000FF00) | ((in4 >> 8) & 0x000000FF);
        out3 = ((in4 << 0) & 0xFF000000) | ((in5 << 16) & 0x00FF0000) | ((in5 << 0) & 0x0000FF00) | ((in1 >> 24) & 0x000000FF);
        out4 = ((in6 << 24) & 0xFF000000) | ((in2 << 0) & 0x00FF0000) | ((in2 >> 16) & 0x0000FF00) | ((in3 << 0) & 0x000000FF);
        out5 = ((in3 << 8) & 0xFF000000) | ((in3 >> 8) & 0x00FF0000) | ((in5 >> 8) & 0x0000FF00) | ((in5 >> 24) & 0x000000FF);
        out6 = ((in6 << 16) & 0xFF000000) | ((in6 << 0) & 0x00FF0000) | ((in6 >> 16) & 0x0000FF00) | ((in3 >> 8) & 0x000000FF);

        // Store interleaved results
        *dest_cast++ = out1;
        *dest_cast++ = out2;
        *dest_cast++ = out3;
        *dest_cast++ = out4;
        *dest_cast++ = out5;
        *dest_cast++ = out6;

        block_count--;
    }

    // no need to handle any remaining samples, there can't be any because this function only works on multiples of 4

    return src_len_in_samps * 2 * DATA_CONVERTERS_I24_SIZE_IN_BYTES;
}

uint32_t data_converters_interleave_2_i24_to_q15_and_swap_endianness(const uint8_t *src0, const uint8_t *src1, q15_t *dest, uint32_t src_len_in_samps)
{
    // return early on bad inputs, we can only work on multiples of 4
    // since four 24 bit samples fits evenly into three 32 bit words
    if (src_len_in_samps & 0b11) // mask of 3 is equiv to mod(4)
    {
        return 0;
    }

    // TODO add comment description of inputs/outputs and add equivalent for-loop for clarity (commented out)

    // Process samples in blocks of 4
    uint32_t block_count = src_len_in_samps >> 2;

    // typecast so we can process 4 bytes at a time
    q31_t *src0_cast = (q31_t *)src0;
    q31_t *src1_cast = (q31_t *)src1;
    q31_t *dest_cast = (q31_t *)dest;

    while (block_count > 0)
    {
        // Load 4 samples (12 bytes) from each source
        uint32_t in1, in2, in3, in4, in5, in6;
        uint32_t out1, out2, out3, out4;

        // Load src0 samples
        in1 = *src0_cast++;
        in2 = *src0_cast++;
        in3 = *src0_cast++;

        // Load src1 samples
        in4 = *src1_cast++;
        in5 = *src1_cast++;
        in6 = *src1_cast++;

        // Combine and truncate to 16 bits, swapping endianness
        // Each output word contains two 16-bit samples
        out1 = ((in4 << 24) & 0xFF000000) | ((in4 << 8) & 0x00FF0000) | ((in1 << 8) & 0x0000FF00) | ((in1 >> 8) & 0x000000FF);
        out2 = ((in4 << 0) & 0xFF000000) | ((in5 << 16) & 0x00FF0000) | ((in1 >> 16) & 0x0000FF00) | ((in2 << 0) & 0x000000FF);
        out3 = ((in5 << 8) & 0xFF000000) | ((in5 >> 8) & 0x00FF0000) | ((in2 >> 8) & 0x0000FF00) | ((in2 >> 24) & 0x000000FF);
        out4 = ((in6 << 16) & 0xFF000000) | ((in6 << 0) & 0x00FF0000) | ((in3 << 0) & 0x0000FF00) | ((in3 >> 16) & 0x000000FF);

        // Store interleaved results
        *dest_cast++ = out1;
        *dest_cast++ = out2;
        *dest_cast++ = out3;
        *dest_cast++ = out4;

        block_count--;
    }

    // no need to handle any remaining samples, there can't be any because this function only works on multiples of 4

    return src_len_in_samps * 2 * DATA_CONVERTERS_Q15_SIZE_IN_BYTES;
}
