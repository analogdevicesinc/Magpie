import config
import numpy as np


def snap_freq_to_DMA_buff_multiple(freq):
    """
    `snap_freq_to_DMA_buff_multiple(f)` is a frequency close to `f` that fits into the DMA buffer such that it is
    continuous across DMA buffer boundaries. Very low frequencies below about 50Hz will be rounded down to zero Hz.
    """

    # all exact multiples of df will be continuous across the block boundary
    df = config.BASE_SAMPLE_RATE / config.DMA_SIZE_IN_SAMPS
    snapped_freq = df * (freq // df)
    return snapped_freq


def generate_test_sig(sample_rate, num_samps, include_sig_at_passband_edge=True):
    """
    `generate_test_sig(sr, n, pb)` is a numpy array filled with sine components for use as a test signal for the
    decimation filters. The test array has a reference sine component at 1kHz, and a swarm of out of band components.
    An optional signal right at the passband edge can be included as well.
    """
    # due to the way the decimation filters are coded
    fpass = sample_rate * 5 / 12
    fstop = sample_rate - fpass

    x = np.arange(num_samps)

    # a reference sine at 1kHz
    vin_1k = np.sin(
        2 * np.pi * x * snap_freq_to_DMA_buff_multiple(1e3) / config.BASE_SAMPLE_RATE
    )

    # multiple out-of-band sine components
    vin_out_of_band = np.zeros(num_samps)
    for f in range(int(fstop + 2e3), int(192e3), int(1e3)):
        vin_out_of_band = vin_out_of_band + np.sin(
            (
                2
                * np.pi
                * x
                * snap_freq_to_DMA_buff_multiple(f)
                / config.BASE_SAMPLE_RATE
            )
            # since all components align to DMA block boundary, add some random phase shift
            # so the time-domain signals don't all hit zero volts at the exact same instant
            + ((np.random.random_sample() - 0.5) * 4 * np.pi)
        )
    vin_out_of_band = vin_out_of_band / max(abs(vin_out_of_band))

    test_data = vin_1k + vin_out_of_band

    if include_sig_at_passband_edge:
        # a sine component right at the passband edge
        vin_at_passband_edge = np.sin(
            2
            * np.pi
            * x
            * snap_freq_to_DMA_buff_multiple(fpass)
            / config.BASE_SAMPLE_RATE
        )
        test_data += vin_at_passband_edge

    test_data = test_data / max(abs(test_data))

    return test_data
