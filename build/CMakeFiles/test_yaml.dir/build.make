# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Produce verbose output by default.
VERBOSE = 1

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/xxx/main

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/xxx/main/build

# Include any dependencies generated for this target.
include CMakeFiles/test_yaml.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/test_yaml.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test_yaml.dir/flags.make

CMakeFiles/test_yaml.dir/tests/test_yaml.cc.o: CMakeFiles/test_yaml.dir/flags.make
CMakeFiles/test_yaml.dir/tests/test_yaml.cc.o: ../tests/test_yaml.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xxx/main/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/test_yaml.dir/tests/test_yaml.cc.o"
	/usr/bin/c++  $(CXX_DEFINES) -D__FILE__=\"tests/test_yaml.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test_yaml.dir/tests/test_yaml.cc.o -c /home/xxx/main/tests/test_yaml.cc

CMakeFiles/test_yaml.dir/tests/test_yaml.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_yaml.dir/tests/test_yaml.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"tests/test_yaml.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xxx/main/tests/test_yaml.cc > CMakeFiles/test_yaml.dir/tests/test_yaml.cc.i

CMakeFiles/test_yaml.dir/tests/test_yaml.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_yaml.dir/tests/test_yaml.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"tests/test_yaml.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xxx/main/tests/test_yaml.cc -o CMakeFiles/test_yaml.dir/tests/test_yaml.cc.s

# Object files for target test_yaml
test_yaml_OBJECTS = \
"CMakeFiles/test_yaml.dir/tests/test_yaml.cc.o"

# External object files for target test_yaml
test_yaml_EXTERNAL_OBJECTS =

../bin/test_yaml: CMakeFiles/test_yaml.dir/tests/test_yaml.cc.o
../bin/test_yaml: CMakeFiles/test_yaml.dir/build.make
../bin/test_yaml: ../lib/libtadpole.so
../bin/test_yaml: CMakeFiles/test_yaml.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/xxx/main/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/test_yaml"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_yaml.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test_yaml.dir/build: ../bin/test_yaml

.PHONY : CMakeFiles/test_yaml.dir/build

CMakeFiles/test_yaml.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test_yaml.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test_yaml.dir/clean

CMakeFiles/test_yaml.dir/depend:
	cd /home/xxx/main/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/xxx/main /home/xxx/main /home/xxx/main/build /home/xxx/main/build /home/xxx/main/build/CMakeFiles/test_yaml.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/test_yaml.dir/depend

