# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/uidj4668/code/C/ALSA/demo/pcm

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/uidj4668/code/C/ALSA/demo/pcm/build

# Include any dependencies generated for this target.
include CMakeFiles/pcm.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/pcm.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/pcm.dir/flags.make

CMakeFiles/pcm.dir/pcm.c.o: CMakeFiles/pcm.dir/flags.make
CMakeFiles/pcm.dir/pcm.c.o: ../pcm.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/uidj4668/code/C/ALSA/demo/pcm/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/pcm.dir/pcm.c.o"
	/usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/pcm.dir/pcm.c.o   -c /home/uidj4668/code/C/ALSA/demo/pcm/pcm.c

CMakeFiles/pcm.dir/pcm.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pcm.dir/pcm.c.i"
	/usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -E /home/uidj4668/code/C/ALSA/demo/pcm/pcm.c > CMakeFiles/pcm.dir/pcm.c.i

CMakeFiles/pcm.dir/pcm.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pcm.dir/pcm.c.s"
	/usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -S /home/uidj4668/code/C/ALSA/demo/pcm/pcm.c -o CMakeFiles/pcm.dir/pcm.c.s

CMakeFiles/pcm.dir/pcm.c.o.requires:
.PHONY : CMakeFiles/pcm.dir/pcm.c.o.requires

CMakeFiles/pcm.dir/pcm.c.o.provides: CMakeFiles/pcm.dir/pcm.c.o.requires
	$(MAKE) -f CMakeFiles/pcm.dir/build.make CMakeFiles/pcm.dir/pcm.c.o.provides.build
.PHONY : CMakeFiles/pcm.dir/pcm.c.o.provides

CMakeFiles/pcm.dir/pcm.c.o.provides.build: CMakeFiles/pcm.dir/pcm.c.o

# Object files for target pcm
pcm_OBJECTS = \
"CMakeFiles/pcm.dir/pcm.c.o"

# External object files for target pcm
pcm_EXTERNAL_OBJECTS =

pcm: CMakeFiles/pcm.dir/pcm.c.o
pcm: CMakeFiles/pcm.dir/build.make
pcm: CMakeFiles/pcm.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable pcm"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/pcm.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/pcm.dir/build: pcm
.PHONY : CMakeFiles/pcm.dir/build

CMakeFiles/pcm.dir/requires: CMakeFiles/pcm.dir/pcm.c.o.requires
.PHONY : CMakeFiles/pcm.dir/requires

CMakeFiles/pcm.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/pcm.dir/cmake_clean.cmake
.PHONY : CMakeFiles/pcm.dir/clean

CMakeFiles/pcm.dir/depend:
	cd /home/uidj4668/code/C/ALSA/demo/pcm/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/uidj4668/code/C/ALSA/demo/pcm /home/uidj4668/code/C/ALSA/demo/pcm /home/uidj4668/code/C/ALSA/demo/pcm/build /home/uidj4668/code/C/ALSA/demo/pcm/build /home/uidj4668/code/C/ALSA/demo/pcm/build/CMakeFiles/pcm.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/pcm.dir/depend

