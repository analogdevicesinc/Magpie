/**
 * @file      data_converters.h
 * @brief     A software interface for converting the format and sample size of buffers of data is represented here.
 * @details   This module is responsible for swapping endianness of buffers and converting sample size from 24 bit to
 *            32 bit and vice versa.
 */

#ifndef DATA_CONVERTERS_H_
#define DATA_CONVERTERS_H_

/* Includes ----------------------------------------------------------------------------------------------------------*/

#include "arm_math.h"

/* Public definitions ------------------------------------------------------------------------------------------------*/

// an i24 integer sample takes up 3 bytes
#define DATA_CONVERTERS_I24_SIZE_IN_BYTES (3)

// q31 samples take up 4 bytes
#define DATA_CONVERTERS_Q31_SIZE_IN_BYTES (4)

#define DATA_CONVERTERS_Q31_AND_I24_LCM_IN_BYTES (DATA_CONVERTERS_Q31_SIZE_IN_BYTES * DATA_CONVERTERS_I24_SIZE_IN_BYTES)
#define DATA_CONVERTERS_I24_SMALLEST_VALID_CHUNK_SIZE (DATA_CONVERTERS_Q31_AND_I24_LCM_IN_BYTES)

/* Public function declarations --------------------------------------------------------------------------------------*/

/**
 * `data_converters_i24_swap_endianness(s, d, l)` stores 24 bit samples in `s` into `d` with the ms and ls bytes swapped
 *
 * @param src the buffer of packed 24 bit samples to swap endianness of, must be at least `l` bytes long
 *
 * @param dest the destination buffer for the packed 24 bit samples, must be at least as long as `src`
 *
 * @param len_in_bytes the length in bytes of the source buffer, must be a multiple of 12 (the smallest valid chunk
 * size in bytes for i24 samples)
 *
 * @post the 24 bit samples in `s` are stored in `d` with their most significant byte and least significant byte swapped
 *
 * Example:
 * The below diagram shows four 24 bit samples packed into three 32 bit words. This is the smallest valid chunk size.
 * Before swapping the samples have their ms bytes first, then mid byte, then ls byte. After swapping the order from
 * right to left is ms byte, mid byte, ls byte.
 *
 *  09 0A 0B 06, 07 08 03 04, 05 00 01 02  <- src buffer pre swapping, big endian, one chunk shown
 * |--------|---------|---------|--------| <- demarcation of the four 24 bit input samples
 *
 *  0B 0A 09 08, 07 06 05 04, 03 02 01 00  <- dest buffer post swapping, little endian, one chunk shown
 * |--------|---------|---------|--------| <- demarcation of the four 24 bit output samples
 */
void data_converters_i24_swap_endianness(uint8_t *src, uint8_t *dest, uint32_t len_in_bytes);

/**
 * @brief `data_converters_i24_to_q31(s, d, l)` converts source array `s` of packed 24 bit samples to q31's and stores
 * them in array `d`
 *
 * @param src the source buffer of signed 24 bit samples, must be at least `l` bytes long
 *
 * @param dest the destination for the expanded 32 bit samples, must be at least src_len * 4/3 bytes long TODO is this right?
 *
 * @param src_len_in_bytes the length of the source array in bytes, not samples, must be a multiple of 12
 *
 * @post the dest array `d` is filled with the 24 bit sampes from `s` expanded to take up 32 bits, the least significant
 * bytes of the output samples are zero'd
 *
 * Example:
 * The below diagram shows four 24 bit input samples expanded into four 32 bit samples. The least significant bytes
 * of each sample are zero'd out in the output buffer.
 *
 *               0B 0A 09 08, 07 06 05 04, 03 02 01 00  <- src buffer, one chunk shown
 *              |--------|---------|---------|--------| <- demarcation of the four 24 bit input samples
 *
 *  0B 0A 09 00, 08 07 06 00, 05 04 03 00, 02 01 00 00  <- dest buffer, one chunk shown
 * |------------|------------|------------|-----------| <- demarcation of the four 32 bit samples with the ls bytes zero'd
 */
void data_converters_i24_to_q31(uint8_t *src, q31_t *dest, uint32_t src_len_in_bytes);

/**
 * @brief `data_converters_q31_to_i24(s, d, l)` converts `l` samples from source array `s` of q31s to 24 bit signed
 * ints and stores them in destination array `d`
 *
 * @param src the source array of q31 samples, must be at least `l` words long
 *
 * @param dest the destination array for the truncated 24 bit samples, must be at least src_len * 3/4 bytes long TODO is this right?
 *
 * @param src_len_in_samps the length of the source array in samples, not bytes, must be a multiple of 4
 *
 * @post the destination array `d` is filled with the 32 bit samples from `s` truncated to take up 24 bits. The 24 bit
 * samples are packed into the destination array so that four 24 bit samples fit into three 32 bit words.
 *
 * Example:
 * The below diagram shows 4 32 bit samples truncated down to 4 24 bit samples. The numbers given to the positions
 * are only to help keep track of the various bytes. We start with 4 32 bit samples taking up 4 words, and finish
 * with 4 24 bit samples taking up 3 words. For each 32 bit input sample, we truncate the least significant byte
 * and shuffle the rest of the bytes to take up the least possible space.
 *
 *  0F 0E 0D 0C, 0B 0A 09 08, 07 06 05 04, 03 02 01 00  <- src buffer, one chunk shown
 * |------------|------------|------------|-----------| <- demarcation of the four 32 bit input samples
 *
 *               0F 0E 0D 0B, 0A 09 07 06, 05 03 02 01  <- dest buffer, one chunk shown
 *              |--------|---------|---------|--------| <- demarcation of the four 24 bit truncated samples, split across the 32 bit words
 */
void data_converters_q31_to_i24(q31_t *src, uint8_t *dest, uint32_t src_len_in_samps);

// adams modification, bitDepth = 16 or 24 (0/1)
void data_converters_q31_to_i16_24(q31_t *src, uint8_t *dest, uint32_t src_len_in_samps,uint8_t bitDepth);


#endif /* DATA_CONVERTERS_H_ */
