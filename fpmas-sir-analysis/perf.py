
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path
import sys
import operator
import argparse
import csv
import re

'''
Reads performance data from "perf.%r.csv" files (used by the read_data() method).

The directory structure of `jobs_dir` must be as follows:
    - <job_id_1>
        - ... (other outputs)
        - perf.%r.csv
    - ...
    - <job_id_n>
        - ... (other outputs)
        - perf.%r.csv
    - ...

Data is filled with `jobs_dir` so that for each label in `perf.%r.csv`,
data[label] = [(mean, min, max),...].

Each tuple corresponds to results produce by a job.  For each job, the dataset
corresponding to `label` for each rank (i.e. for each `perf.%r.csv` file) is
built. Then the mean, minimum and maximum values of this dataset are stored in
a tuple and appended to `data[label]`.

@param data dictionnary entry corresponding to [n_agent][k][mode][n_proc]
@param job_ids_dir Directory containing job ids subdirectories 
'''
def read_perf(data, num_proc, jobs_dir):
    # Stores <label>: [list_of_values], with a value for each rank
    for job_id_dir in jobs_dir.iterdir():
        values = {}
        with open(job_id_dir / ("perf.0.csv")) as perf_file:
            csv_data = csv.DictReader(perf_file)
            for label in csv_data.__next__().keys():
                if label not in data:
                    data[label] = []
                values[label] = []

        for rank in range(0, num_proc) :
            with open(job_id_dir / ("perf." + str(rank) + ".csv")) as perf_file:
                csv_data = csv.DictReader(perf_file)
                # only one row in practice
                for row in csv_data:
                    for key in row.keys():
                        values[key].append(int(row[key]))

        for key in values.keys():
            if re.match(r".*time.*", key):
                data[key].append((
                        sum(values[key]) / num_proc,
                        min(values[key]),
                        max(values[key])
                        ))
            elif re.match(r".*count.*", key):
                data[key].append((
                        sum(values[key]),
                        sum(values[key]),
                        sum(values[key])
                        ))


'''
Reads raw data from the input directory.
'''
def read_data(root_dir, k_values, n_cities):
    root_path = Path(root_dir)
    num_agent_dirs = [num_agent_dir for num_agent_dir in root_path.iterdir() \
            if num_agent_dir.is_dir()]

    data = {}
    for num_agent_dir in num_agent_dirs:
        num_agent = int(num_agent_dir.stem)
        if n_cities==None or len(n_cities)==0 or num_agent in n_cities:
            data[num_agent]={}

            k_dirs = [k_dir for k_dir in num_agent_dir.iterdir()\
                    if num_agent_dir.is_dir()]
            for k_dir in k_dirs:
                k = int(k_dir.stem)
                if k_values==None or len(k_values)==0 or k in k_values:
                    data[num_agent][k] = {}

                    mode_dirs = [mode_dir for mode_dir in k_dir.iterdir() \
                            if k_dir.is_dir()]

                    for mode_dir in mode_dirs:
                        mode = mode_dir.stem
                        data[num_agent][k][mode] = {}

                        num_proc_dirs = [num_proc_dir for num_proc_dir in mode_dir.iterdir() \
                                if num_proc_dir.is_dir()]
                        for num_proc_dir in num_proc_dirs :
                            num_proc = int(num_proc_dir.stem)
                            data[num_agent][k][mode][num_proc] = {}

                            read_perf(\
                                    data[num_agent][k][mode][num_proc],\
                                    num_proc, num_proc_dir)

    return data

'''
Sorts a dict entries by key.
'''
def sort_by_key(input_dict):
    return sorted(input_dict.items(), key=operator.itemgetter(0))

'''
`raw_data` is a dictionnay with the following shape:
  - <n_agent>
    - <k>
      - <mode>
        - <num_proc>
          - <label>
            - [(mean, min, max), ...]
with the last list containing a tuple for each job.

We want to restructure this dataset so that is as the following shape, so that
it can be passed to matplotlib:
  - <n_agent>
    - <k>
      - <mode>
        - <label>
          - ([num_procs, ...], [mean, ...], [min, ...], [max, ...])

with the last tuple containing list with one item for each num_proc. To do so,
mean values are computed on previous job items.
'''
def build_datasets(raw_data):
    dataset = {}
    for (n_agent, k_values) in sort_by_key(raw_data):
        dataset[n_agent] = {}
        for (k_value, modes) in sort_by_key(k_values):
            dataset[n_agent][k_value] = {}
            for (mode, num_procs) in modes.items():
                dataset[n_agent][k_value][mode] = {}
                node = dataset[n_agent][k_value][mode]
                for (num_proc, labels) in sort_by_key(num_procs):
                    for (label, data) in labels.items():
                        if label not in node:
                            node[label] = ([], [], [], [])
                        values = (
                            [item[0] for item in data],\
                            [item[1] for item in data],\
                            [item[2] for item in data]\
                        )
                        node[label][0].append(num_proc)
                        node[label][1].append(sum(values[0])/len(values[0]))
                        node[label][2].append(sum(values[1])/len(values[1]))
                        node[label][3].append(sum(values[2])/len(values[2]))

    return dataset


'''
`dataset` must be a directory with the following structure:
    {n_agent: {k: {mode: {label:
        ([num_procs, ...], [mean, ...], [min, ...], [max, ...])
    }}}}

label_pattern -- regex: used as a label filter. Data corresponding
to each label is shown only if the label matches `label_pattern`.
ylabel -- str: Label used for the Y axis
'''
def plot(
        title,
        dataset, ylabel, num_proc_scale=1.5,
        labels=[r".*"],
        modes=[r".*"],
        yscale="linear"):
    plt.suptitle(title)
    # subplot index
    index=1
    # Number of rows in the subplot environment
    # Data for each number of agent is displayed on each row
    num_rows=len(dataset)

    # Used to plot legent only on the first subplot
    plt_legend = True
    for (num_agent, num_agent_data) in dataset.items():
        # A list containing ranks for which data must be displayed.
        # Handles the case when data is not available for all "num_procs" for
        # some (n_agent, k, mode, label) parameters.
        num_procs=set([])
        for k_data in num_agent_data.values():
            for mode_data in k_data.values():
                for perf_data in mode_data.values():
                    for num_proc in perf_data[0]:
                        num_procs.add(num_proc)
        num_procs=sorted(list(num_procs))

        # Number of columns in the subplot environment
        # In the n_agent row, a subplot is created for each k
        num_columns=len(num_agent_data)

        for (k, k_data) in num_agent_data.items():
            # Initializes the subplot for the current (n_agent, k) parameters
            ax = plt.subplot(num_rows, num_columns, index)
            plt.title(str(num_agent) + " cities (K=" + str(k) + ")")
            plt.yscale(yscale)
            # increments subplot index for next subplot
            index+=1

            # Number of modes available for the current (n_agent, k) parameters
            mode_index = 0
            # Bars that will be plotted
            # Data are grouped by label, independently of the mode, with the
            # following form:
            # {"label": ([x_positions], [mean], [[min], [max]])}
            # So, if data is available for "label" for "ghost" and "hard_sync"
            # modes, `bars_data` is built as follows:
            # {"some_label": (
            #   [ghost_x_positions, ..., hard_sync_x_positions, ...],
            #   [ghost_mean_values, ..., hard_sync_mean_values, ...]
            #   [ghost_min_values, ..., hard_sync_min_values, ...]
            #   [ghost_max_values, ..., hard_sync_max_values, ...]
            #   )}
            # Data for "some_label" is then plotted in one block, so that a
            # single legend is associated to all data corresponding to label,
            # for any mode
            bars_data={}
            # Width of each bar
            bar_width = .1
            # Brief labels (not very generic)
            mode_labels={"ghost": "G", "hard_sync": "H"}

            filtered_modes=[]
            for mode in k_data.keys():
                match = False
                i = 0
                while(i < len(modes) and match==False):
                    if re.match(modes[i], mode):
                        filtered_modes.append(mode)
                        match=True
                    i+=1
            num_modes = len(filtered_modes)

            for mode in filtered_modes:
                mode_data = k_data[mode]
                # filtered data labels that will be plotted
                filtered_labels = []
                for label in mode_data.keys():
                    match = False
                    i = 0
                    while(i < len(labels) and match==False):
                        if re.match(labels[i], label):
                            filtered_labels.append(label)
                            match=True
                        i+=1
                num_labels = len(filtered_labels)

                # Margin between bars associated to different modes
                mode_margin = num_labels/2 * bar_width + .05
                mode_offset = 0
                if num_modes > 1:
                    # Center position of bars associated to the current mode
                    mode_offset = mode_index * 2 * mode_margin / (num_modes-1) - mode_margin
                    
                label_index = 0
                for label in filtered_labels:
                    perf_data = mode_data[label]
                    if label not in bars_data:
                        # Initializes the bar data entry.
                        # Data is represented as follows:
                        # ([num_procs], [mean], [[min], [max]]
                        bars_data[label] = ([], [], [[], []])

                    # Computes positions of each bars
                    x = [num_proc_scale*i + mode_offset\
                            + bar_width*(label_index - float(num_labels-1) /2)\
                            for i in range(0, len(num_procs))]
                    # Adds x values to current bars data
                    bars_data[label][0].extend(x)
                    # Adds mean values to current bars data
                    bars_data[label][1].extend(perf_data[1])

                    # according to the matplotlib bar "yerr" parameter, errors
                    # must be **relative** to data values.
                    for i in range(0, len(perf_data[2])):
                        # Adds min values to current bars data
                        bars_data[label][2][0].append(
                                abs(perf_data[1][i] - perf_data[2][i])
                                )
                        # Adds max values to current bars data
                        bars_data[label][2][1].append(
                                abs(perf_data[1][i] - perf_data[3][i])
                                )
                    label_index+=1
                mode_index+=1
                if num_modes > 1:
                    # Adds mode labels to each bar groups
                    for i in range(0, len(num_procs)):
                        ax.annotate(mode_labels[mode],
                                xy=(num_proc_scale*i + mode_offset, 0),
                                xytext=(0, -16),
                                textcoords="offset points",
                                va="bottom",
                                ha="center")


                # Show ticks for each number of processes
                ax.set_xticks([num_proc_scale*i\
                        for i in range(0, len(num_procs))])
                ax.set_xticklabels(num_procs)
                
            # Finally, plots bars for each label
            for (label, data) in bars_data.items():
                ax.bar(
                        data[0], data[1], yerr=data[2],
                        align="center", width=bar_width, label=label
                        )

            if plt_legend:
                ax.legend()
                plt_legend = False
            plt.xlabel("Number of cores")
            plt.ylabel(ylabel)

def plot_times(dataset):
    """ Plots time data from the input dataset """
    plot(
            "Time probe results of the distributed SIR model simulation",
            dataset, "Times (seconds)", labels=[r".*time.*"])

def plot_counts(dataset):
    """ Plots call counts data from the input dataset """
    plot(
            "Call counts for the distributed SIR model simulation",
            dataset, "Call counts", num_proc_scale=0.7,
            labels=[r".*count.*"],
            modes=[r"ghost"],
            yscale="log")
    

'''
Builds an argument parser mainly used to filter data outputs
'''
def build_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument(
            'results_dir', metavar='D', type=str, default=".", nargs='?',\
                    help="Output directory of the fpmas-sir-macropop simulation\
                    (default: current directory)")
    parser.add_argument(
            '-k', '--k_values', type=int, nargs="*",\
                    help="K values to plot (default: plots all values)")
    parser.add_argument(
            '-n', '--n_cities', type=int, nargs="*",\
                    help="Plots graphs for the specified city counts (default: plots all data)")
    return parser


"""
The structure of the input directory, specified as the first parameter of the
script, must be as follows:
    - <root>
      - <n_agent>
        - <k>
          - ghost
            - <num_proc>
              - <job_id>
                - ... (other outputs)
                - perf.%r.csv
          - hard_sync
            - <num_proc>
              - <job_id>
                - ... (other outputs)
                - perf.%r.csv
with '%r' in [0, num_proc[.

The purpose of this script is to compute statistics about values contained
in perf.%r.csv files, for each <num_proc>, and to display it using matplotlib.
"""
if __name__ == "__main__":
    parser = build_parser()
    args = parser.parse_args()

    root_dir = args.results_dir 

    print("Reading data from \"" + root_dir + "\"")
    # Reads row data from input directory
    perf_data = read_data(root_dir, args.k_values, args.n_cities)
    # Builds performance dataset
    dataset = build_datasets(perf_data)
    # Plots time data
    plot_times(dataset)
    plt.figure() # Build a new matplotlib figure
    # Plots call counts data
    plot_counts(dataset)
    # Show the two figures
    plt.show()
