import matplotlib.pyplot as plt
import numpy as np
import sys
import csv
from pathlib import Path
import argparse

def read_csv(output_file):
    data = [[], [], [], [], []]
    with open(output_file) as csv_file:
        csv_reader = csv.DictReader(csv_file)
        for row in csv_reader:
            data[0].append(int(row["T"]))
            data[1].append(float(row["S"]))
            data[2].append(float(row["I"]))
            data[3].append(float(row["R"]))
            data[4].append(float(row["N"]))
    return data

def plot(output_files, rows=-1, columns=1):
    index=1
    plt.figure()
    if rows<0 and columns<0:
        rows=1
        columns=int(len(output_files))
    elif rows<0:
        rows = int(len(output_files)/columns)
    elif columns<0:
        columns=int(len(output_files)/rows)

    for output_file in output_files:
        data = read_csv(output_file)
        plt.subplot(rows, columns, index)
        plt.title("SIR model simulation results (" +\
                Path(output_file).stem + ")")
        plt.plot(data[0], data[1], label="Susceptible")
        plt.plot(data[0], data[2], label="Infected")
        plt.plot(data[0], data[3], label="Removed")
        plt.plot(data[0], data[4], label="Total Population")
        plt.xlabel("Time Step")
        plt.xticks(np.arange(data[0][0], data[0][-1], step=100))
        max_y = max(data[4])
        step_y = max_y / 10
        plt.ylabel("Global Population")
        plt.yticks(np.arange(0, max_y, step=step_y))
        index+=1
        plt.legend()
    plt.show()

def build_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument(
            'output_files', metavar='F', type=str, nargs='+',\
                    help="Output files of the fpmas-sir-macropop simulation")
    parser.add_argument(
            '-r', '--n_rows', type=int, default=-1,\
                    help="Number of rows in the subplot environment (default:1)")
    parser.add_argument(
            '-c', '--n_columns', type=int, default=-1,\
                    help="Number of columns in the subplot environment\
                    (default:number of output_files)")
    return parser

if __name__ == "__main__":
    parser = build_parser()
    args = parser.parse_args()
    plot(args.output_files,rows=args.n_rows,columns=args.n_columns)
