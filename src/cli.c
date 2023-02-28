/*
 * This file handles everything involving processing command line options.
 */

#include<cli.h>
#include<main.h>

/* If we're in debug mode we add the "--time" cli option. */
// TODO: Add "--func-hash-table" and "--var-hash-table"
#if DEBUG
const char help_message[] = "Usage: Sail [options] files...\n\nOptions:\n\t-h\
\t\t\t--help\t\t\tDisplays help\n\t-t\t\t\t--time\t\t\tTimes compilation proc\
ess\n\t-o <file>\t\t--output <file>\t\tAllows specification of output file\n\
\t-O<level>\t\t--opt <level>\t\tLevel of optimization 0-3\n";
#else
const char help_message[] = "Usage: Sail [options] files...\n\nOptions:\n\t-h\
\t\t\t--help\t\t\tDisplays help\n\t-o <file>\t\t--output <file>\t\tAllows speci\
fication of output file\n\t-O<level>\t\t--opt <level>\t\tLevel of optimization \
0-3\n";
#endif

cli_options global_cli_options;

/*
 * This goes through CLI options and sets the appropriate flags in
 * "global_cli_options".
 */
void process_cli_options(u32 argc, char *args[])
{
	vector input_files = vector_init(sizeof(char*), 2);
	for (i32 i=1; i < argc; i++) {
		if (!strcmp(args, "--")) {
			if (!strcmp(args[i], "--help")) {
				printf(help_message);
				exit(0);
			} else if (!strcmp(args[i], "--opt")) {
				i++;
				global_cli_options.opt = args[i][0]-48;
			}
			#if DEBUG
			if (!strcmp(args[i], "--time"))
				global_cli_options.time_compilation = true;
			#endif
		}
		else if (args[i][0] == '-') {
			switch(args[i][1])
			{
			case 'h':
				printf(help_message);
				exit(0);
				break;
			case 'O':
				// TODO: This should make sure the next argument is a number.
				global_cli_options.opt = args[i][2]-48;
				break;
			#if DEBUG
			case 't':
				global_cli_options.time_compilation = true;
				break;
			#endif
			}
		}
		else {
			char* str = malloc(strlen(args[i])+1);
			CHECK_MALLOC(str);
			strcpy(str, &args[i]);
			vector_append(&input_files, str);
		}
	}
	if (!VECTOR_SIZE(input_files))
		send_error("No input files. Use -h for help");
	global_cli_options.input_files = input_files;
}
