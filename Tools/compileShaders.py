import argparse
import os

#TODO: Add support for include directory

# Setup arguments
parser = argparse.ArgumentParser()
parser.add_argument("source", help="Directory of the shader files.", type=str)
parser.add_argument("output", help="Directory where to output the Spir-V files.", type=str)
parser.add_argument("-v", "--verbose", help="Display more log messages for debug reasons.", action="store_true")
parser.add_argument("-O", "--optimize", help="Optimize Spir-V for better performance.", action="store_true")
args = parser.parse_args()


def log(str, force_log = False):
	global args
	if (args.verbose or force_log):
		print("[ShaderCompilation]: " + str)

def compile_shader(shader_file_name):
	global args

	shader_dir = args.source + shader_file_name
	output_shader_dir = args.output + shader_file_name + ".spv";
	compile_command = "glslc " + shader_dir + " -o " + output_shader_dir

	if (args.optimize):
		compile_command = compile_command + " -O"
	
	log("Compiling " + shader_dir)
	os.system(compile_command)


# Compile Shaders
log("Compiling shaders from path " + args.source + " to destination " + args.output)

if (args.optimize):
	log("Optimize enabled.")

if (not os.path.exists(args.output)):
	log("Creating " + args.output)
	os.system("mkdir " + args.output)

for file_name in os.listdir(args.source):
	extension = os.path.splitext(file_name)[-1];
	if (extension == ".vert") or (extension == ".frag"):
		compile_shader(file_name);
				
log("Compilation Complete", True)
