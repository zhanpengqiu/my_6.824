# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/qzp/lab/study/src/raft

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/qzp/lab/study/src/raft/build

# Include any dependencies generated for this target.
include CMakeFiles/client_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/client_test.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/client_test.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/client_test.dir/flags.make

CMakeFiles/client_test.dir/client_test.cpp.o: CMakeFiles/client_test.dir/flags.make
CMakeFiles/client_test.dir/client_test.cpp.o: ../client_test.cpp
CMakeFiles/client_test.dir/client_test.cpp.o: CMakeFiles/client_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/qzp/lab/study/src/raft/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/client_test.dir/client_test.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/client_test.dir/client_test.cpp.o -MF CMakeFiles/client_test.dir/client_test.cpp.o.d -o CMakeFiles/client_test.dir/client_test.cpp.o -c /home/qzp/lab/study/src/raft/client_test.cpp

CMakeFiles/client_test.dir/client_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/client_test.dir/client_test.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/qzp/lab/study/src/raft/client_test.cpp > CMakeFiles/client_test.dir/client_test.cpp.i

CMakeFiles/client_test.dir/client_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/client_test.dir/client_test.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/qzp/lab/study/src/raft/client_test.cpp -o CMakeFiles/client_test.dir/client_test.cpp.s

# Object files for target client_test
client_test_OBJECTS = \
"CMakeFiles/client_test.dir/client_test.cpp.o"

# External object files for target client_test
client_test_EXTERNAL_OBJECTS =

client_test: CMakeFiles/client_test.dir/client_test.cpp.o
client_test: CMakeFiles/client_test.dir/build.make
client_test: libraftmodule.a
client_test: /usr/local/lib/libgrpc++_reflection.a
client_test: /usr/local/lib/libgrpc++.a
client_test: /usr/local/lib/libgrpc.a
client_test: /usr/local/lib/libupb_json_lib.a
client_test: /usr/local/lib/libupb_textformat_lib.a
client_test: /usr/local/lib/libutf8_range_lib.a
client_test: /usr/local/lib/libupb_message_lib.a
client_test: /usr/local/lib/libupb_base_lib.a
client_test: /usr/local/lib/libupb_mem_lib.a
client_test: /usr/local/lib/libre2.a
client_test: /usr/local/lib/libz.a
client_test: /usr/local/lib/libcares.a
client_test: /usr/local/lib/libgpr.a
client_test: /usr/local/lib/libabsl_random_distributions.a
client_test: /usr/local/lib/libabsl_random_seed_sequences.a
client_test: /usr/local/lib/libabsl_random_internal_pool_urbg.a
client_test: /usr/local/lib/libabsl_random_internal_randen.a
client_test: /usr/local/lib/libabsl_random_internal_randen_hwaes.a
client_test: /usr/local/lib/libabsl_random_internal_randen_hwaes_impl.a
client_test: /usr/local/lib/libabsl_random_internal_randen_slow.a
client_test: /usr/local/lib/libabsl_random_internal_platform.a
client_test: /usr/local/lib/libabsl_random_internal_seed_material.a
client_test: /usr/local/lib/libabsl_random_seed_gen_exception.a
client_test: /usr/local/lib/libssl.a
client_test: /usr/local/lib/libcrypto.a
client_test: /usr/local/lib/libaddress_sorting.a
client_test: /usr/local/lib/libprotobuf.a
client_test: /usr/local/lib/libabsl_log_internal_check_op.a
client_test: /usr/local/lib/libabsl_leak_check.a
client_test: /usr/local/lib/libabsl_die_if_null.a
client_test: /usr/local/lib/libabsl_log_internal_conditions.a
client_test: /usr/local/lib/libabsl_log_internal_message.a
client_test: /usr/local/lib/libabsl_log_internal_nullguard.a
client_test: /usr/local/lib/libabsl_examine_stack.a
client_test: /usr/local/lib/libabsl_log_internal_format.a
client_test: /usr/local/lib/libabsl_log_internal_proto.a
client_test: /usr/local/lib/libabsl_log_internal_log_sink_set.a
client_test: /usr/local/lib/libabsl_log_sink.a
client_test: /usr/local/lib/libabsl_log_entry.a
client_test: /usr/local/lib/libabsl_flags_internal.a
client_test: /usr/local/lib/libabsl_flags_marshalling.a
client_test: /usr/local/lib/libabsl_flags_reflection.a
client_test: /usr/local/lib/libabsl_flags_config.a
client_test: /usr/local/lib/libabsl_flags_program_name.a
client_test: /usr/local/lib/libabsl_flags_private_handle_accessor.a
client_test: /usr/local/lib/libabsl_flags_commandlineflag.a
client_test: /usr/local/lib/libabsl_flags_commandlineflag_internal.a
client_test: /usr/local/lib/libabsl_log_initialize.a
client_test: /usr/local/lib/libabsl_log_globals.a
client_test: /usr/local/lib/libabsl_vlog_config_internal.a
client_test: /usr/local/lib/libabsl_log_internal_fnmatch.a
client_test: /usr/local/lib/libabsl_log_internal_globals.a
client_test: /usr/local/lib/libabsl_raw_hash_set.a
client_test: /usr/local/lib/libabsl_hash.a
client_test: /usr/local/lib/libabsl_city.a
client_test: /usr/local/lib/libabsl_low_level_hash.a
client_test: /usr/local/lib/libabsl_hashtablez_sampler.a
client_test: /usr/local/lib/libabsl_statusor.a
client_test: /usr/local/lib/libabsl_status.a
client_test: /usr/local/lib/libabsl_cord.a
client_test: /usr/local/lib/libabsl_cordz_info.a
client_test: /usr/local/lib/libabsl_cord_internal.a
client_test: /usr/local/lib/libabsl_cordz_functions.a
client_test: /usr/local/lib/libabsl_exponential_biased.a
client_test: /usr/local/lib/libabsl_cordz_handle.a
client_test: /usr/local/lib/libabsl_crc_cord_state.a
client_test: /usr/local/lib/libabsl_crc32c.a
client_test: /usr/local/lib/libabsl_crc_internal.a
client_test: /usr/local/lib/libabsl_crc_cpu_detect.a
client_test: /usr/local/lib/libabsl_bad_optional_access.a
client_test: /usr/local/lib/libabsl_strerror.a
client_test: /usr/local/lib/libabsl_str_format_internal.a
client_test: /usr/local/lib/libabsl_synchronization.a
client_test: /usr/local/lib/libabsl_stacktrace.a
client_test: /usr/local/lib/libabsl_symbolize.a
client_test: /usr/local/lib/libabsl_debugging_internal.a
client_test: /usr/local/lib/libabsl_demangle_internal.a
client_test: /usr/local/lib/libabsl_graphcycles_internal.a
client_test: /usr/local/lib/libabsl_kernel_timeout_internal.a
client_test: /usr/local/lib/libabsl_malloc_internal.a
client_test: /usr/local/lib/libabsl_time.a
client_test: /usr/local/lib/libabsl_civil_time.a
client_test: /usr/local/lib/libabsl_time_zone.a
client_test: /usr/local/lib/libabsl_bad_variant_access.a
client_test: /usr/local/lib/libutf8_validity.a
client_test: /usr/local/lib/libabsl_strings.a
client_test: /usr/local/lib/libabsl_int128.a
client_test: /usr/local/lib/libabsl_strings_internal.a
client_test: /usr/local/lib/libabsl_string_view.a
client_test: /usr/local/lib/libabsl_base.a
client_test: /usr/local/lib/libabsl_spinlock_wait.a
client_test: /usr/local/lib/libabsl_throw_delegate.a
client_test: /usr/local/lib/libabsl_raw_logging_internal.a
client_test: /usr/local/lib/libabsl_log_severity.a
client_test: CMakeFiles/client_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/qzp/lab/study/src/raft/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable client_test"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/client_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/client_test.dir/build: client_test
.PHONY : CMakeFiles/client_test.dir/build

CMakeFiles/client_test.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/client_test.dir/cmake_clean.cmake
.PHONY : CMakeFiles/client_test.dir/clean

CMakeFiles/client_test.dir/depend:
	cd /home/qzp/lab/study/src/raft/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/qzp/lab/study/src/raft /home/qzp/lab/study/src/raft /home/qzp/lab/study/src/raft/build /home/qzp/lab/study/src/raft/build /home/qzp/lab/study/src/raft/build/CMakeFiles/client_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/client_test.dir/depend

