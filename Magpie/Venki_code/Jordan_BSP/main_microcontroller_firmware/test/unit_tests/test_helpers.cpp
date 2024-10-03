
#include "test_helpers.hpp"

uint16_t arr_slice_to_u16(char *arr, uint32_t starting_pos)
{
    const uint8_t ls_byte = arr[starting_pos + 0];
    const uint8_t ms_byte = arr[starting_pos + 1];

    return (ms_byte << 8) | ls_byte;
}

uint32_t arr_slice_to_u32(char *arr, uint32_t starting_pos)
{
    const uint8_t b0 = arr[starting_pos + 0];
    const uint8_t b1 = arr[starting_pos + 1];
    const uint8_t b2 = arr[starting_pos + 2];
    const uint8_t b3 = arr[starting_pos + 3];

    return (b3 << 24) | (b2 << 16) | (b1 << 8) | (b0);
}

uint32_t arr_slice_to_i24(uint8_t *arr, uint32_t starting_pos)
{
    const uint8_t ls_byte = arr[starting_pos + 0];
    const uint8_t mids_byte = arr[starting_pos + 1];
    const uint8_t ms_byte = arr[starting_pos + 2];

    return 0x00FFFFFF & ((ms_byte << 16) | (mids_byte << 8) | (ls_byte));
}
