import argparse
import os
import subprocess

# Tools
compiler = "glslc"

# Settings
supported_extensions = [".vert", ".frag"]

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


def compile_shader(shader_dir, include_dirs):
    global args

    shader_file_name = os.path.basename(shader_dir)
    output_shader_dir = args.output + shader_file_name + ".spv";
    compile_command = [compiler, shader_dir, "-o", output_shader_dir]

    if args.optimize:
        compile_command.append("-O")

    if include_dirs:
        compile_command = compile_command + include_dirs

    log("Compiling " + shader_dir)

    result = subprocess.run(compile_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    error = result.stderr.decode('utf-8')
    if error:
        log("Error running " + str(compile_command))
        log(error.replace('\n', ''))
        return False
    else:
        stdOutput = result.stdout.decode('utf-8')
        if stdOutput:
            log(stdOutput.replace('\n',''))

    return True


def list_shaders(source_dir):
    shaders = []
    dirs = []

    # Split shaders from directories.
    paths = os.listdir(source_dir)
    for path in paths:
        path_dir = source_dir + path
        if os.path.isfile(path_dir):
            extension = os.path.splitext(path)[-1]
            if extension in supported_extensions:
                shaders.append(path_dir)
        else:
            dirs.append(path_dir + "\\")

    for dir in dirs:
        shaders = shaders + list_shaders(dir)
        
    return shaders


def main():
    global args

    log("Compiling shaders from path " + args.source + " to destination " + args.output)

    if args.optimize:
        log("Optimize enabled.")

    # Build list of include Directories
    include_dirs = []
    if not args.includes is None:
        log("Include Directories:")
        for include_dir in args.includes:
            log(" - " + include_dir)
            include_dirs = include_dirs + ["-I", include_dir]

    # Create output directory if necessary
    if not os.path.exists(args.output):
        log("Creating " + args.output)
        os.system("mkdir " + args.output)

    # Compile shaders
    shaders = list_shaders(args.source)

    for shader in shaders:
        result = compile_shader(shader, include_dirs);

        if not result:
            log("Compilation Failed")
            return

    log("Compilation Complete", True)


if __name__ == '__main__':
    main()
