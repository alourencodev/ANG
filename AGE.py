import argparse
import os
import subprocess

# Global constants
compiler = "Visual Studio 17 2022"
build_dir = "Build"
tools_dir = "Tools" # Assumes it is inside build_dir
shaders_dir = "Shaders"
game_name = "ANG"

# Setup Arguments
parser = argparse.ArgumentParser()
parser.add_argument("-b", "--build", help="Build the project.", action="store_true")
parser.add_argument("-B", "--Build", help="Build the project and runs the entire tools pipeline.", action="store_true")
parser.add_argument("-c", "--clean", help="Clean Project.", action="store_true")
parser.add_argument("-s", "--shaders", help="Build Shaders", action="store_true")
parser.add_argument("-r", "--run", help="Run Game", action="store_true")
parser.add_argument("-T", "--Tools", help="Compile Tools", action="store_true")
parser.add_argument("-t", "--tests", help="Build and run unit tests.", action="store_true")
parser.add_argument("-v", "--verbose", help="Display more log messages, for debug reasons.", action="store_true")
parser.add_argument("build_config", help="On what configuration should the project be built. debug | releaseDbgInfo | release", type=str)
args = parser.parse_args()


def log(str, force_log = False):
    global args
    if args.verbose or force_log:
        print("[" + game_name + "]: " + str)

def run_command(command):
    log("Running command: " + str(command))
    subprocess.run(command)

def clean():
    return

def build(config):

    #TODO: Track build time?

    log("Building [" + config + "]", True)

    # Generate Make File
    cmake_build_config = "-DCMAKE_BUILD_TYPE=" + config
    cmake_build_dir_arg = "-B=" + build_dir

    cmake_command = ["cmake", cmake_build_config, "-G", compiler, cmake_build_dir_arg]

    if args.verbose:
        cmake_command.append("-DCMAKE_EXPORT_COMPILE_COMMANDS=1")
        cmake_command.append("-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON")

    # Run CMake
    run_command(cmake_command)

    # Compile
    build_command = ["cmake", "--build", build_dir]
    run_command(build_command)

    return;

def compile_shaders(config):

    shader_source = game_name + "\\" + shaders_dir + "\\"
    shader_dest = build_dir + "\\" + config + "\\" + shaders_dir + "\\"

    command = ["py", "Tools\\compileShaders.py", shader_source, shader_dest]

    # Set config specific flags
    if config == "Debug":
        command.append("-v")
    else:
        command.append("-O")

    # Set the include directories
    engine_shaders = "AGE\\" + shaders_dir + "\\"
    command = command + ["-I", shader_source, engine_shaders]
    
    run_command(command)

    return

def run_tool_pipeline(config):
    compile_shaders(config)
    return;

def tests(config):
    # TODO
    return

def run(config):
    exe_dir = build_dir + "\\" + config + "\\"
    root_dir = os.getcwd()
    os.chdir(exe_dir)

    run_command([game_name + ".exe"])

    os.chdir(root_dir)

    return

def main():
    global args   

    if args.clean:
        clean()

    if args.build or args.Build:
        build(args.build_config)

    if args.Build:
        run_tool_pipeline(args.build_config)
    elif args.shaders:
        compile_shaders(args.build_config)

    if args.tests:
        tests(args.build_config)

    if args.run:
        run(args.build_config)
    
    return

if __name__ == '__main__':
    main()
