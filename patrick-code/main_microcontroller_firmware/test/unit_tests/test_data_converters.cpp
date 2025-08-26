#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test_helpers.hpp"

extern "C"
{
#include "data_converters.h"
}

using namespace testing;

/**-------------------------------------- data_converters_i24_swap_endianness --------------------------------------- */

TEST(DataConvertersTest, i24_swap_endianness_smallest_chunk_check_all_bytes)
{
    // the smallest valid chunk is 12 bytes, which is four 24 bit samples
    uint8_t src[DATA_CONVERTERS_I24_SMALLEST_VALID_CHUNK_SIZE] = {
        0x00, 0x11, 0x22, // 1st sample
        0x33, 0x44, 0x55,
        0x66, 0x77, 0x88,
        0x99, 0xAA, 0xBB}; // 4th sample

    uint8_t dest[DATA_CONVERTERS_I24_SMALLEST_VALID_CHUNK_SIZE] = {0};

    data_converters_i24_swap_endianness(src, dest, 4);

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
        0x12, 0x34, 0x56};     // extra sample at the that should not be stored in dest
    uint32_t len_in_samps = 4; // because the length here says to stop after 4 samples

    uint8_t dest[DATA_CONVERTERS_I24_SMALLEST_VALID_CHUNK_SIZE + DATA_CONVERTERS_I24_SIZE_IN_BYTES] = {0};
    dest[12] = 0xDA;
    dest[13] = 0xDB;
    dest[14] = 0x0D;

    data_converters_i24_swap_endianness(src, dest, len_in_samps);

    ASSERT_EQ(dest[12], 0xDA); // <- these should not be swapped
    ASSERT_EQ(dest[13], 0xDB);
    ASSERT_EQ(dest[14], 0x0D); // <- these should not be swapped
}

/**------------------------------ data_converters_i24_to_q31_with_endian_swap --------------------------------------- */

TEST(DataConvertersTest, i24_to_q31_with_endian_swap_smallest_chunk_check_all_bytes)
{
    // the smallest valid chunk is 12 bytes, which is four 24 bit samples which will be expanded into 4 32 bit words
    uint8_t src[DATA_CONVERTERS_Q31_AND_I24_LCM_IN_BYTES] = {
        0x11, 0x22, 0x33,
        0x44, 0x55, 0x66,
        0x77, 0x88, 0x99,
        0xAA, 0xBB, 0xCC};

    q31_t dest[4] = {0};

    data_converters_i24_to_q31_with_endian_swap(src, dest, 4);

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

    data_converters_i24_to_q31_with_endian_swap(src, dest, num_samps);

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
        0xDA, 0xDB, 0x0D};     // extra sample at the that should not be moved into the destination array
    uint32_t len_in_samps = 4; // because the length here says to stop after 4 samples

    q31_t dest[5] = {0, 0, 0, 0, 42}; // sentinal value at the end should not be changed

    data_converters_i24_to_q31_with_endian_swap(src, dest, len_in_samps);

    ASSERT_EQ(dest[4], 42);
}

/**------------------------------ data_converters_i24_to_q15_with_endian_swap --------------------------------------- */

TEST(DataConvertersTest, i24_to_q15_with_endian_swap_smallest_chunk_check_all_bytes)
{
    // the smallest valid chunk is 12 bytes, which is four 24 bit samples
    uint8_t src[DATA_CONVERTERS_I24_SMALLEST_VALID_CHUNK_SIZE] = {
        0x00, 0x11, 0x22, // 1st sample
        0x33, 0x44, 0x55,
        0x66, 0x77, 0x88,
        0x99, 0xAA, 0xBB}; // 4th sample

    q15_t dest[4] = {0};

    data_converters_i24_to_q15_with_endian_swap(src, dest, 4);

    ASSERT_THAT(dest, ElementsAre(
                          0x0011,
                          0x3344,
                          0x6677,
                          0x99AA));
}

TEST(DataConvertersTest, i24_to_q15_with_endian_swap_check_sample_in_the_middle)
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

    data_converters_i24_to_q15_with_endian_swap(src, dest, num_samps);

    // check an arbitrary sample in the middle
    const uint32_t sample_to_check = 8;
    const uint32_t src_idx_to_check = sample_to_check * DATA_CONVERTERS_I24_SIZE_IN_BYTES;

    const q15_t expected = (src[src_idx_to_check] << 8) | src[src_idx_to_check + 1];

    ASSERT_EQ(dest[sample_to_check], expected);
}

TEST(DataConvertersTest, i24_to_q15_with_endian_swap_should_not_go_past_array_end)
{
    uint8_t src[DATA_CONVERTERS_Q31_AND_I24_LCM_IN_BYTES + DATA_CONVERTERS_I24_SIZE_IN_BYTES] = {
        0x00, 0x01, 0x02,
        0x03, 0x04, 0x05,
        0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B,
        0xDA, 0xDB, 0x0D};     // extra sample at the that should not be moved into the destination array
    uint32_t len_in_samps = 4; // because the length here says to stop after 4 samples

    q15_t dest[5] = {0, 0, 0, 0, 42}; // sentinal value at the end should not be changed

    data_converters_i24_to_q15_with_endian_swap(src, dest, len_in_samps);

    ASSERT_EQ(dest[4], 42);
}

/**------------------------------ data_converters_q31_to_i24 -------------------------------------------------------- */

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
    // arbitrary number of samples to check, note that the num samples must be a multiple of 4
    const uint32_t num_samps = 20;

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

/**------------------------------ data_converters_q31_to_q15 -------------------------------------------------------- */

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
    // arbitrary number of samples to check, note that the num samples must be a multiple of 4
    const uint32_t num_samps = 44;

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

/**------------------------- data_converters_interleave_2_q31_to_q15 ------------------------------------------------ */

TEST(DataConvertersTest, data_converters_interleave_2_q31_to_q15_smallest_chunk_check_all_bytes)
{
    // Define the smallest chunk size (4 samples for each source)
    const uint32_t src_len_in_samps = 4;
    const uint32_t dest_len_in_bytes = src_len_in_samps * 2 * sizeof(q15_t);

    // Create source arrays
    q31_t src0[src_len_in_samps] = {0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00};
    q31_t src1[src_len_in_samps] = {0xFFEEDDCC, 0xBBAA9988, 0x77665544, 0x33221100};

    // Create destination array
    q15_t dest[dest_len_in_bytes / sizeof(q15_t)] = {0};

    // Call the function
    uint32_t bytes_written = data_converters_interleave_2_q31_to_q15(src0, src1, dest, src_len_in_samps);

    // Check the number of bytes written
    ASSERT_EQ(bytes_written, dest_len_in_bytes);

    ASSERT_THAT(dest, ElementsAre(
                          0x1122, 0xFFEE,
                          0x5566, 0xBBAA,
                          0x99AA, 0x7766,
                          0xDDEE, 0x3322));
}

TEST(DataConvertersTest, data_converters_interleave_2_q31_to_q15_check_samples_in_the_middle)
{
    // Define a larger sample size to have a "middle" to check
    const uint32_t src_len_in_samps = 16;
    const uint32_t dest_len_in_bytes = src_len_in_samps * 2 * sizeof(q15_t);

    // Create source arrays and fill with incrementing values
    q31_t src0[src_len_in_samps];
    q31_t src1[src_len_in_samps];
    for (uint32_t i = 0; i < src_len_in_samps; ++i)
    {
        src0[i] = (i << 16) | i;                 // Upper 16 bits: i, Lower 16 bits: i
        src1[i] = ((i + 100) << 16) | (i + 100); // Upper 16 bits: i+100, Lower 16 bits: i+100
    }

    // Create destination array
    q15_t dest[dest_len_in_bytes / sizeof(q15_t)] = {0};

    // Call the function
    uint32_t bytes_written = data_converters_interleave_2_q31_to_q15(src0, src1, dest, src_len_in_samps);

    // Check the number of bytes written
    ASSERT_EQ(bytes_written, dest_len_in_bytes);

    // Check a sample in the middle
    const uint32_t middle_sample = src_len_in_samps / 2;

    // Check src0 middle sample
    ASSERT_EQ(dest[middle_sample * 2], static_cast<q15_t>(src0[middle_sample] >> 16));

    // Check src1 middle sample
    ASSERT_EQ(dest[middle_sample * 2 + 1], static_cast<q15_t>(src1[middle_sample] >> 16));

    // Additional checks for surrounding samples
    const uint32_t check_range = 2; // Check 2 samples before and after the middle
    for (uint32_t i = middle_sample - check_range; i <= middle_sample + check_range; ++i)
    {
        ASSERT_EQ(dest[i * 2], static_cast<q15_t>(src0[i] >> 16))
            << "Mismatch in src0 at sample " << i;
        ASSERT_EQ(dest[i * 2 + 1], static_cast<q15_t>(src1[i] >> 16))
            << "Mismatch in src1 at sample " << i;
    }
}

TEST(DataConvertersTest, data_converters_interleave_2_q31_to_q15_should_not_go_past_array_end)
{
    // Define source arrays with extra elements that should not be processed
    const uint32_t src_len_in_samps = 4;
    const uint32_t extra_samples = 2;
    const uint32_t total_src_len = src_len_in_samps + extra_samples;

    q31_t src0[total_src_len] = {
        0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00,
        0xDEADBEEF, 0xCAFEBABE // Extra samples that should not be processed
    };
    q31_t src1[total_src_len] = {
        0xFFEEDDCC, 0xBBAA9988, 0x77665544, 0x33221100,
        0xFEEDFACE, 0xC0FFEE00 // Extra samples that should not be processed
    };

    // Create destination array with extra space
    const uint32_t dest_len_in_samps = src_len_in_samps * 2;
    const uint32_t dest_extra_space = 4;
    const uint32_t total_dest_len = dest_len_in_samps + dest_extra_space;
    q15_t dest[total_dest_len];

    // Initialize destination array with a known pattern
    for (uint32_t i = 0; i < total_dest_len; ++i)
    {
        dest[i] = 12345;
    }

    // Call the function with the specified length
    uint32_t bytes_written = data_converters_interleave_2_q31_to_q15(src0, src1, dest, src_len_in_samps);

    // Check the number of bytes written
    ASSERT_EQ(bytes_written, src_len_in_samps * 2 * sizeof(q15_t));

    // Check that the function did not write past the expected end of the destination array
    for (uint32_t i = dest_len_in_samps; i < total_dest_len; ++i)
    {
        ASSERT_EQ(dest[i], 12345) << "Unexpected value at index " << i;
    }

    // Check that the function did not read past the end of the source arrays
    ASSERT_EQ(src0[src_len_in_samps], 0xDEADBEEF) << "Source array 0 was modified unexpectedly";
    ASSERT_EQ(src1[src_len_in_samps], 0xFEEDFACE) << "Source array 1 was modified unexpectedly";
}

/**------------------------- data_converters_interleave_2_q31_to_i24 ------------------------------------------------ */

TEST(DataConvertersTest, data_converters_interleave_2_q31_to_i24_smallest_chunk_check_all_bytes)
{
    // Define the smallest chunk size (4 samples for each source)
    const uint32_t src_len_in_samps = 4;
    const uint32_t dest_len_in_bytes = src_len_in_samps * 2 * 3; // 3 bytes per i24 sample

    // Create source arrays
    q31_t src0[src_len_in_samps] = {0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00};
    q31_t src1[src_len_in_samps] = {0xAABBCCDD, 0xEEFF0011, 0x22334455, 0x66778899};

    // Create destination array
    uint8_t dest[dest_len_in_bytes] = {0};

    // Call the function
    uint32_t bytes_written = data_converters_interleave_2_q31_to_i24(src0, src1, dest, src_len_in_samps);

    // Check the number of bytes written
    ASSERT_EQ(bytes_written, dest_len_in_bytes);

    ASSERT_THAT(dest, ElementsAre(
                          0x33, 0x22, 0x11,
                          0xCC, 0xBB, 0xAA,
                          0x77, 0x66, 0x55,
                          0x00, 0xFF, 0xEE,
                          0xBB, 0xAA, 0x99,
                          0x44, 0x33, 0x22,
                          0xFF, 0xEE, 0xDD,
                          0x88, 0x77, 0x66));
}

TEST(DataConvertersTest, data_converters_interleave_2_q31_to_i24_check_samples_in_the_middle)
{
    // Define a larger sample size to have a "middle" to check
    const uint32_t src_len_in_samps = 16;
    const uint32_t dest_len_in_bytes = src_len_in_samps * 2 * 3; // 3 bytes per i24 sample

    // Create source arrays and fill with incrementing values
    q31_t src0[src_len_in_samps];
    q31_t src1[src_len_in_samps];
    for (uint32_t i = 0; i < src_len_in_samps; ++i)
    {
        src0[i] = (i << 24) | (i << 16) | (i << 8) | i;
        src1[i] = ((i + 100) << 24) | ((i + 100) << 16) | ((i + 100) << 8) | (i + 100);
    }

    // Create destination array
    uint8_t dest[dest_len_in_bytes] = {0};

    // Call the function
    uint32_t bytes_written = data_converters_interleave_2_q31_to_i24(src0, src1, dest, src_len_in_samps);

    // Check the number of bytes written
    ASSERT_EQ(bytes_written, dest_len_in_bytes);

    // Check a sample in the middle
    const uint32_t middle_sample = src_len_in_samps / 2;
    const uint32_t middle_byte = middle_sample * 6; // 6 bytes per interleaved sample pair

    // Check src0 middle sample
    ASSERT_EQ(dest[middle_byte], static_cast<uint8_t>(src0[middle_sample] >> 24));
    ASSERT_EQ(dest[middle_byte + 1], static_cast<uint8_t>(src0[middle_sample] >> 16));
    ASSERT_EQ(dest[middle_byte + 2], static_cast<uint8_t>(src0[middle_sample] >> 8));

    // Check src1 middle sample
    ASSERT_EQ(dest[middle_byte + 3], static_cast<uint8_t>(src1[middle_sample] >> 24));
    ASSERT_EQ(dest[middle_byte + 4], static_cast<uint8_t>(src1[middle_sample] >> 16));
    ASSERT_EQ(dest[middle_byte + 5], static_cast<uint8_t>(src1[middle_sample] >> 8));

    // Additional checks for surrounding samples
    const uint32_t check_range = 2; // Check 2 samples before and after the middle
    for (uint32_t i = middle_sample - check_range; i <= middle_sample + check_range; ++i)
    {
        uint32_t byte_index = i * 6;
        ASSERT_EQ(dest[byte_index], static_cast<uint8_t>(src0[i] >> 24))
            << "Mismatch in src0 at sample " << i << ", byte 0";
        ASSERT_EQ(dest[byte_index + 1], static_cast<uint8_t>(src0[i] >> 16))
            << "Mismatch in src0 at sample " << i << ", byte 1";
        ASSERT_EQ(dest[byte_index + 2], static_cast<uint8_t>(src0[i] >> 8))
            << "Mismatch in src0 at sample " << i << ", byte 2";
        ASSERT_EQ(dest[byte_index + 3], static_cast<uint8_t>(src1[i] >> 24))
            << "Mismatch in src1 at sample " << i << ", byte 0";
        ASSERT_EQ(dest[byte_index + 4], static_cast<uint8_t>(src1[i] >> 16))
            << "Mismatch in src1 at sample " << i << ", byte 1";
        ASSERT_EQ(dest[byte_index + 5], static_cast<uint8_t>(src1[i] >> 8))
            << "Mismatch in src1 at sample " << i << ", byte 2";
    }
}

TEST(DataConvertersTest, data_converters_interleave_2_q31_to_i24_should_not_go_past_array_end)
{
    // Define source arrays with extra elements that should not be processed
    const uint32_t src_len_in_samps = 4;
    const uint32_t extra_samples = 2;
    const uint32_t total_src_len = src_len_in_samps + extra_samples;

    q31_t src0[total_src_len] = {
        0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00,
        0xDEADBEEF, 0xCAFEBABE // Extra samples that should not be processed
    };
    q31_t src1[total_src_len] = {
        0xAABBCCDD, 0xEEFF0011, 0x22334455, 0x66778899,
        0xFEEDFACE, 0xC0FFEE00 // Extra samples that should not be processed
    };

    // Create destination array with extra space
    const uint32_t dest_len_in_bytes = src_len_in_samps * 2 * 3; // 3 bytes per i24 sample
    const uint32_t dest_extra_space = 6;
    const uint32_t total_dest_len = dest_len_in_bytes + dest_extra_space;
    uint8_t dest[total_dest_len];

    // Initialize destination array with a known pattern
    for (uint32_t i = 0; i < total_dest_len; ++i)
    {
        dest[i] = 0xAA;
    }

    // Call the function with the specified length
    uint32_t bytes_written = data_converters_interleave_2_q31_to_i24(src0, src1, dest, src_len_in_samps);

    // Check the number of bytes written
    ASSERT_EQ(bytes_written, dest_len_in_bytes);

    // Check that the function did not write past the expected end of the destination array
    for (uint32_t i = dest_len_in_bytes; i < total_dest_len; ++i)
    {
        ASSERT_EQ(dest[i], 0xAA) << "Unexpected value at index " << i;
    }

    // Check that the function did not read past the end of the source arrays
    ASSERT_EQ(src0[src_len_in_samps], 0xDEADBEEF) << "Source array 0 was modified unexpectedly";
    ASSERT_EQ(src1[src_len_in_samps], 0xFEEDFACE) << "Source array 1 was modified unexpectedly";
}

/**------------------------- data_converters_interleave_2_i24_and_swap_endianness ----------------------------------- */

TEST(DataConvertersTest, interleave_2_i24_and_swap_endianness_smallest_chunk_check_all_bytes)
{
    // Define the smallest chunk size (4 samples for each source)
    const uint32_t src_len_in_samps = 4;
    const uint32_t dest_len_in_bytes = src_len_in_samps * 2 * 3; // 3 bytes per i24 sample

    // Create source arrays
    uint8_t src0[12] = {
        0x11, 0x22, 0x33, // Sample 0
        0x44, 0x55, 0x66, // Sample 1
        0x77, 0x88, 0x99, // Sample 2
        0xAA, 0xBB, 0xCC  // Sample 3
    };

    uint8_t src1[12] = {
        0xDD, 0xEE, 0xFF, // Sample 0
        0x12, 0x34, 0x56, // Sample 1
        0x78, 0x9A, 0xBC, // Sample 2
        0xDE, 0xF0, 0x12  // Sample 3
    };

    // Create destination array
    uint8_t dest[dest_len_in_bytes] = {0};

    // Call the function
    uint32_t bytes_written = data_converters_interleave_2_i24_and_swap_endianness(src0, src1, dest, src_len_in_samps);

    // Check the number of bytes written
    ASSERT_EQ(bytes_written, dest_len_in_bytes);

    ASSERT_THAT(dest, ElementsAre(
                          0x33, 0x22, 0x11, 0xFF, 0xEE, 0xDD, // Sample pair 0
                          0x66, 0x55, 0x44, 0x56, 0x34, 0x12, // Sample pair 1
                          0x99, 0x88, 0x77, 0xBC, 0x9A, 0x78, // Sample pair 2
                          0xCC, 0xBB, 0xAA, 0x12, 0xF0, 0xDE  // Sample pair 3
                          ));
}

TEST(DataConvertersTest, interleave_2_i24_and_swap_endianness_check_samples_in_the_middle)
{
    // Define a larger sample size to have a "middle" to check
    const uint32_t src_len_in_samps = 16;
    const uint32_t dest_len_in_bytes = src_len_in_samps * 2 * 3; // 3 bytes per i24 sample

    // Create source arrays and fill with incrementing values
    uint8_t src0[src_len_in_samps * 3];
    uint8_t src1[src_len_in_samps * 3];

    // Fill arrays with recognizable patterns
    for (uint32_t i = 0; i < src_len_in_samps; i++)
    {
        uint32_t base_idx = i * 3;
        // src0 pattern: each sample is (i, i+1, i+2)
        src0[base_idx] = i;
        src0[base_idx + 1] = i + 1;
        src0[base_idx + 2] = i + 2;

        // src1 pattern: each sample is (100+i, 101+i, 102+i)
        src1[base_idx] = 100 + i;
        src1[base_idx + 1] = 101 + i;
        src1[base_idx + 2] = 102 + i;
    }

    // Create destination array
    uint8_t dest[dest_len_in_bytes] = {0};

    // Call the function
    uint32_t bytes_written = data_converters_interleave_2_i24_and_swap_endianness(src0, src1, dest, src_len_in_samps);

    // Check the number of bytes written
    ASSERT_EQ(bytes_written, dest_len_in_bytes);

    // Check a sample in the middle
    const uint32_t middle_sample = src_len_in_samps / 2;
    const uint32_t middle_byte = middle_sample * 6; // 6 bytes per interleaved sample pair

    // Check src0 middle sample (should be endian swapped)
    ASSERT_EQ(dest[middle_byte], middle_sample + 2);     // MSB
    ASSERT_EQ(dest[middle_byte + 1], middle_sample + 1); // Middle byte
    ASSERT_EQ(dest[middle_byte + 2], middle_sample);     // LSB

    // Check src1 middle sample (should be endian swapped)
    ASSERT_EQ(dest[middle_byte + 3], middle_sample + 102); // MSB
    ASSERT_EQ(dest[middle_byte + 4], middle_sample + 101); // Middle byte
    ASSERT_EQ(dest[middle_byte + 5], middle_sample + 100); // LSB
}

TEST(DataConvertersTest, interleave_2_i24_and_swap_endianness_should_not_go_past_array_end)
{
    const uint32_t src_len_in_samps = 4;
    const uint32_t extra_samples = 2;
    const uint32_t total_src_len = (src_len_in_samps + extra_samples) * 3; // *3 for i24

    uint8_t src0[total_src_len] = {
        0x11, 0x22, 0x33, // Sample 0
        0x44, 0x55, 0x66, // Sample 1
        0x77, 0x88, 0x99, // Sample 2
        0xAA, 0xBB, 0xCC, // Sample 3
        0xDD, 0xEE, 0xFF, // Extra sample that should not be processed
        0x00, 0x11, 0x22  // Extra sample that should not be processed
    };

    uint8_t src1[total_src_len] = {
        0x33, 0x44, 0x55, // Sample 0
        0x66, 0x77, 0x88, // Sample 1
        0x99, 0xAA, 0xBB, // Sample 2
        0xCC, 0xDD, 0xEE, // Sample 3
        0xFF, 0x00, 0x11, // Extra sample that should not be processed
        0x22, 0x33, 0x44  // Extra sample that should not be processed
    };

    // Create destination array with extra space
    const uint32_t dest_len_in_bytes = src_len_in_samps * 2 * 3; // 3 bytes per i24 sample
    const uint32_t dest_extra_space = 6;
    const uint32_t total_dest_len = dest_len_in_bytes + dest_extra_space;
    uint8_t dest[total_dest_len];

    // Initialize destination array with a known pattern
    for (uint32_t i = 0; i < total_dest_len; i++)
    {
        dest[i] = 0xAA;
    }

    // Call the function with the specified length
    uint32_t bytes_written = data_converters_interleave_2_i24_and_swap_endianness(src0, src1, dest, src_len_in_samps);

    // Check the number of bytes written
    ASSERT_EQ(bytes_written, dest_len_in_bytes);

    // Check that the function did not write past the expected end of the destination array
    for (uint32_t i = dest_len_in_bytes; i < total_dest_len; i++)
    {
        ASSERT_EQ(dest[i], 0xAA) << "Unexpected value at index " << i;
    }

    // Verify the last valid samples were written correctly
    ASSERT_EQ(dest[dest_len_in_bytes - 6], 0xCC); // Last sample from src0
    ASSERT_EQ(dest[dest_len_in_bytes - 5], 0xBB);
    ASSERT_EQ(dest[dest_len_in_bytes - 4], 0xAA);
    ASSERT_EQ(dest[dest_len_in_bytes - 3], 0xEE); // Last sample from src1
    ASSERT_EQ(dest[dest_len_in_bytes - 2], 0xDD);
    ASSERT_EQ(dest[dest_len_in_bytes - 1], 0xCC);
}

/**------------------------- data_converters_interleave_2_i24_to_q15_and_swap_endianness ---------------------------- */

TEST(DataConvertersTest, interleave_2_i24_to_q15_and_swap_endianness_smallest_chunk_check_all_bytes)
{
    // Test with 4 samples (one complete block)
    const uint32_t src_len_in_samps = 4;

    // Input data for source 0. Only upper 16 bits are kept after conversion
    uint8_t src0[] = {
        0x00, 0x11, 0x22,
        0x33, 0x44, 0x55,
        0x66, 0x77, 0x88,
        0x99, 0xAA, 0xBB};

    // Input data for source 1
    uint8_t src1[] = {
        0xCC, 0xDD, 0xEE,
        0xFF, 0x12, 0x34,
        0x45, 0x67, 0x89,
        0xAB, 0xCD, 0xDE};

    // Output buffer (8 samples * 2 bytes per sample)
    q15_t dest[8] = {0};

    uint32_t result = data_converters_interleave_2_i24_to_q15_and_swap_endianness(
        src0, src1, dest, src_len_in_samps);

    // Check return value (should be number of bytes written)
    ASSERT_EQ(result, src_len_in_samps * 2 * sizeof(q15_t));

    ASSERT_THAT(dest, ElementsAre(
                          0x0011,
                          0xCCDD,
                          0x3344,
                          0xFF12,
                          0x6677,
                          0x4567,
                          0x99AA,
                          0xABCD));
}
