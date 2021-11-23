import argparse
import os

# glslc or glslangValidator
compiler = "glslc"

# Setup arguments
parser = argparse.ArgumentParser()
parser.add_argument("source", help="Directory of the shader files.", type=str)
parser.add_argument("output", help="Directory where to output the Spir-V files.", type=str)
parser.add_argument("-v", "--verbose", help="Display more log messages for debug reasons.", action="store_true")
parser.add_argument("-O", "--optimize", help="Optimize Spir-V for better performance.", action="store_true")
parser.add_argument("-I", "--includes", nargs='*', help="Directories of include files")
args = parser.parse_args()


def log(str, force_log = False):
	global args
	if args.verbose or force_log:
		print("[ShaderCompilation]: " + str)


def compile_shader(shader_file_name, include_dirs):
	global args

	shader_dir = args.source + shader_file_name
	output_shader_dir = args.output + shader_file_name + ".spv";
	compile_command = compiler + " " + shader_dir + " -o " + output_shader_dir

	if args.optimize:
		compile_command = compile_command + " -O"

	if include_dirs:
		compile_command = compile_command + include_dirs
	
	log("Compiling " + shader_dir)
	os.system(compile_command)


def main():
	global args

	log("Compiling shaders from path " + args.source + " to destination " + args.output)

	if args.optimize:
		log("Optimize enabled.")

	# Build list of include Directories
	include_dirs = ""
	if not args.includes is None:
		log("Include Directories:")
		for include_dir in args.includes:
			log(" - " + include_dir)
			include_dirs = include_dirs + " -I " + include_dir

	# Create output directory if necessary
	if not os.path.exists(args.output):
		log("Creating " + args.output)
		os.system("mkdir " + args.output)

	# Compile shaders
	for file_name in os.listdir(args.source):
		extension = os.path.splitext(file_name)[-1];
		if (extension == ".vert") or (extension == ".frag"):
			compile_shader(file_name, include_dirs);

	log("Compilation Complete", True)


if __name__ == '__main__':
	main()
