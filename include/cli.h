/*
 * This file handles everything involving processing command line options.
 */

#ifndef CLI_H
#define CLI_H

#include<common.h>
#include<string.h>
#include<archs.h>

/* struct cli_options - One static defintion of all command line options.
 * @target: The arcitecture and OS we are compiling for
 * @opt: The compilation optimization level
 * @time_compilation: If we are timing the compilation process (debug only)
 * @print_intermediates: If the intermediates get printed (debug only)
 * @run_tests: If this should run tests (debug only)
 * @input_files: An array of the names of the input files
 * @input_files_length: The length of "input_files"
 * @output_file_name: The name of the output file
 */
typedef struct cli_options {
	arch target;
	u8 opt;
	#if DEBUG
	bool time_compilation;
	bool print_intermediates;
	bool run_tests;
	#endif
	vector input_files;
	u8 input_files_length;
	char output_file_name[];
} cli_options;

/*
 * This goes through CLI options and sets the appropriate flags in
 * "global_cli_options".
 */
void process_cli_options(u32 argc, char *args[]);

extern cli_options global_cli_options;

#endif