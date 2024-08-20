
#include <stdint.h>

/**
 * @brief `arr_slice_to_u16(a, p)` is the u16 resulting from the two bytes in `a` starting at position `p`
 *
 * @param arr the array to extract a u16 from, must be at least `pos + 1` bytes long
 *
 * @param starting_pos the array index to start at when extracting the u16
 *
 * @retval the 16 bit integer given by `(arr[p+1] << 8) | arr[p]`
 */
uint16_t arr_slice_to_u16(char *arr, uint32_t starting_pos);

/**
 * @brief `arr_slice_to_u32(a, p)` is the u32 resulting from the four bytes in `a` starting at position `p`
 *
 * @param arr the array to extract a u32 from, must be at least `pos + 3` bytes long
 *
 * @param starting_pos the array index to start at when extracting the u32
 *
 * @retval the 32 bit integer given by `(arr[p+3] << 24) | (arr[p+2] << 16) | (arr[p+1] << 8) | arr[p]`
 */
uint32_t arr_slice_to_u32(char *arr, uint32_t starting_pos);

/**
 * @brief `arr_slice_to_i24(a, p)` is the i24 resulting from the three bytes in `a` starting at position `p`
 *
 * @param arr the array to extract a i24 from, must be at least `pos + 2` bytes long
 *
 * @param starting_pos the array index to start at when extracting the i24
 *
 * @retval the 24 bit integer given by `(arr[p+2] << 16) | (arr[p+1] << 8) | arr[p]`, note that the ms byte is always 0
 */
uint32_t arr_slice_to_i24(uint8_t *arr, uint32_t starting_pos);
