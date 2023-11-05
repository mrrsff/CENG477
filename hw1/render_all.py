import subprocess
import time
import os

scenes = [f for f in os.listdir("./inputs")]

for scene in scenes:
    print(f"Rendering {scene}")
    timeStart = time.time()
    subprocess.run([f"./raytracer.exe", f"./inputs/{scene}"])
    timeEnd = time.time()
    print(f"Done in {timeEnd - timeStart:.2f} seconds")