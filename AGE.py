import argparse
import os
import subprocess

# Global constants
compiler = "Visual Studio 17 2022"
build_folder = "Build"
tools_folder = "Tools"
shaders_folder = "Shaders"
game_name = "ANG"

# Setup Arguments
parser = argparse.ArgumentParser()
parser.add_argument("-b", "--build", help="Build the project.", action="store_true")
parser.add_argument("-s", "--shaders", help="Build Shaders", action="store_true")
parser.add_argument("-r", "--run", help="Run Game", action="store_true")
parser.add_argument("-t", "--tests", help="Build and run unit tests.", action="store_true")
parser.add_argument("-v", "--verbose", help="Display more log messages, for debug reasons.", action="store_true")
parser.add_argument("build_config", help="On what configuration should the project be built. Debug | ReleaseDbgInfo | Release", type=str)
parser.add_argument("-l", "--log", nargs='*', help="Enable logs for given tags")
args = parser.parse_args()


# Utils
def log(str, force_log = False):
    global args
    if args.verbose or force_log:
        print("[" + game_name + "]: " + str)

def run_command(command):
    log("Running command: " + str(command))
    subprocess.run(command)

def get_build_dir():
    global args
    return build_folder + '\\' + args.build_config + '\\'

# Commands
def build(config):
    log("Building [" + config + "]", True)

    # Generate Make File
    cmake_build_config = "-DCMAKE_BUILD_TYPE=" + config
    cmake_build_dir_arg = "-B=" + build_folder

    cmake_command = ["cmake", cmake_build_config, "-G", compiler, cmake_build_dir_arg]

    if args.verbose:
        cmake_command.append("-DCMAKE_EXPORT_COMPILE_COMMANDS=1")
        cmake_command.append("-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON")

    cmake_command.append("-DAGE_TESTS=1")
    cmake_command.append("-DAGE_TOOLS=1")

    # Run CMake
    run_command(cmake_command)

    # Compile
    build_command = ["cmake", "--build", build_dir]
    run_command(build_command)

    return;

def compile_shaders(config):
    shader_source = game_name + "\\" + shaders_folder + "\\"
    shader_dest = get_build_dir() + shaders_folder + "\\"

    command = ["py", "Tools\\compileShaders.py", shader_source, shader_dest]

    # Set config specific flags
    if config == "Debug":
        command.append("-v")
    else:
        command.append("-O")

    # Set the include directories
    engine_shaders = "AGE\\" + shaders_folder + "\\"
    command = command + ["-I", shader_source, engine_shaders]
    
    run_command(command)

    return

def run_tests():
    log("Running Tests")

    tests_exe_dir = get_build_dir() + "Tests\\" 

    root_dir = os.getcwd()
    os.chdir(tests_exe_dir)

    # Run every executable in the Tests output dir
    files = os.listdir(".")
    for file in files:
        if os.path.splitext(file)[-1] == ".exe":
            run_command([file])

    return

def run():
    log("Running Game")

    root_dir = os.getcwd()
    os.chdir(get_build_dir())

    command = [game_name + ".exe"]
    if args.log:
        command.append("-enableLog")
        command = command + args.log

    run_command(command)

    os.chdir(root_dir)

    return

def main():
    global args   

    if args.build:
        build(args.build_config)

    if args.shaders:
        compile_shaders(args.build_config)

    if args.tests:
        run_tests()

    if args.run:
        run()
    
    return

if __name__ == '__main__':
    main()
