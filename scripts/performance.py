import matplotlib.pyplot as plt
import matplotlib
import numpy as np
import pandas as pd
import seaborn as sns
import argparse
import sys
import os
from enum import Enum
from scipy import stats
import itertools as it

class EventType(Enum):
    ADD = 0
    UPDATE = 1
    CANCEL = 2

EventTypeStrings = [
    "Add",
    "Update",
    "Cancel"
]

def reject_outliers(data, m=2):
    return data[abs(data - np.mean(data)) < m * np.std(data)]

def draw_event_type_plot(ax, data):
    sns.boxplot(data=data, x="EventType", y="Nanoseconds", ax=ax)
    ax.set_title("Duration by Event Type (ns)")
    # ax.set_yscale("log")

def load_performance_file(path, save_to_file, remove_outliers, threshold_z):
    data = pd.read_csv(path)

    out = data.head()
    out_name = None
    if save_to_file:
        base_name_parts = os.path.basename(path).split(".")
        out_dir = os.path.dirname(path)
        base_name = base_name_parts[0]
        out_name = out_dir + os.sep + base_name + ".png"

    # fig, ax = plt.subplots(1,2, figsize=(15,15))
    fig, (ax1, ax2, ax3) = plt.subplots(1, 3, figsize=(15,15))
    fig, (ax1, ax3) = plt.subplots(1, 2, figsize=(15,15))
    
    draw_event_type_plot(ax1, data)

    adds = data[data['EventType'] == 0]
    updates = data[data['EventType'] == 1]
    cancels = data[data['EventType'] == 2]

    if remove_outliers:
        z_add = np.abs(stats.zscore(adds['Count']))
        z_update = np.abs(stats.zscore(updates['Count']))
        z_cancel = np.abs(stats.zscore(cancels['Count']))

        add_outliers = np.where(z_add > threshold_z)[0]
        adds = data.drop(add_outliers)
        adds = adds[adds['EventType'] == 0]

        update_outliers = np.where(z_update > threshold_z)[0]
        updates = data.drop(update_outliers)
        updates = updates[updates['EventType'] == 1]

        cancel_outliers = np.where(z_cancel > threshold_z)[0]
        cancels = data.drop(cancel_outliers)
        cancels = cancels[cancels['EventType'] == 2]

    has_adds = len(adds) > 0
    has_updates = len(updates) > 0
    has_cancels = len(cancels) > 0

    #  We want to filter out outliers for a better plot
    stddev_add = np.std(adds.Count) if adds is not None else 0
    max_add_cycles = max(adds.Count) if adds is not None else 0
    mean_add_cycles = adds.Count.mean() if adds is not None else 0

    stddev_update = np.std(updates.Count) if updates is not None else 0
    max_update_cycles = max(updates.Count) if updates is not None else 0
    mean_update_cycles = updates.Count.mean() if updates is not None else 0
    
    stddev_cancel = np.std(cancels.Count) if cancels is not None else 0
    max_cancel_cycles = max(cancels.Count) if cancels is not None else 0
    mean_cancel_cycles = cancels.Count.mean() if cancels is not None else 0

    binwidth = 20000
    # if has_adds:
    #     ax2.plot(adds.I, adds.Count, 'g-^', label='Adds')
    #     # plt.hist(adds.Count, color='green', label='Adds', bins=np.arange(min(adds.Count), max(adds.Count) + binwidth, binwidth))
    
    # if has_updates:
    #     ax2.plot(updates.I, updates.Count, 'b-o', label='Updates')
    #     # plt.hist(updates.Count, color="blue", label='Updates', bins=np.arange(min(updates.Count), max(updates.Count) + binwidth, binwidth))
    
    # if has_cancels:
    #     ax2.plot(cancels.I, cancels.Count, 'r-x', label='Cancels')
    #     # plt.hist(updates.Count, color='red', label='Updates', bins=np.arange(min(updates.Count), max(updates.Count) + binwidth, binwidth))

    first_legend = plt.legend()
    text_box_width = 0
    add_data_strings = [
        r'0 - Add: %d' % (len(adds)),
        r'$\mu=%.3f$' % (mean_add_cycles),
        r'$\sigma=%.3f$' % (stddev_add),
    ]

    update_data_strings = [
        r'1 - Update: %d' % (len(updates)),
        r'$\mu=%.3f$' % (mean_update_cycles),
        r'$\sigma=%.3f$' % (stddev_update)
    ]

    cancel_data_strings = [
        r'2 - Cancel: %d' % (len(cancels)),
        r'$\mu=%.3f$' % (mean_cancel_cycles),
        r'$\sigma=%.3f$' % (stddev_cancel)
    ]

    space = [25*'-']

    add_data = '\n'.join((
        add_data_strings
    ))

    text_box_strings = add_data_strings + space + update_data_strings + space + cancel_data_strings
    text_box_data = '\n'.join((
        text_box_strings
    ))

    font_size = 12
    # text_box_width = max([ax.text(0, 0, s, fontsize=font_size).get_window_extent(renderer=fig.canvas.get_renderer()).width \
    #     for s in text_box_strings])
    ax1.text(0.992, 0.94, text_box_data, transform=ax1.transAxes, color='#2d841c', verticalalignment='top', horizontalalignment='right',
        bbox=dict(facecolor='none', edgecolor='#2d841c', boxstyle='round'))
        
    if remove_outliers:
        outlier_data = ''.join((
            r'Filter level: %d $\sigma$' %(threshold_z)
        ))
        ax1.text(0.992, 0.80, outlier_data, transform=ax1.transAxes, color='black', horizontalalignment='right', verticalalignment='top',
        bbox=dict(facecolor='none', edgecolor='black', boxstyle='round'))

    plt_title = "Performance for "
    if has_adds:
        plt_title += " Adds(0) "
    if has_updates:
        plt_title += " Updates(1) "
    if has_cancels:
        plt_title += " Cancels(2)"

    plt_title +=  "(Clock Cycles)"
    # ax2.set_title(plt_title)
    # ax2.set_ylabel('Clock Cycles')

    grouped = data.groupby("EventType")["Nanoseconds"].sum().sort_values(ascending=False)
    grouped.plot(kind="bar", title="Total Time by Event Type", ylabel="Nanoseconds", ax=ax3)

    # ax1.title(plt_title)
    # ax1.set_ylabel('Clock Cycles')
    # ax1.xlabel('I')

    plt.tight_layout()
    ax1.legend(loc='best')
    # ax2.legend(loc='best')
    ax3.legend(loc='best')
    if save_to_file:
        plt.savefig(out_name)
    else:
        plt.show()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', '--file')
    parser.add_argument('-w', '--write-to-file', type=bool, default=True)
    parser.add_argument('-c', '--clean-outliers', type=bool, default=True)
    parser.add_argument('-z', '--zfactor', type=int, default=2)

    args = parser.parse_args()
    save_to_file = args.write_to_file
    remove_outliers = args.clean_outliers
    z_factor = args.zfactor
    if args.file is None and save_to_file:
        print(f'Requires an output file path')
        sys.exit(1)
    load_performance_file(args.file, save_to_file, remove_outliers, z_factor)

if __name__ == "__main__":
    main()