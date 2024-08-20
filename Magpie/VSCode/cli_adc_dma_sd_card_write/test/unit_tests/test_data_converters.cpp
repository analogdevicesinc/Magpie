#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test_helpers.hpp"

extern "C"
{
#include "data_converters.h"
}

using namespace testing;

TEST(DataConvertersTest, i24_swap_endianness_smallest_chunk_check_all_bytes)
{
    // the smallest valid chunk is 12 bytes, which is four 24 bit samples
    uint8_t src[DATA_CONVERTERS_I24_SMALLEST_VALID_CHUNK_SIZE] = {
        0x00, 0x11, 0x22, // 1st sample
        0x33, 0x44, 0x55,
        0x66, 0x77, 0x88,
        0x99, 0xAA, 0xBB}; // 4th sample

    uint8_t dest[DATA_CONVERTERS_I24_SMALLEST_VALID_CHUNK_SIZE] = {0};

    data_converters_i24_swap_endianness(src, dest, DATA_CONVERTERS_I24_SMALLEST_VALID_CHUNK_SIZE);

    ASSERT_THAT(dest, ElementsAre(
                          0x22, 0x11, 0x00, // see that the ms byte and ls byte of each sample is swapped
                          0x55, 0x44, 0x33,
                          0x88, 0x77, 0x66,
                          0xBB, 0xAA, 0x99));
}

TEST(DataConvertersTest, i24_swap_endianness_check_sample_in_the_middle)
{
    // arbitrary number of samples to check, note that the num samples must be a multiple of 4
    const uint8_t num_samps = 16;
    const uint8_t len_in_bytes = num_samps * DATA_CONVERTERS_I24_SIZE_IN_BYTES;

    uint8_t src[len_in_bytes];

    // fill the array with mock data so we have something to check, use the idx as value so it's easy to test
    for (uint8_t i = 0; i < len_in_bytes; i++)
    {
        src[i] = i;
    }

    uint8_t dest[len_in_bytes] = {0};

    data_converters_i24_swap_endianness(src, dest, len_in_bytes);

    // check an arbitrary sample in the middle of the array filled with mock data
    const uint8_t sample_to_check = 7;
    const uint8_t idx_to_check = sample_to_check * DATA_CONVERTERS_I24_SIZE_IN_BYTES;

    ASSERT_EQ(dest[idx_to_check], idx_to_check + 2);     // arb sample byte 0
    ASSERT_EQ(dest[idx_to_check + 1], idx_to_check + 1); // arb sample byte 1
    ASSERT_EQ(dest[idx_to_check + 2], idx_to_check);     // arb sample byte 2
}

TEST(DataConvertersTest, i24_swap_endianness_should_not_go_past_array_end)
{
    uint8_t src[DATA_CONVERTERS_I24_SMALLEST_VALID_CHUNK_SIZE + DATA_CONVERTERS_I24_SIZE_IN_BYTES] = {
        0x00, 0x11, 0x22,
        0x33, 0x44, 0x55,
        0x66, 0x77, 0x88,
        0x99, 0xAA, 0xBB,
        0x12, 0x34, 0x56};      // extra sample at the that should not be stored in dest
    uint32_t len_in_bytes = 12; // because the length here says to stop at 12 bytes

    uint8_t dest[DATA_CONVERTERS_I24_SMALLEST_VALID_CHUNK_SIZE + DATA_CONVERTERS_I24_SIZE_IN_BYTES] = {0};
    dest[12] = 0xDA;
    dest[13] = 0xDB;
    dest[14] = 0x0D;

    data_converters_i24_swap_endianness(src, dest, len_in_bytes);

    ASSERT_EQ(dest[12], 0xDA); // <- these should not be swapped
    ASSERT_EQ(dest[13], 0xDB);
    ASSERT_EQ(dest[14], 0x0D); // <- these should not be swapped
}

// TEST(DataConvertersTest, i14_swap_endianness_works_in_place)
// {
// }

TEST(DataConvertersTest, i24_to_q15_smallest_chunk_check_all_bytes)
{
    // the smallest valid chunk is 12 bytes, which is four 24 bit samples
    uint8_t src[DATA_CONVERTERS_I24_SMALLEST_VALID_CHUNK_SIZE] = {
        0x00, 0x11, 0x22, // 1st sample
        0x33, 0x44, 0x55,
        0x66, 0x77, 0x88,
        0x99, 0xAA, 0xBB}; // 4th sample

    q15_t dest[4] = {0};

    data_converters_i24_to_q15(src, dest, DATA_CONVERTERS_I24_SMALLEST_VALID_CHUNK_SIZE);

    ASSERT_THAT(dest, ElementsAre(
                          0x2211,
                          0x5544,
                          0x8877,
                          0xBBAA));
}

TEST(DataConvertersTest, i24_to_q15_check_sample_in_the_middle)
{
    // arbitrary number of samples to check, note that the num samples must be a multiple of 4
    const uint8_t num_samps = 16;
    const uint8_t src_len_in_bytes = num_samps * DATA_CONVERTERS_I24_SIZE_IN_BYTES;

    uint8_t src[src_len_in_bytes];

    // fill the array with mock data so we have something to check, use the idx as value so it's easy to test
    for (uint8_t i = 0; i < src_len_in_bytes; i++)
    {
        src[i] = i;
    }

    q15_t dest[num_samps];

    data_converters_i24_to_q15(src, dest, src_len_in_bytes);

    // check an arbitrary sample in the middle
    const uint32_t sample_to_check = 8;
    const uint32_t src_idx_to_check = sample_to_check * DATA_CONVERTERS_I24_SIZE_IN_BYTES;

    // the expected q15 is a right-shifted version of the original i24
    const uint32_t expected = arr_slice_to_i24(src, src_idx_to_check) >> 8;

    ASSERT_EQ(dest[sample_to_check], expected);
}

TEST(DataConvertersTest, i24_to_q15_should_not_go_past_array_end)
{
    uint8_t src[DATA_CONVERTERS_Q31_AND_I24_LCM_IN_BYTES + DATA_CONVERTERS_I24_SIZE_IN_BYTES] = {
        0x00, 0x01, 0x02,
        0x03, 0x04, 0x05,
        0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B,
        0xDA, 0xDB, 0x0D};          // extra sample at the that should not be moved into the destination array
    uint32_t src_len_in_bytes = 12; // becaue the length here says to stop at after 4 samples

    q15_t dest[5] = {0, 0, 0, 0, 42}; // sentinal value at the end should not be changed

    data_converters_i24_to_q15(src, dest, src_len_in_bytes);

    ASSERT_EQ(dest[4], 42);
}

TEST(DataConvertersTest, i24_to_q31_with_endian_swap_smallest_chunk_check_all_bytes)
{
    // the smallest valid chunk is 12 bytes, which is four 24 bit samples which will be expanded into 4 32 bit words
    uint8_t src[DATA_CONVERTERS_Q31_AND_I24_LCM_IN_BYTES] = {
        0x11, 0x22, 0x33,
        0x44, 0x55, 0x66,
        0x77, 0x88, 0x99,
        0xAA, 0xBB, 0xCC};

    q31_t dest[4] = {0};

    data_converters_i24_to_q31_with_endian_swap(src, dest, DATA_CONVERTERS_Q31_AND_I24_LCM_IN_BYTES);

    ASSERT_THAT(dest, ElementsAre(
                          0x11223300,
                          0x44556600,
                          0x77889900,
                          0xAABBCC00));
}

TEST(DataConvertersTest, i24_to_q31_with_endian_swap_check_sample_in_the_middle)
{
    // arbitrary number of samples to check, note that the num samples must be a multiple of 4
    const uint8_t num_samps = 16;
    const uint8_t src_len_in_bytes = num_samps * DATA_CONVERTERS_I24_SIZE_IN_BYTES;

    uint8_t src[src_len_in_bytes];

    // fill the array with mock data so we have something to check, use the idx as value so it's easy to test
    for (uint8_t i = 0; i < src_len_in_bytes; i++)
    {
        src[i] = i;
    }

    q31_t dest[num_samps];

    data_converters_i24_to_q31_with_endian_swap(src, dest, src_len_in_bytes);

    // check an arbitrary sample in the middle
    const uint32_t sample_to_check = 11;
    const uint32_t src_idx_to_check = sample_to_check * DATA_CONVERTERS_I24_SIZE_IN_BYTES;

    // the expected q31 is a left-shifted version of the original i24
    const uint32_t expected_no_swap = arr_slice_to_i24(src, src_idx_to_check) << 8;
    const uint32_t expected = ((expected_no_swap << 16) & 0xFF000000) | ((expected_no_swap << 0) & 0x00FF0000) | ((expected_no_swap >> 16) & 0x0000FF00);

    ASSERT_EQ(dest[sample_to_check], expected);
}

TEST(DataConvertersTest, i24_to_q31_with_endian_swap_should_not_go_past_array_end)
{
    uint8_t src[DATA_CONVERTERS_Q31_AND_I24_LCM_IN_BYTES + DATA_CONVERTERS_I24_SIZE_IN_BYTES] = {
        0x00, 0x01, 0x02,
        0x03, 0x04, 0x05,
        0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B,
        0xDA, 0xDB, 0x0D};          // extra sample at the that should not be moved into the destination array
    uint32_t src_len_in_bytes = 12; // becaue the length here says to stop at after 4 samples

    q31_t dest[5] = {0, 0, 0, 0, 42}; // sentinal value at the end should not be changed

    data_converters_i24_to_q31_with_endian_swap(src, dest, src_len_in_bytes);

    ASSERT_EQ(dest[4], 42);
}

TEST(DataConvertersTest, q31_to_i24_smallest_chunk_check_all_bytes)
{
    // the smallest valid chunk is four q31 words, which is four 32 bit samples which will be crammed into 12 bytes
    const uint32_t src_len_in_samps = 4;
    q31_t src[src_len_in_samps] = {
        0x33221100,
        0x77665544,
        0xBBAA9988,
        0xFFEEDDCC};

    uint8_t dest[DATA_CONVERTERS_Q31_AND_I24_LCM_IN_BYTES] = {0};

    data_converters_q31_to_i24(src, dest, src_len_in_samps);

    ASSERT_THAT(dest, ElementsAre(
                          0x11, 0x22, 0x33,
                          0x55, 0x66, 0x77,
                          0x99, 0xAA, 0xBB,
                          0xDD, 0xEE, 0xFF));
}

TEST(DataConvertersTest, q31_to_i24_check_sample_in_the_middle)
{
    // arbitrary number of samples to check, note that the num samples must be a multiple of 3 (bytes % 12 = 0)
    const uint32_t num_samps = 18;

    q31_t src[num_samps];
    uint8_t dest[num_samps * DATA_CONVERTERS_I24_SIZE_IN_BYTES];

    // check an arbitrary sample in the middle
    const uint32_t samp_to_check = 5;
    const uint32_t dest_idx_to_check = samp_to_check * DATA_CONVERTERS_I24_SIZE_IN_BYTES;

    // set the value to an arbitrary value
    const uint32_t arb_val_big_enough_to_be_non_zero_when_shifted = 0x42424242;
    src[samp_to_check] = arb_val_big_enough_to_be_non_zero_when_shifted;

    data_converters_q31_to_i24(src, dest, num_samps);

    // the expected i24 is a right-shifted version of the original q31
    const uint32_t expected = arb_val_big_enough_to_be_non_zero_when_shifted >> 8;
    const uint32_t actual = arr_slice_to_i24(dest, dest_idx_to_check);

    ASSERT_EQ(actual, expected);
}

TEST(DataConvertersTest, q31_to_i24_should_not_go_past_array_end)
{
    q31_t src[5] = {
        0x03020100,
        0x07060504,
        0x0B0A0908,
        0x0F0E0D0C,
        0x12345678};               // this last word should not be copied into the destination array
    uint32_t src_len_in_samps = 4; // because the length here says to stop at 4 sample

    uint8_t dest[16] = {0};

    data_converters_q31_to_i24(src, dest, src_len_in_samps);

    ASSERT_EQ(dest[12], 0); // everything past here should still be zero'd out
    ASSERT_EQ(dest[13], 0);
    ASSERT_EQ(dest[14], 0);
    ASSERT_EQ(dest[15], 0);
}

TEST(DataConvertersTest, q31_to_i24_works_in_place)
{
    const uint32_t src_len_in_samps = 4;
    q31_t src[src_len_in_samps] = {
        0x33221100,
        0x77665544,
        0xBBAA9988,
        0xFFEEDDCC};

    data_converters_q31_to_i24(src, (uint8_t *)src, src_len_in_samps);

    // the three 32-bit words of src now contain four 24-bit truncated samples
    ASSERT_EQ(src[0], 0x55332211);
    ASSERT_EQ(src[1], 0xAA997766);
    ASSERT_EQ(src[2], 0xFFEEDDBB);
}

TEST(DataConvertersTest, q31_to_q15_smallest_chunk_check_all_bytes)
{
    // the smallest valid chunk is four q31 words, which is four 32 bit samples which will be crammed into 12 bytes
    const uint32_t src_len_in_samps = 4;
    q31_t src[src_len_in_samps] = {
        0x33221100,
        0x77665544,
        0xBBAA9988,
        0xFFEEDDCC};

    q15_t dest[src_len_in_samps] = {0};

    data_converters_q31_to_q15(src, dest, src_len_in_samps);

    ASSERT_THAT(dest, ElementsAre(
                          0x3322,
                          0x7766,
                          0xBBAA,
                          0xFFEE));
}

TEST(DataConvertersTest, q31_to_q15_check_sample_in_the_middle)
{
    // arbitrary number of samples to check, note that the num samples must be a multiple of 3 (bytes % 12 = 0)
    const uint32_t num_samps = 42;

    q31_t src[num_samps];
    q15_t dest[num_samps];

    // check an arbitrary sample in the middle
    const uint32_t samp_to_check = 17;
    const uint32_t dest_idx_to_check = samp_to_check;

    // set the value to an arbitrary value
    const uint32_t arb_val_big_enough_to_be_non_zero_when_shifted = 0x42424242;
    src[samp_to_check] = arb_val_big_enough_to_be_non_zero_when_shifted;

    data_converters_q31_to_q15(src, dest, num_samps);

    ASSERT_EQ(src[samp_to_check] >> 16, dest[samp_to_check]);
}

TEST(DataConvertersTest, q31_to_q15_should_not_go_past_array_end)
{
    q31_t src[5] = {
        0x03020100,
        0x07060504,
        0x0B0A0908,
        0x0F0E0D0C,
        0x12345678};               // this last word should not be copied into the destination array
    uint32_t src_len_in_samps = 4; // because the length here says to stop at 4 sample

    q15_t dest[8] = {0};

    data_converters_q31_to_q15(src, dest, src_len_in_samps);

    ASSERT_EQ(dest[4], 0); // everything past here should still be zero'd out
    ASSERT_EQ(dest[5], 0);
    ASSERT_EQ(dest[6], 0);
    ASSERT_EQ(dest[7], 0);
}

TEST(DataConvertersTest, q31_to_q15_works_in_place)
{
    const uint32_t src_len_in_samps = 4;
    q31_t src[src_len_in_samps] = {
        0x33221100,
        0x77665544,
        0xBBAA9988,
        0xFFEEDDCC};

    data_converters_q31_to_q15(src, (q15_t *)src, src_len_in_samps);

    // the first two 32-bit words of src now contain four 16-bit truncated samples
    ASSERT_EQ(src[0], 0x77663322);
    ASSERT_EQ(src[1], 0xFFEEBBAA);
}
