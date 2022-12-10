/*
 * This file handles everything involving processing command line options.
 */

#ifndef CLI_H
#define CLI_H

#include<common.h>
#include<string.h>

/* struct cli_options - One static defintion of all command line options.
 * @compilation_target: The arcitecture and OS we are compiling for
 * @opt: The compilation optimization level
 * @time_compilation: If we are timing the compilation process (debug only)
 * @input_files: An array of the names of the input files
 * @input_files_length: The length of "input_files"
 * @output_file_name: The name of the output file
 */
struct cli_options {
	enum { linux_ARMv7 } compilation_target;
	u8 opt;
	#if DEBUG
	bool time_compilation;
	#endif
	char **input_files;
	u8 input_files_length;
	char output_file_name[];
};

void process_cli_options(u32 argc, char *args[]);

void free_cli_options();

/*
 * This function returns a pointer to the global cli options.
 */
struct cli_options *get_global_cli_options();

#endif
