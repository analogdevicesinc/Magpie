import config
import numpy as np


def generate_test_sig_with_approx_freqs(
    freqs: list[float],
    weights: list[float],
    num_samps: int,
) -> tuple[np.array, np.array]:
    """
    `generate_test_sig_with_approx_freqs(fs, ws, n)` is the (x, y) pair of numpy arrays representing a test signal of 1 or
    more sine componets. The x array represents the linear inputs, and the y array represents the actual sine components.

    Each frequency in the list of frequencies `fs` is snapped to a nearby frequency that evenly fits into the DMA
    buffer length defined in config.h, this means that the resulting test signal will be continuous across DMA block
    boundaries. Each frequency component is scaled by the weight in `ws` with the matching index.

    Very low frequencies (below about 50Hz) will round down to zero Hz.

    The final numpy array is normalized to the range [-1, +1]

    The sample rate and DMA buffer length defined in config.py influence the final result.

    Keyword arguments:
    freqs -- the list of frequencies to include in the test signal, frequencies may be slightly adjusted from the given values
    weights -- the weights to apply to each frequency component
    num_samps -- the length of the resulting array

    Raises:
    An exception if the lengths of `fs` and `ws` are not the same

    Examples:
    >>> x, y = generate_test_sig_with_approx_freqs(freqs=[1e3, 50e3, 120e3], weights=[1, 0.02, 0.01], num_samps=8256)
    >>> plt.plot(x, y)
    >>> plt.show()

    results in a numpy array with weighted sine components of approximately 1kHz, 50kHz, and 120kHz
    """
    if len(freqs) != len(weights):
        raise Exception("Length of frequency and weights arrays must be identical")

    # all exact multiples of df will be continuous across the block boundary
    df = config.BASE_SAMPLE_RATE / config.DMA_SIZE_IN_SAMPS

    x = np.arange(num_samps)

    y = np.zeros(num_samps)

    for i in range(len(freqs)):
        # the actual frequency is nudged so that it fits in a multiple of the DMA block size
        actual_f = df * (freqs[i] // df)

        y += weights[i] * np.sin(2 * np.pi * x * actual_f / config.BASE_SAMPLE_RATE)

    # scale to be between [-1, +1]
    y = y / np.max(abs(y))

    return x, y
