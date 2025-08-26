import argparse
import config
import ctypes
import numpy as np
import scipy

from plots import plot_decimation_comparison
from test_data_generator import generate_test_sig

parser = argparse.ArgumentParser()
parser.add_argument(
    "clib",
    type=str,
    help="Path to the compiled C library of filter functions to test",
)
parser.add_argument(
    "sample_rate",
    type=int,
    help="The sample rate in kHz, ex: 192 => 192kHz",
    choices={24, 48, 96, 192},
)
parser.add_argument(
    "test_pb",
    type=int,
    help="Boolean signal, 1 to include a sine component right at the passband, 0 to leave it out",
)

args = parser.parse_args()

# load the compiled C-library and set appropriate data types for the function args
clib = ctypes.cdll.LoadLibrary(args.clib)
c_i32_p = ctypes.POINTER(ctypes.c_int32)
clib.decimation_filter_set_sample_rate.argtypes = [ctypes.c_uint]  # sample rate in Hz
clib.decimation_filter_downsample.argtypes = [
    c_i32_p,  # src
    c_i32_p,  # dest
    ctypes.c_uint,  # src_len_in_bytes
]
clib.decimation_filter_downsample.restype = ctypes.c_uint


decimated_sample_rate = int(args.sample_rate * 1000)

raw_y = generate_test_sig(
    decimated_sample_rate,
    config.DMA_SIZE_IN_SAMPS,
    include_sig_at_passband_edge=args.test_pb == 1,
)

# expand the test data to fill up some of the range of an i32, but don't go all the way to full-scale
raw_y = raw_y * 2**30
raw_y = raw_y.astype(np.int32)

decimation_factor = config.BASE_SAMPLE_RATE / decimated_sample_rate

num_dma_chunks = 100

len_of_decimated_chunk = int(config.DMA_SIZE_IN_SAMPS // decimation_factor)

len_of_final_arr = int(num_dma_chunks * len_of_decimated_chunk)

src = (ctypes.c_int32 * len(raw_y))(*raw_y)
intermediate_dest = (ctypes.c_int32 * len(raw_y))()
decimated_y = np.zeros(len_of_final_arr).astype(np.int32)

clib.decimation_filter_set_sample_rate(decimated_sample_rate)

# copy more than one buffer worth of data into the output buffer, so if we listen to a WAV file it is long enough to hear
for i in range(num_dma_chunks):
    actual_dest_len = clib.decimation_filter_downsample(
        src,
        intermediate_dest,
        len(raw_y),
    )

    offset = int((i * config.DMA_SIZE_IN_SAMPS) / decimation_factor)

    for j in range(len_of_decimated_chunk):
        decimated_y[j + offset] = intermediate_dest[j]

scipy.io.wavfile.write(
    f"./out/{int(decimated_sample_rate//1000)}kHz_{int(decimation_factor)}_to_1_decimation.wav",
    decimated_sample_rate,
    decimated_y,
)

plot_decimation_comparison(
    raw_y,
    decimated_y,
    config.BASE_SAMPLE_RATE,
    decimated_sample_rate,
)
