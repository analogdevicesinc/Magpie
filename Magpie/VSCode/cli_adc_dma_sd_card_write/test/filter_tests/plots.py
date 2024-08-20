import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

import config


def plot_decimation_comparison(
    raw_y,
    decimated_y,
    raw_sample_rate,
    decimated_sample_rate,
):
    """
    `plot_decimation_comparison(ry, dy, rsr, dsr)` plots out a comparison between raw, unfiltered array `ry` with sample
    rate `rsr` and decimated array `dy` with sample rate `dsr`. It is expected that `dy` is a decimated version of `ry`
    """
    raw_max = max(raw_y)

    # make copies of the input arrays when normalizing so we don't mutate them while processing
    raw_y_ = raw_y / raw_max
    decimated_y_ = decimated_y / raw_max

    decimation_factor = int(raw_sample_rate // decimated_sample_rate)

    fig, axs = plt.subplots(nrows=2, ncols=2)

    fig.suptitle(
        f"Comparison of synthetic test data before and after {decimation_factor}:1 decimation"
    )

    axs[0, 0].set_title(f"Raw test Data, {int(raw_sample_rate//1000)}kHz, undecimated")

    lowest_peak = 1e3

    about_3_cycles_of_test_data = raw_y_[: int((raw_sample_rate * 3) / lowest_peak)]

    axs[0, 0].plot(
        np.arange(len(about_3_cycles_of_test_data)), about_3_cycles_of_test_data
    )
    axs[0, 0].set_xlabel("Sample")
    axs[0, 0].set_ylabel("Amplitude")
    axs[0, 0].set_ylim(-1, 1)
    axs[0, 0].grid(True)

    axs[1, 0].magnitude_spectrum(raw_y_, raw_sample_rate, scale="dB")
    axs[1, 0].set_xlabel("Frequency (Hz)")
    axs[1, 0].xaxis.set_major_formatter(ticker.EngFormatter())
    axs[1, 0].set_ylim(-120, 5)
    axs[1, 0].grid(True)

    axs[0, 1].set_title(
        f"Test data after decimation to {int(decimated_sample_rate//1000)}kHz"
    )

    # skip past the first few samples, because these will have a discontinuity
    about_3_cycles_of_decimated_data = decimated_y_[
        config.DMA_SIZE_IN_SAMPS : config.DMA_SIZE_IN_SAMPS
        + int((decimated_sample_rate * 3) / lowest_peak)
    ]

    axs[0, 1].plot(
        np.arange(len(about_3_cycles_of_decimated_data)),
        about_3_cycles_of_decimated_data,
    )
    axs[0, 1].set_xlabel("Sample")
    axs[0, 1].set_ylabel("Amplitude")
    axs[0, 1].set_ylim(-1, 1)
    axs[0, 1].grid(True)

    axs[1, 1].magnitude_spectrum(decimated_y_, decimated_sample_rate, scale="dB")
    axs[1, 1].set_xlabel("Frequency (Hz)")
    axs[1, 1].xaxis.set_major_formatter(ticker.EngFormatter())
    # limit the x-range, this is due to the way the decimation filters are coded, go just past the passband
    axs[1, 1].set_xlim(0, decimated_sample_rate * 5 / 12 + 1e3)
    axs[1, 1].set_ylim(-120, 5)
    axs[1, 1].grid(True)

    plt.show()
