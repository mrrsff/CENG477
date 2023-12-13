import os
import subprocess

#if windows .exe file or if linux no extension
if os.name == 'nt':
	rasterizer_executable = "./rasterizer.exe"
else:
	rasterizer_executable = "./rasterizer"

# Define the path to the "inputs_outputs" folder
inputs_outputs_folder = "./inputs_outputs"

# Create output folder if it doesn't exist
output_folder = "./my_outputs"
if not os.path.exists(output_folder):
	os.makedirs(output_folder)
# If exists clear
else:
	for file in os.listdir(output_folder):
		os.remove(os.path.join(output_folder, file))
 
# Traverse the "inputs_outputs" folder. There is multiple directories in this folder.
# If a directory is found, traverse it and find the .xml files and rasterize them.
for root, dirs, files in os.walk(inputs_outputs_folder):
	ppmCreated = False
	for name in dirs:
		# Define the path to the current directory
		current_directory = os.path.join(root, name)
		# Traverse the current directory
		for root2, dirs2, files2 in os.walk(current_directory):
			for name2 in files2:
				# Define the path to the current file
				current_file = os.path.join(root2, name2)
				# If the current file is a .xml file, rasterize it
				if current_file.endswith(".xml"):
					ppmCreated = True
					print("Rasterizing " + name2)
					subprocess.call([rasterizer_executable, current_file])
		
		# All outputs will be in the working directory
		# Move the outputs to the output folder
		if ppmCreated == False:
			continue
		newOutputFolder = os.path.join(os.getcwd(), output_folder)
		newOutputFolder = os.path.join(newOutputFolder, name)
		if not os.path.exists(newOutputFolder):
			os.makedirs(newOutputFolder)
		for file in os.listdir(os.getcwd()):
			if file.endswith(".ppm"):
				os.rename(file, os.path.join(newOutputFolder, file))
				print("Moved " + file + " to " + newOutputFolder)