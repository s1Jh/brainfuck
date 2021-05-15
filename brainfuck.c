/**
 * Brainfuck interpreter.
 *
 * Made by sijh.
 *
 * 17.4.2021:
 *      started the project, wrote pretty much all the code, some bugs still preventing functionality
 *
 * 25.5.2021:
 *      fixed up all the bugs, tested all commands, first sucessful execution of Hello world!
 *
 * 26.5.2021:
 *      refactored the argument parser a touch, implemented tape modes, fixed a buffer overrun issue in reads
 *
 * 08.5.2021 - 15.5.2021:
 *      added ascii image output, program now quits when it reaches a \0 character in the program tape, reorganizations
 *      5% performance imporovement in the mandelbro algorithm (average 57 seconds instead of 60) thanks to the early exit at program end
 *      refactored find_matching_brace a bit
 *
 * Compile with "gcc -Wall -O3 -o brainfuck brainfuck.c".
 *
 * idk, do whatever you want license
 * 
 * :)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define PROGRAM_SIZE 30000
#define TAPE_SIZE 30000
#define IMG_WIDTH 128
#define IMG_HEIGHT 64

#define VER_MAJOR 1 
#define VER_MINOR 1

#define COMP_ARG(ar, x, y) if (strcmp(ar, x ) == 0 || strcmp(ar, y ) == 0)

// enums
typedef enum DataMode
{
	DM_Wrap,	  // when we reach past the data buffer, wrap around
	DM_Terminate, // when we reach past the data buffer, halt the program
	DM_Clamp 	  // when we reach past the data buffer, stay at the last or first index
} e_datamode;

typedef enum SearchDirection
{
	SD_Forward = 1,
	SD_Backward = -1
} e_searchdir;

// pre decls
int main(int argc, char **argv);
int execute();
int setup_env();
void com_exit(int status);
void parse_arguments(int argc, char **argv);
void print_help();
int find_matching_brace(unsigned char* str, int start, e_searchdir dir);
int load_program(char* path);

void out_img();
void out(char v);
char in();

// globals
unsigned char *tape;
unsigned char *program;
int data_ctr;
int ins_ctr;

char* program_path;
int debug_log = 0;
e_datamode tape_mode = DM_Wrap;
int tape_size = TAPE_SIZE;
int program_size = PROGRAM_SIZE;

int image_mode = 0;
int image_w = IMG_WIDTH;
int image_h = IMG_HEIGHT;
int image_a;
char* image;

// function definitions
int find_matching_brace(unsigned char *str, int start, e_searchdir dir)
{
	int lb = 0;
	int rb = 0;
	
	//count = count + start * (int)dir;

    int end = dir == SD_Forward ? program_size : 0;

	for (int i = start; start != end; i += (int)dir)
	{
		if (str[i] == '[') lb++;
		if (str[i] == ']') rb++;
				
		if (lb == rb)
		{
			// the last matching brace is here, return the index
			return i;
		}
	}

	return -1;
}

int main(int argc, char **argv)
{
	printf("Brainfuck interpreter v%i.%i\n", VER_MAJOR, VER_MINOR);
	printf("Created by sijh, 17.04.2021, refer to brainfuck --help for usage.\n");
	
	parse_arguments(argc, argv);

	int err = setup_env();
	if (err != 0x0)
	{
		printf("Non fatal error during setup: %i.\n", err);
	}

	int bfprog_status = execute();

	printf("Program halted with code %x.\n", bfprog_status);

    out_img();

	com_exit(bfprog_status);	
}

int load_program(char* path)
{
	int status = 0x0;

	printf("Loading program \"%s\".\n", path);

	FILE* f = fopen(path, "r+");

	if (f == NULL)
	{
		printf("[CRITICAL ERROR]: Couldn't open file %s.\n", path);
		com_exit(0x0401);
	}

	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	rewind(f);

	if (size > program_size)
	{
		printf("[CRITICAL ERROR]: Can't fit program into memory, program: %iB, memory: %iB.\n", size, program_size);
		com_exit(0x0402);
	}

	int result = fread(program, sizeof(char), size, f);

	if (result != size)
	{
		printf("[WARNING]: Read the incorrect amount of bytes, read: %iB, expected: %iB.\n", result, size);
		status = 0x0403;
	}
	fclose(f);

	return status;
}

void print_help()
{
	printf(
		"Created by sijh, last modified 17.4.2021\n"
		"Usage: brainfuck [arguments] FILE\n"
		"Arguments:\n"
		"\t-h/--help\tDisplays this message.\n"
		"\t-t/--tape [int]\tSets the emulated tape size (default: %i).\n"
		"\t-p/--program [int]\tSets the emulated program storage size (default: %i).\n"
		"\t-d/--debug\t Enables program debugging.\n"
		"\t-tm/--tape-mode\t Sets how the tape behaves when programs index over the specified limit.\n"
		"\t-i/--image\t\t\tEnables image output.\n"
		"\t-iw/--image-width [int]\t\tSets the width of the output ascii image, also enables image mode (default: %i).\n"
		"\t-ih/--image-height [int]\tSets the height of the output ascii image, also enables image mode (default: %i).\n",
		TAPE_SIZE, PROGRAM_SIZE, IMG_WIDTH, IMG_HEIGHT
	);
}

void parse_arguments(int argc, char **argv)
{
	for (int i = 0; i < argc; i++)
	{
		char* arg = argv[i];

		// help switch
		COMP_ARG(arg, "-h", "--help")
		{
			print_help();
			com_exit(0x0000);
		}
		// tape size switch
		COMP_ARG(arg, "-t", "--tape")
		{
			if (i + 1 < argc)
			{
				i++;
				char *l = argv[i];

				int result = atoi(l);

				if (result == 0)
				{
					printf("Tape size parameter either malformed or 0, defaulting to %i.\n", TAPE_SIZE);
				}
				else
				{
					printf("Setting tape size to %i\n", result);
					tape_size = result;
				}
			}
			else 
			{
				printf("Tape size switch specified, but no value passed in.\n");
			}
		}
		// program size switch
		COMP_ARG(arg, "-p", "--program")
		{
			if (i + 1 < argc)
			{
				i++;
				char *l = argv[i];

				int result = atoi(l);

				if (result == 0)
				{
					printf("Program size parameter either malformed or 0, defaulting to %i.\n", TAPE_SIZE);
				}
				else
				{
					printf("Setting program size to %i\n", result);
					program_size = result;
				}
			}
			else 
			{
				printf("Program size switch specified, but no value passed in.\n");
			}
		}
		COMP_ARG(arg, "-iw", "--image-width")
		{
			image_mode = 1;
			if (i + 1 < argc)
			{
				i++;
				char *l = argv[i];

				int result = atoi(l);

				if (result == 0)
				{
					printf("Image width parameter either malformed or 0, defaulting to %i.\n", IMG_WIDTH);
				}
				else
				{
					printf("Setting image width to %i\n", result);
					image_w = result;
				}
			}
			else
			{
				printf("Image width switch specified, but no value passed in.\n");
			}
		}
		COMP_ARG(arg, "-ih", "--image-height")
		{
			image_mode = 1;
			if (i + 1 < argc)
			{
				i++;
				char *l = argv[i];

				int result = atoi(l);

				if (result == 0)
				{
					printf("Image height parameter either malformed or 0, defaulting to %i.\n", IMG_HEIGHT);
				}
				else
				{
					printf("Setting image height to %i\n", result);
					image_h = result;
				}
			}
			else
			{
				printf("Image height switch specified, but no value passed in.\n");
			}
		}
		COMP_ARG(arg, "-d", "--debug")
		{
			debug_log = 1;
		}
		COMP_ARG(arg, "-i", "--image")
		{
			image_mode = 1;
		}
		COMP_ARG(arg, "-tm", "--tape-mode")
		{
			if (i + 1 < argc)
			{
				i++;
				char *mod = argv[i];

				if (strcmp(mod, "wrap") == 0)
				{
					printf("Setting tape mode to wrap.\n");
					tape_mode = DM_Wrap;
				}
				else if (strcmp(mod, "clamp") == 0)
				{
					printf("Setting tape mode to stop at overrun.\n");
					tape_mode = DM_Clamp;
				}
				else if (strcmp(mod, "terminate") == 0)
				{
					printf("Setting tape mode to terminate on overrun.\n");
					tape_mode = DM_Terminate;
				}
			}
			else
			{
				printf("Tape mode switch specified, but no value passed in.\n");
			}
		}
		// the last switch is always the program name
		if (i == argc - 1 && i != 0)
		{
			program_path = arg;
		}
	}
	if (program_path == NULL)
	{
		printf("No program path passed in.\n");
		com_exit(0x0005);
	}
}

int setup_env()
{
	// first we do a malice check
	if (tape_size <= 0)
	{
		printf("[CRITICAL ERROR]: Tape size set to zero or less\n");
		com_exit(0x0103);
	}
	if (program_size <= 0)
	{
		printf("[CRITICAL ERROR]: Program size set to zero or less\n");
		com_exit(0x0104);
	}

	// try to allocate tape memory
	tape = (unsigned char*)calloc(tape_size, sizeof(char));
	if (tape == NULL)
	{
		printf("[CRITICAL ERROR]: Couldn't allocate tape memory\n");
		com_exit(0x0101);
	}

	// try to allocate program memory
	program = (unsigned char*)calloc(program_size, sizeof(char));
	if (program == NULL)
	{
		printf("[CRITICAL ERROR]: Couldn't allocate program memory\n");
		com_exit(0x0102);
	}

	// lastly put the pointers to location zero
	data_ctr = 0;
	ins_ctr = 0;

    // if image mode is enabled, initialize relate variables
    if (image_mode == 1)
	{
		if (image_w <= 0)
		{
			printf("[CRITICAL ERROR]: Image width set to zero or less\n");
			com_exit(0x0105);
		}
		if (image_h <= 0)
		{
			printf("[CRITICAL ERROR]: Image height set to zero or less\n");
			com_exit(0x0106);
		}

		image = (char*)calloc(image_w * image_h, sizeof(char));
		if (image == NULL)
		{
			printf("[CRITICAL ERROR]: Could not allocate space for the image\n");
			com_exit(0x0107);
		}

		image_a = 0;
	}

	int err = load_program(program_path);

	if (err != 0x0)
	{
		printf("Errors occured while reading the file: %i.\n", err);
	}

	return 0;
}

void com_exit(int status)
{
	free(tape);
	free(program);
	free(image);

	if (status != 0x0)
	{
		printf("ERROR: 0x%x\n", status);
	}

	exit(status);
}

int execute()
{
	int status = 1;

	// print a listing of the program
	printf("Beginning execution on program:\n\n"
		"========================================================\n"
		"\n%s\n"
		"========================================================\n", 
		program
	);
	int time = clock();
	int had_op;
	while (status == 1)
	{
		// FIXME: this is fucking ugly	

		had_op = 1;

		switch(program[ins_ctr])
		{
        default:
			had_op = 0;
			break;

		case '>':
			data_ctr++;
			break;
		case '<':
			data_ctr--;
			break;
		case '+':
			tape[data_ctr]++;
			break;
		case '-':
			tape[data_ctr]--;
			break;
		case '.':
			out(tape[data_ctr]);
			break;
		case ',':
			tape[data_ctr] = in();
			break;
			// since we auto increment after we are done processing
			// we will jump directly to the characters here and
			// let the code above move us to where we're actually 
			// supposed to be
		case '[':
		{
			if (tape[data_ctr] == 0)
			{
				int result = find_matching_brace(program, ins_ctr, SD_Forward);

				if (result == -1)
				{
					// fail
					printf("%i: Failed to find a closing brace\n", ins_ctr);
					status = 0x0205;
				}
				else
				{
					// success
					ins_ctr = result;
				}
			}
		}
			break;
		case ']':
			if (tape[data_ctr] != 0)
			{
				int result = find_matching_brace(program, ins_ctr, SD_Backward);

				if (result == -1)
				{
					// fail
					printf("%i: Failed to find a openning brace\n", ins_ctr);
					status = 0x0206;
				}
				else
				{
					// success
					ins_ctr = result;
				}
			}
			break;

        case 0:
            status = 0x0;
            break;
		}

		// done processing, next instruction
		ins_ctr++;

		// check our bounds
		// first the instruction bounds
		if (ins_ctr < 0)
		{
			// we have underflowed, either way halt and throw an error
			status = 0x0201;
		}
		if (ins_ctr >= program_size)
		{
			// we went past the last instruction meaning succesfull execution, return 0
			status = 0x0;
		}

		// then the data bounds
		if (data_ctr < 0)
		{
			switch (tape_mode)
			{
			case DM_Wrap:
				status %= tape_size;
				break;
			case DM_Clamp:
				data_ctr = 0;
				break;
			default:
			case DM_Terminate:
				status = 0x0203;
				break;	
			}
		}
		if (data_ctr >= tape_size)
		{
			switch (tape_mode)
			{
			case DM_Wrap:
				status %= tape_size;
				break;
			case DM_Clamp:
				data_ctr = tape_size - 1;
				break;
			default:
			case DM_Terminate:
				status = 0x0204;
				break;	
			}
		}
		if (had_op && debug_log)
		{
			printf("@%i:\tins: %c (0x%x)\tdata: 0x%x\t| ctrs: d:0x%x\ti:0x%x\n", 
				ins_ctr, (char)program[ins_ctr], (char)program[ins_ctr], tape[data_ctr], data_ctr, ins_ctr
			);
		}
	}
	
	time = clock() - time;

	printf("========================================================\nTook %i clicks, %f seconds.\n", time, ((float)time) / CLOCKS_PER_SEC);

	return status;
}

void out_img()
{
	if (image_mode == 1)
	{
		printf("Image:\n========================================================\n");
		for (int i = 0; i < image_w * image_h; i++)
		{
			printf("%c", image[i]);
		}
		printf("\n========================================================\n");
	}
}

void out(char v)
{
	if (image_mode == 1)
	{
		image_a %= image_w * image_h;
		image[image_a] = v;
		image_a++;
	}
	else
	{
		printf("-> %d\n", v);
	}
}

char in()
{
	int val = 0;
	printf("<- ");
    scanf("%i", &val);
	// make sure to do a narrowing conversion, since we expect an 8 bit value
	return (unsigned char)val;
}
