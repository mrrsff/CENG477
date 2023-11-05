import subprocess
import time
import sys
import os

scene = str(sys.argv[1])

print(f"Rendering {scene}")
timeStart = time.time()
subprocess.run([f"./raytracer.exe", f"./inputs/{scene}"])
timeEnd = time.time()
print(f"Done in {timeEnd - timeStart:.2f} seconds")