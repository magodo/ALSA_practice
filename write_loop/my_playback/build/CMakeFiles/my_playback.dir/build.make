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
CMAKE_SOURCE_DIR = /home/uidj4668/code/C/ALSA/write_loop/my_playback

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/uidj4668/code/C/ALSA/write_loop/my_playback/build

# Include any dependencies generated for this target.
include CMakeFiles/my_playback.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/my_playback.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/my_playback.dir/flags.make

CMakeFiles/my_playback.dir/my_playback.c.o: CMakeFiles/my_playback.dir/flags.make
CMakeFiles/my_playback.dir/my_playback.c.o: ../my_playback.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/uidj4668/code/C/ALSA/write_loop/my_playback/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/my_playback.dir/my_playback.c.o"
	/usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/my_playback.dir/my_playback.c.o   -c /home/uidj4668/code/C/ALSA/write_loop/my_playback/my_playback.c

CMakeFiles/my_playback.dir/my_playback.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/my_playback.dir/my_playback.c.i"
	/usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -E /home/uidj4668/code/C/ALSA/write_loop/my_playback/my_playback.c > CMakeFiles/my_playback.dir/my_playback.c.i

CMakeFiles/my_playback.dir/my_playback.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/my_playback.dir/my_playback.c.s"
	/usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -S /home/uidj4668/code/C/ALSA/write_loop/my_playback/my_playback.c -o CMakeFiles/my_playback.dir/my_playback.c.s

CMakeFiles/my_playback.dir/my_playback.c.o.requires:
.PHONY : CMakeFiles/my_playback.dir/my_playback.c.o.requires

CMakeFiles/my_playback.dir/my_playback.c.o.provides: CMakeFiles/my_playback.dir/my_playback.c.o.requires
	$(MAKE) -f CMakeFiles/my_playback.dir/build.make CMakeFiles/my_playback.dir/my_playback.c.o.provides.build
.PHONY : CMakeFiles/my_playback.dir/my_playback.c.o.provides

CMakeFiles/my_playback.dir/my_playback.c.o.provides.build: CMakeFiles/my_playback.dir/my_playback.c.o

# Object files for target my_playback
my_playback_OBJECTS = \
"CMakeFiles/my_playback.dir/my_playback.c.o"

# External object files for target my_playback
my_playback_EXTERNAL_OBJECTS =

my_playback: CMakeFiles/my_playback.dir/my_playback.c.o
my_playback: CMakeFiles/my_playback.dir/build.make
my_playback: CMakeFiles/my_playback.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable my_playback"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/my_playback.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/my_playback.dir/build: my_playback
.PHONY : CMakeFiles/my_playback.dir/build

CMakeFiles/my_playback.dir/requires: CMakeFiles/my_playback.dir/my_playback.c.o.requires
.PHONY : CMakeFiles/my_playback.dir/requires

CMakeFiles/my_playback.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/my_playback.dir/cmake_clean.cmake
.PHONY : CMakeFiles/my_playback.dir/clean

CMakeFiles/my_playback.dir/depend:
	cd /home/uidj4668/code/C/ALSA/write_loop/my_playback/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/uidj4668/code/C/ALSA/write_loop/my_playback /home/uidj4668/code/C/ALSA/write_loop/my_playback /home/uidj4668/code/C/ALSA/write_loop/my_playback/build /home/uidj4668/code/C/ALSA/write_loop/my_playback/build /home/uidj4668/code/C/ALSA/write_loop/my_playback/build/CMakeFiles/my_playback.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/my_playback.dir/depend

