import pathlib
import random
import shutil

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
                for proc in ["2", "4", "8", "16", "32"]:
                    job = str(random.randint(300000, 400000))
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

