/* Private includes --------------------------------------------------------------------------------------------------*/

#include "wav_header.h"
#include <stdint.h>

/* Private defines ---------------------------------------------------------------------------------------------------*/

// This value depends on whether or not there is the extra 2 bytes after the bits_per_sample field and before "data"
// If the extra two bytes are there, this must be 18, if not it must be 16
#define WAVE_HEADER_FMT_CHUNK_SIZE (16)

// Always use PCM format for now. If we implement compression in the future this may change
#define WAVE_HEADER_FMT_TAG_PCM (1)

/* Private types -----------------------------------------------------------------------------------------------------*/

/**
 * @brief A structure for holding wav file header information is represented here.
 *
 * This can be cast to a char* and written directly to disk.
 */
typedef struct __attribute__((packed))
{
    char riff[4];              /* always the string "RIFF" */
    uint32_t file_len_minus_8; /* file length in bytes - 8 bytes */
    char wave[4];              /* always the string "WAVE" */
    char fmt_[4];              /* always the string "fmt " (note the trailing space) */
    uint32_t fmt_chunk_size;   /* size of FMT chunk in bytes, usually 16 or 18 */
    uint16_t fmt_tag;          /* 1=PCM, 257=Mu-Law, 258=A-Law, 259=ADPCM  */
    uint16_t num_channels;     /* 1=mono, 2=stereo */
    uint32_t sample_rate;      /* samples per second */
    uint32_t bytes_per_sec;    /* bytes per second = sample_rate * bytes_per_sample */
    uint16_t bytes_per_block;  /* num channels * bytes per sample */
    uint16_t bits_per_sample;  /* number of bits per sample */
    char data[4];              /* always the string "data" */
    uint32_t data_length;      /* data length in bytes (file_length - the length of this struct) */
    char pad_to_sector[512-44]; // add 0-valued bytes to sector boundary (512).
    // Note that (512-44) is divisible by both 2 and 3, so the
    // word alignment for 16 bits (2 bytes) and 24 bits (3 bytes) will be correct once
    // the data appears starting at 512
} Wave_Header_t;

/* Private variables -------------------------------------------------------------------------------------------------*/

// we use one static instance of Wave_Header_t and update its fields using the wav_header_set_attributes(a) function
static Wave_Header_t wave_header = {
    // Some attributes of the wave header never change. We set them once here, and then never update them again.
    // Other attributes (file length, sample rate, etc) do change dynamically via calls to wav_header_set_attributes(a)
    .riff = {'R', 'I', 'F', 'F'},
    .wave = {'W', 'A', 'V', 'E'},
    .fmt_ = {'f', 'm', 't', ' '},
    .fmt_chunk_size = WAVE_HEADER_FMT_CHUNK_SIZE,
    .fmt_tag = WAVE_HEADER_FMT_TAG_PCM,
    .data = {'d', 'a', 't', 'a'},
    .pad_to_sector = {0}
};

const uint32_t HEADER_LENGTH = sizeof(wave_header);

/* Public function definitions ---------------------------------------------------------------------------------------*/

void wav_header_set_attributes(Wave_Header_Attributes_t *attributes)
{
    wave_header.file_len_minus_8 = attributes->file_length - 8;
    wave_header.num_channels = attributes->num_channels;
    wave_header.sample_rate = attributes->sample_rate;
    wave_header.bytes_per_block = (attributes->bits_per_sample / 8) * wave_header.num_channels;
    wave_header.bytes_per_sec = wave_header.bytes_per_block * wave_header.sample_rate;
    wave_header.bits_per_sample = attributes->bits_per_sample;
    wave_header.data_length = attributes->file_length - HEADER_LENGTH;
}

char *wav_header_get_header()
{
    // cast the struct as an array of bytes so we can write it directly to the SD card
    return (char *)&wave_header;
}

uint32_t wav_header_get_header_length()
{
    return HEADER_LENGTH;
}
