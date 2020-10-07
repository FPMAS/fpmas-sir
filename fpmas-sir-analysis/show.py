import matplotlib.pyplot as plt
import numpy as np
import sys
import csv
from pathlib import Path

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

def plot(output_files):
    index=1
    plt.figure()
    num_rows=len(output_files)
    for output_file in output_files:
        data = read_csv(output_file)
        plt.subplot(num_rows, 1, index)
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
        plt.ylabel("Population")
        plt.yticks(np.arange(0, max_y, step=step_y))
        index+=1
        plt.legend()
    plt.show()

if __name__ == "__main__":
    files = [sys.argv[i] for i in range(1, len(sys.argv))]
    plot(files)
