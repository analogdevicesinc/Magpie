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

/// q15 samples take up 2 bytes
#define DATA_CONVERTERS_Q15_SIZE_IN_BYTES (2)

/// an i24 integer sample takes up 3 bytes
#define DATA_CONVERTERS_I24_SIZE_IN_BYTES (3)

/// q31 samples take up 4 bytes
#define DATA_CONVERTERS_Q31_SIZE_IN_BYTES (4)

/// The least common multiple of q31 and i24 is important, we process blocks of samples simultaneously and need to be able to fit an even number of samples into a word boundary
#define DATA_CONVERTERS_Q31_AND_I24_LCM_IN_BYTES (DATA_CONVERTERS_Q31_SIZE_IN_BYTES * DATA_CONVERTERS_I24_SIZE_IN_BYTES)

/// The smallest valid chunk size for an i24 is 12 bytes, this is four 24 bit samples crammed into three 32 bit words
#define DATA_CONVERTERS_I24_SMALLEST_VALID_CHUNK_SIZE (DATA_CONVERTERS_Q31_AND_I24_LCM_IN_BYTES)

/* Public function declarations --------------------------------------------------------------------------------------*/

/**
 * @brief `data_converters_i24_swap_endianness(S, D, L)` stores `L` 24 bit samples from `S` into `D` with the ms and
 * ls bytes swapped.
 *
 * @param src the buffer of packed 24 bit samples to swap endianness of. Must be at least `3L` bytes long.
 *
 * @param dest the destination buffer for the packed 24 bit samples, must be at least as long as `src`.
 *
 * @param src_len_in_samps the length of the source array in samples. This must be a multiple of 4, because 4 is the
 * number of 24 bit samples that fit evenly into three 32 bit words.
 *
 * @retval the length of the data transferred to the dest buffer in bytes. This will be `3L` if all operations were
 * successful, else 0.
 *
 * @post the dest array `D` is filled with `L` samples from `S` with the endianness of each sample swapped.
 *
 * Example:
 * The below diagram shows four 24 bit samples packed into three 32 bit words. This is the smallest valid chunk size.
 * Before swapping the samples have their ms bytes first, then mid byte, then ls byte. After swapping the order from
 * right to left is ms byte, mid byte, ls byte.
 * ```
 *  09 0A 0B 06, 07 08 03 04, 05 00 01 02  <- src buffer pre swapping, big endian, one chunk shown
 * |--------|---------|---------|--------| <- demarcation of the four 24 bit input samples
 *
 *  0B 0A 09 08, 07 06 05 04, 03 02 01 00  <- dest buffer post swapping, little endian, one chunk shown
 * |--------|---------|---------|--------| <- demarcation of the four 24 bit output samples
 * ```
 */
uint32_t data_converters_i24_swap_endianness(const uint8_t *src, uint8_t *dest, uint32_t src_len_in_samps);

/**
 * @brief `data_converters_i24_to_q31_with_endian_swap(S, D, L)` swaps the endianness of `L` samples in source array
 * `S` of packed 24 bit samples, expands them to q31's, and finally stores them in array `D`.
 *
 * @param src the source buffer of signed 24 bit samples, must be at least `3L` bytes long.
 *
 * @param dest the destination for the expanded 32 bit samples, must be at least '4L' bytes long.
 *
 * @param src_len_in_samps the length of the source array in samples. This must be a multiple of 4, because 4 is the
 * number of 24 bit samples that fit evenly into three 32 bit words.
 *
 * @retval the length of the data transferred to the dest buffer in bytes. This will be `4L` if all operations were
 * successful, else 0.
 *
 * @post the dest array `D` is filled with `L` 24 bit samples from `S` with the endianness swapped and expanded to
 * take up 32 bits, the least significant bytes of the output samples are zero'd.
 *
 * Example:
 * The below diagram shows four 24 bit input samples expanded into four 32 bit samples. The least significant bytes
 * of each sample are zero'd out in the output buffer.
 * ```
 *               0B 0A 09 08, 07 06 05 04, 03 02 01 00  <- src buffer, one chunk shown
 *              |--------|---------|---------|--------| <- demarcation of the four 24 bit input samples
 *
 *  09 0A 0B 00, 06 07 08 00, 03 04 05 00, 00 01 20 00  <- dest buffer, one chunk shown, note the endianness swap
 * |------------|------------|------------|-----------| <- demarcation of the four 32 bit samples with the ls bytes zero'd
 * ```
 */
uint32_t data_converters_i24_to_q31_with_endian_swap(const uint8_t *src, q31_t *dest, uint32_t src_len_in_samps);

/**
 * @brief `data_converters_i24_to_q15_with_endian_swap(S, D, L)` swaps the endianness of `L` samples in source array
 * `S` of packed 24 bit samples, truncates them to q15's, and finally stores them in array `D`.
 *
 * @param src the source buffer of signed 24 bit samples, must be at least `3L` bytes long.
 *
 * @param dest the destination for the truncated 16 bit samples, must be at least '2L' bytes long.
 *
 * @param src_len_in_samps the length of the source array in samples. This must be a multiple of 4, because 4 is the
 * number of 24 bit samples that fit evenly into three 32 bit words.
 *
 * @retval the length of the data transferred to the dest buffer in bytes. This will be `2L` if all operations were
 * successful, else 0.
 *
 * @post the dest array `D` is filled with `L` samples from `S` with the endianness swapped and truncated to 16 bits.
 *
 * Example:
 * The below diagram shows four 24 bit input samples expanded into four 32 bit samples. The least significant bytes
 * of each sample are zero'd out in the output buffer.
 * ```
 *  0B 0A 09 08, 07 06 05 04, 03 02 01 00  <- src buffer, one chunk shown
 * |--------|---------|---------|--------| <- demarcation of the four 24 bit input samples
 *
 *               09 0A 06 07, 03 04 00 01  <- dest buffer, one chunk shown
 *              |-----|------|-----|-----| <- demarcation of the four 16 bit samples with the ls bytes truncated
 * ```
 */
uint32_t data_converters_i24_to_q15_with_endian_swap(const uint8_t *src, q15_t *dest, uint32_t src_len_in_samps);

/**
 * @brief `data_converters_q31_to_i24(S, D, L)` converts `L` samples from source array `S` of q31s to 24 bit signed
 * ints and stores them in destination array `D`. This function can work in-place if the same pointer is passed as `S`
 * and `D` with `D` cast to a (uint8_t *).
 *
 * @param src the source array of q31 samples, must be at least `L` 32 bit words long.
 *
 * @param dest the destination array for the truncated 24 bit samples, must be at least `3L` bytes long.
 *
 * @param src_len_in_samps the length of the source array in samples. This must be a multiple of 4, because 4 is the
 * number of 24 bit samples that fit evenly into three 32 bit words.
 *
 * @retval the length of the data transferred to the dest buffer in bytes. This will be `3L` if all operations were
 * successful, else 0.
 *
 * @post the destination array `D` is filled with `L` 32 bit samples from `S` truncated to take up 24 bits. The 24 bit
 * samples are packed into the destination array so that four 24 bit samples fit into three 32 bit words.
 *
 * Example:
 * The below diagram shows 4 32 bit samples truncated down to 4 24 bit samples. The numbers given to the positions
 * are only to help keep track of the various bytes. We start with 4 32 bit samples taking up 4 words, and finish
 * with 4 24 bit samples taking up 3 words. For each 32 bit input sample, we truncate the least significant byte
 * and shuffle the rest of the bytes to take up the least possible space.
 * ```
 *  0F 0E 0D 0C, 0B 0A 09 08, 07 06 05 04, 03 02 01 00  <- src buffer, one chunk shown
 * |------------|------------|------------|-----------| <- demarcation of the four 32 bit input samples
 *
 *               0F 0E 0D 0B, 0A 09 07 06, 05 03 02 01  <- dest buffer, one chunk shown
 *              |--------|---------|---------|--------| <- demarcation of the four 24 bit truncated samples, split across the 32 bit words
 * ```
 */
uint32_t data_converters_q31_to_i24(const q31_t *src, uint8_t *dest, uint32_t src_len_in_samps);

/**
 * @brief `data_converters_q31_to_q15(S, D, L)` converts `L` samples from source array `S` of q31s to q15s and stores
 * them in destination array `D`. This function can work in-place if the same pointer is passed as `S` and `D` with
 * `D` cast to a (q15_t *).
 *
 * @param src the source array of q31 samples, must be at least `L` words long.
 *
 * @param dest the destination array for the truncated q15 samples, must be at `L` words long.
 *
 * @param src_len_in_samps the length of the source array in samples, no restriction on length.
 *
 * @retval the length of the data transferred to the dest buffer in bytes. This will be `2L` if all operations were
 * successful, else 0.
 *
 * @post the destination array `D` is filled with `L` 32 bit samples from `S` truncated to take up 16 bits.
 *
 * Example:
 * The below diagram shows 4 32 bit samples truncated down to 4 24 bit samples. The numbers given to the positions
 * are only to help keep track of the various bytes. We start with 4 32 bit samples taking up 4 words, and finish
 * with 4 24 bit samples taking up 3 words. For each 32 bit input sample, we truncate the least significant byte
 * and shuffle the rest of the bytes to take up the least possible space.
 * ```
 *  0F 0E 0D 0C, 0B 0A 09 08, 07 06 05 04, 03 02 01 00  <- src buffer, one chunk shown
 * |------------|------------|------------|-----------| <- demarcation of the four 32 bit input samples
 *
 *                            0F 0E 0B 0A, 07 06 03 02  <- dest buffer, one chunk shown
 *                           |-----|------|-----|-----| <- demarcation of the four 16 bit truncated samples
 * ```
 */
uint32_t data_converters_q31_to_q15(const q31_t *src, q15_t *dest, uint32_t src_len_in_samps);

/**
 * @brief `data_converters_interleave_2_q31_to_q15(S0, S1, D, L)` interleaves `L` samples from `S0` and `S1` into
 * destination array `D`. Samples are trunctated to 16 bits and interleaved in an alternating pattern given by
 * `[S0[0], S1[0], S0[1], S1[1], S0[2], ... S0[L - 1], S1[L - 1]]`.
 *
 * @param src0 the first of the two streams to truncate and interleave, must be at least `L` q31_t's long.
 *
 * @param src1 the second of the two streams to truncate and interleave, must be at least `L` q31_t's long.
 *
 * @param dest the destination array for the truncated and interleaved data, must be at least `2L` q15_t's long.
 *
 * @param src_len_in_samps the number of samples from each of `src0` and `src1` to interleave into `dest`. No
 * restriction on length.
 *
 * @retval the length of the data transferred to the `dest` buffer in bytes. This will be `4L` if all operations were
 * successful, else 0.
 *
 * @post the destination array `D` is filled with a total of `2L` samples from `S0` and `S1` (`L` samples from `S0`
 * and another `L` samples from `S1`), each sample is truncated to 16 bits and interleaved one after the other.
 *
 * Example:
 * The below diagram shows 4 32-bit samples from each source interleaved into 8 16-bit samples in the destination.
 * Only the most significant 16 bits of each source sample are used.
 * ```
 *  11 22 33 44, 55 66 77 88, 99 AA BB CC, DD EE FF 00  <- 4 q31_t samples from src0
 *  FF EE DD CC, BB AA 99 88, 77 66 55 44, 33 22 11 00  <- 4 q31_t samples from src1
 * |------------|------------|------------|-----------| <- demarcation of the 32 bit input samples
 *
 *  11 22, FF EE, 55 66, BB AA, 99 AA, 77 66, DD EE, 33 22  <- 8 truncated and interleaved q15_t samples in dest
 * |------|------|------|------|------|------|------|-----| <- demarcation of the eight 16 bit output samples
 *  src0   src1   src0   src1   src0   src1   src0   src1
 * ```
 */
uint32_t data_converters_interleave_2_q31_to_q15(const q31_t *src0, const q31_t *src1, q15_t *dest, uint32_t src_len_in_samps);

/**
 * @brief `data_converters_interleave_2_q31_to_i24(S0, S1, D, L)` interleaves `L` samples from `S0` and `S1` into
 * destination array `D`. Samples are trunctated to 24 bits and interleaved in an alternating pattern given by
 * `[S0[0], S1[0], S0[1], S1[1], S0[2], ... S0[L - 1], S1[L - 1]]`.
 *
 * @param src0 the first of the two streams to truncate and interleave, must be at least `L` q31_t's long.
 *
 * @param src1 the second of the two streams to truncate and interleave, must be at least `L` q31_t's long.
 *
 * @param dest the destination array for the truncated and interleaved data, must be at least `6L` bytes long.
 *
 * @param src_len_in_samps the number of samples from each of `src0` and `src1` to interleave into `dest`. This must be
 * a multiple of 4, because 4 is the number of 24 bit samples that fit evenly into three 32 bit words.
 *
 * @retval the length of the data transferred to the `dest` buffer in bytes. This will be `6L` if all operations were
 * successful, else 0.
 *
 * @post the destination array `D` is filled with a total of `2L` samples from `S0` and `S1` (`L` samples from `S0`
 * and another `L` samples from `S1`), each sample is truncated to 24 bits and interleaved one after the other.
 *
 * Example:
 * The below diagram shows 2 32-bit samples from each source interleaved into 4 24-bit samples in the destination.
 * Only the most significant 24 bits of each source sample are used.
 * ```
 *  11 22 33 44, 55 66 77 88  <- 2 q31_t samples from src0
 *  AA BB CC DD, EE FF 00 11  <- 2 q31_t samples from src1
 * |------------|-----------| <- demarcation of the 32 bit input samples
 *
 *  11 22, 33 AA, BB CC, 55 66, 77 EE, FF 00  <- 4 truncated and interleaved i24 samples in dest
 * |---------|----------|---------|---------| <- demarcation of the four 24 bit output samples split across 6 bytes
 *    src0       src1       src0      src1
 * ```
 */
uint32_t data_converters_interleave_2_q31_to_i24(const q31_t *src0, const q31_t *src1, uint8_t *dest, uint32_t src_len_in_samps);

/**
 * @brief `data_converters_interleave_2_i24_and_swap_endianness(S0, S1, D, L)` interleaves `L` samples from `S0` and
 * `S1` into destination array `D` while also swapping the endianness of each sample. Samples are interleaved in an
 * alternating pattern given by `[S0[0], S1[0], S0[1], S1[1], S0[2], ... S0[L - 1], S1[L - 1]]`.
 *
 * @param src0 the first of the two streams to endian-swap and interleave, must be at least `3L` bytes long.
 *
 * @param src1 the second of the two streams to endian-swap and interleave, must be at least `3L` bytes long.
 *
 * @param dest the destination array for the endian-swapped and interleaved data, must be at least `6L` bytes long.
 *
 * @param src_len_in_samps the number of samples from each of `src0` and `src1` to interleave into `dest`. This must be
 * a multiple of 4, because 4 is the number of 24 bit samples that fit evenly into three 32 bit words.
 *
 * @retval the length of the data transferred to the `dest` buffer in bytes. This will be `6L` if all operations were
 * successful, else 0.
 *
 * @post the destination array `D` is filled with a total of `2L` samples from `S0` and `S1` (`L` samples from `S0`
 * and another `L` samples from `S1`), samples have their endianness swapped and are interleaved one after the other.
 *
 * Example:
 * ```
 * TODO
 * ```
 */
uint32_t data_converters_interleave_2_i24_and_swap_endianness(const uint8_t *src0, const uint8_t *src1, uint8_t *dest, uint32_t src_len_in_samps);

/**
 * @brief `data_converters_interleave_2_i24_to_q15_and_swap_endianness(S0, S1, D, L)` interleaves `L` samples from `S0`
 * and `S1` into destination array `D` while also swapping the endianness of each sample and truncating each sample from
 * 24 bits to 16 bits. Samples are interleaved in an alternating pattern given by
 * `[S0[0], S1[0], S0[1], S1[1], S0[2], ... S0[L - 1], S1[L - 1]]`.
 *
 * @param src0 the first of the two streams to truncate, endian-swap, and interleave, must be at least `3L` bytes long.
 *
 * @param src1 the second of the two streams to truncate, endian-swap, interleave, must be at least `3L` bytes long.
 *
 * @param dest the destination array for the truncated, endian-swapped, interleaved data, must be at least `4L` bytes long.
 *
 * @param src_len_in_samps the number of samples from each of `src0` and `src1` to interleave into `dest`. This must be
 * a multiple of 4, because 4 is the number of 24 bit samples that fit evenly into three 32 bit words.
 *
 * @retval the length of the data transferred to the `dest` buffer in bytes. This will be `4L` if all operations were
 * successful, else 0.
 *
 * @post the destination array `D` is filled with a total of `2L` samples from `S0` and `S1` (`L` samples from `S0`
 * and another `L` samples from `S1`), each sample is truncated to 16 bits and interleaved one after the other.
 *
 * Example:
 * ```
 * TODO
 * ```
 */
uint32_t data_converters_interleave_2_i24_to_q15_and_swap_endianness(const uint8_t *src0, const uint8_t *src1, q15_t *dest, uint32_t src_len_in_samps);

#endif /* DATA_CONVERTERS_H_ */
