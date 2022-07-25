import argparse
import os
import subprocess

# Global constants
build_dir = "Build"
shader_compiler = "Tools\\ShaderCompiler.exe"
shader_compiler_debug = "Tools\\ShaderCompiler.exe" #TODO: Change to ShaderCompiler_d when available
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

    cmake_command = ["cmake", cmake_build_config, "-G", "MinGW Makefiles", cmake_build_dir_arg]

    if args.verbose:
        cmake_command.append("-DCMAKE_EXPORT_COMPILE_COMMANDS=1")

    if args.Tools:
        cmake_command.append("-DAGE_TOOLS=1")

    run_command(cmake_command)

    # Compile
    build_command = ["cmake", "--build", build_dir]
    run_command(build_command)
    
    # Copy dll to exe folder
    age_dll_source_dir = build_dir + "\\AGE\\libAGE.dll"
    age_dll_dest_dir = build_dir + "\\" + game_name + "\\libAGE.dll"
    copy_command = ["cp", age_dll_source_dir, age_dll_dest_dir]
    run_command(copy_command)

    return;

def compile_shaders(config):
    compiler = shader_compiler_debug
    if config == "Release":
        compiler = shader_compiler
    
    shader_source = game_name + "\\" + shaders_dir
    shader_dest = build_dir + "\\" + game_name + "\\" + shaders_dir
    engine_shaders = "AGE\\" + shaders_dir

    compile_shaders_command = [compiler, shader_source, shader_dest, shader_source, engine_shaders]   
    run_command(compile_shaders_command)

    return

def run_tool_pipeline(config):
    compile_shaders(config)
    return;

def tests(config):
    return

def run():
    exe_dir = build_dir + "\\" + game_name + "\\" + game_name + ".exe"
    run_command([exe_dir])

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
        run()
    
    return

if __name__ == '__main__':
    main()
