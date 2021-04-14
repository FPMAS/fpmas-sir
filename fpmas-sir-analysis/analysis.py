import matplotlib.pyplot as plt
from pathlib import Path
import sys
import operator
import argparse

'''
The output directory structure is as follow :
    - <root>
      - <n_agent>
        - <k>
          - ghost
            - <num_proc>
              - <job_id>
                - ... (outputs)
                - time.out
          - hard_sync
            - <num_proc>
              - <job_id>
                - ... (outputs)
                - time.out
The purpose of this script is to compute statistic about values contained
in time.out files, for each <num_proc>, and to display it using matplotlib.
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
                            data[num_agent][k][mode][num_proc] = []

                            job_dirs = [job_dir for job_dir in num_proc_dir.iterdir()]
                            for job_dir in job_dirs:
                                output = job_dir / "time.out"
                                with open(output, "r") as time_file:
                                    for time in time_file:
                                        data[num_agent][k][mode][num_proc].append(float(time))
    return data

def mean_data(time_data):
    mean = {num_agent : { k : { mode : { num_proc : \
            sum(data) / len(data) for (num_proc, data) in mode_data.items() if len(data) > 0}\
            for (mode, mode_data) in k_data.items()} \
            for (k, k_data) in num_agent_data.items()} \
            for (num_agent, num_agent_data) in time_data.items()}
    return mean

def min_max_data(time_data):
    mean = {num_agent : { k : { mode : { num_proc : \
            (min(data), max(data)) for (num_proc, data) in mode_data.items()\
            if len(data) > 0}\
            for (mode, mode_data) in k_data.items()} \
            for (k, k_data) in num_agent_data.items()} \
            for (num_agent, num_agent_data) in time_data.items()}
    return mean

def num_procs(dataset):
    procs=set([])
    for num_agent_data in dataset.values():
        for k_data in num_agent_data.values():
            for mode_data in k_data.values():
                for proc in mode_data.keys():
                    procs.add(proc)
    return sorted(list(procs))

def plot_data(title,
        ylabel, mean_data, min_max_data,
        yscale="linear",
        other_data=[],
        enable_titles=True):
    if enable_titles:
        plt.suptitle(title)
    index=1
    num_rows=len(mean_data)
    markers={"ghost": "o", "hard_sync": "x"}

    procs=num_procs(mean_data)
    for (num_agent, num_agent_data) in mean_data.items():
        plt.subplot(num_rows, 1, index)
        index+=1
        if enable_titles:
            plt.title(str(num_agent) + " cities")

        for (k, k_data) in num_agent_data.items():
            for (mode, mode_data) in k_data.items():
                data = dict(sorted(mode_data.items(), key=operator.itemgetter(0)))

                error_list = dict(sorted(min_max_data[num_agent][k][mode].items(), key=operator.itemgetter(0)))
                errors = [[abs(data[proc] - error_list[proc][j]) for proc in \
                    data.keys()] for j in [0, 1]]

                plt.yscale(yscale)
                plt.errorbar(data.keys(), data.values(), \
                    yerr=errors,\
                    label="K=" + str(k) + " (" + mode + ")",\
                    marker=markers[mode])

        for data in other_data:
            for (label, values) in data.items():
                plt.plot(values[0], values[1], label=label)

        plt.xlabel("Number of cores")
        plt.ylabel(ylabel)
        plt.xticks(procs)
        plt.legend()

def plot_exec_time(mean_data, min_max_data, **kwargs):
    plot_data(
            "Execution times of the distributed SIR model simulation",
            "Execution time (seconds)",
            mean_data, min_max_data, **kwargs)
    
def plot_speed_up(mean_data, min_max_data, **kwargs):
    mean_data_cpy = mean_data.copy()
    min_max_data_cpy = min_max_data.copy()
    for (num_agent, num_agent_data) in mean_data_cpy.items():
        mean_data_cpy[num_agent] = num_agent_data.copy()
        min_max_data_cpy[num_agent] = min_max_data[num_agent].copy()
        for (k, k_data) in mean_data_cpy[num_agent].items():
            num_agent_data[k] = k_data.copy()
            min_max_data_cpy[num_agent][k] = min_max_data[num_agent][k].copy()
            for (mode, mode_data) in num_agent_data[k].items():
                k_data[mode] = mode_data.copy()
                min_max_data_cpy[num_agent][k][mode] = min_max_data[num_agent][k][mode].copy()
                t_0 = k_data[mode][min(list(k_data[mode].keys()))]
                for proc in k_data[mode].keys():
                    k_data[mode][proc] = t_0 / k_data[mode][proc]
                    min_max_data_cpy[num_agent][k][mode][proc] = (
                            t_0/min_max_data_cpy[num_agent][k][mode][proc][0],
                            t_0/min_max_data_cpy[num_agent][k][mode][proc][1]
                            )

    procs = num_procs(mean_data)
    plot_data(
            "Speed up for the distributed SIR model simulation",
            "Speed up",
            mean_data_cpy, min_max_data_cpy,
            other_data=[{"linear": (procs, procs)}],
            **kwargs
            )

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
    parser.add_argument('--no-titles', action='store_true')
    return parser


if __name__ == "__main__":
    parser = build_parser()
    args = parser.parse_args()

    root_dir = args.results_dir 
    print("Reading data from \"" + root_dir + "\"")
    time_data = read_data(root_dir, args.k_values, args.n_cities)
    # print("Raw time data : " + str(time_data))
    mean_data = mean_data(time_data)
    # print("Mean times : " + str(mean_data))
    min_max_data = min_max_data(time_data)
    # print("Min max times : " + str(min_max_data))
    plot_exec_time(mean_data, min_max_data, enable_titles=not args.no_titles)
    plt.figure()
    plot_speed_up(mean_data, min_max_data, enable_titles=not args.no_titles)

    plt.show()

