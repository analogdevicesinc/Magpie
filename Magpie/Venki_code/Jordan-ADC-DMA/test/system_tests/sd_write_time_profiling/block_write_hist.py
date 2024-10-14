import argparse
import matplotlib.pyplot as plt
import pandas as pd

parser = argparse.ArgumentParser()
parser.add_argument(
    "plot_details",
    type=str,
    help="Details to add to the title of the generated plot",
)

args = parser.parse_args()

df = pd.read_csv("block_write_times_microsec.csv").transpose().reset_index()
df.columns = df.iloc[0]

df.drop(df.index[0], inplace=True)
df.dropna(inplace=True)
df[df.columns] = df[df.columns].apply(pd.to_numeric, errors="coerce")

# convert all values to units of seconds
for col in df.columns:
    df[col] = df[col].apply(lambda x: x / 1000000)

t_max = max(df.max(axis=1))
t_min = min(df.min(axis=1))

# make a plot with a histogram for each sample-rate/bit-depth combo, use a 7x2 layout like this
#   24k 16bit, 24k 24bit
# ... 48k through 192k sample rates follow the same pattern
# 384k 16 bit, 384k 24bit

sample_rates = ["24k", "48k", "96k", "192k", "384k"]
bit_depths = ["16 bit", "24 bit"]

num_sample_rates = len(sample_rates)
num_bit_depths = len(bit_depths)
fig, axs = plt.subplots(num_sample_rates, num_bit_depths)

fig.suptitle(
    f"Histogram of filter & SD card write time, {df.shape[0]} blocks each. {args.plot_details}"
)

for ax, col in zip(axs[0], bit_depths):
    ax.set_title(col)

for ax, row in zip(axs[:, 0], sample_rates):
    ax.set_ylabel(row, size="large")

for i in range(num_sample_rates):
    for j in range(num_bit_depths):
        col = df.columns[(i * 2) + j]

        axs[i, j].hist(x=df[col], bins=100, range=[t_min, t_max], alpha=0.7)

        axs[i, j].axvline(x=0.0085, color="red", ls="solid", label="8.5msec limit")

        axs[i, j].axvline(x=df[col].mean(), color="blue", ls="dashed", label="mean")

        axs[i, j].axvline(
            x=df[col].median(), color="green", ls="dashdot", label="median"
        )

        axs[i, j].axvline(x=df[col].max(), color="purple", ls="dotted", label="max")

        axs[i, j].set_xlabel("Time to filter and write 1 DMA block (seconds)")

plt.legend()

for ax in fig.get_axes():
    ax.label_outer()

plt.show()
