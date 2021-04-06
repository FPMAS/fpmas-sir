import pathlib
import random
import shutil
import csv

'''
A simple script to generate a random "test_data" directory that can be used as
an input to the analysis.py script.
'''

if __name__ == "__main__":
    root = pathlib.Path("test_data")
    if root.exists():
        shutil.rmtree(root)
    for num_agent in ["10000", "100000"]:
        for k in ["3", "4", "6"]:
            for mode in ["ghost", "hard_sync"]:
                for proc in ["1", "2", "4", "8", "16", "32"]:
                    jobs = [str(random.randint(300000, 400000)) for i in range(0, 5)]
                    for job in jobs:
                        path =  root / num_agent / k / mode / proc / job
                        path.mkdir(parents=True)
                        out_path = path / "time.out"
                        with open(out_path, "w") as out_file:
                            for i in range(10):
                                random_value = 1 / (int(proc) / (1000*32) + 1 / (1000 * int(k)))
                                noise = random.uniform(\
                                        -0.1 * random_value,\
                                        0.1 * random_value)
                                if mode == "ghost":
                                    random_value-=500
                                random_value += noise
                                print(str(random_value), file=out_file)
                        for n in range(0, int(proc)):
                            perf_file= path / ("perf." + str(n) + ".csv")
                            with open(perf_file, 'w', newline='') as csv_file:
                                writer = csv.writer(csv_file)
                                writer.writerow([
                                        "behavior_time", "comm_time", "distant_comm_time",
                                        "sync_time",
                                        "comm_count", "distant_comm_count"])
                                behavior_time = random.randint(1000, 2000)
                                comm_time = behavior_time - random.randint(0, 100)
                                distant_comm_time = comm_time - random.randint(0, 100)
                                sync_time = random.randint(50, 500)
                                comm_count = random.randint(50, 100)
                                distant_comm_count = comm_count - random.randint(0, 40)
                                writer.writerow([
                                        behavior_time, comm_time, distant_comm_time,
                                        sync_time,
                                        comm_count, distant_comm_count])

