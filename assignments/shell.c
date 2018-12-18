#include "shell.h"
#include "shell_commands.h"
#include "uart.h"
#include "pi.h"
#include "strings.h"

#define LINE_LEN 80


static int (*shell_printf)(const char * format, ...);

static const int TOTAL_COMMANDS = 5;

static const command_t commands[] = {
    {"help",   "<cmd> prints a list of commands or description of cmd", cmd_help},
    {"echo",   "<...> echos the user input to the screen", cmd_echo},
    {"reboot", "returns the user back to the bootloader", cmd_reboot},
    {"peek", "<address> displays the value stored at the address given by the first argument", cmd_peek},
    {"poke", "<address, value> stores value given by the second argument at address given by the first argument", cmd_peek}
};

int cmd_echo(int argc, const char *argv[]) 
{
    for (int i = 1; i < argc; ++i) 
        shell_printf("%s ", argv[i]);
    shell_printf("\n");
    return 0;
}



int print_all_help(){
    int i = 0;
    while(i < TOTAL_COMMANDS){
        printf("%s:  %s\n", commands[i].name, commands[i].description);
        i++;
    }
    return 0;
}

int print_cmd(char *arg){
    int i = 0;
    printf("Arg: %s\n", arg);
    while(i < TOTAL_COMMANDS){
        if (strcmp(commands[i].name, arg) == 0){
            printf("%s:  %s\n", commands[i].name, commands[i].description);
            return 1;
        }
        i++;
    }
    return 0;

}

int cmd_help(int argc, const char *argv[]) 
{
    if (argc == 1){
        return print_all_help();
    }
    if(print_cmd(argv[1])){
        return 0;
    }
    printf("Command: '%s' not found. Try Again\n", argv[1]);
    return 1;
}



int cmd_reboot(int argc, const char* argv[]){
    pi_reboot();
}



int cmd_peek(int argc, const char* argv[]){
    if(argc != 2){
        printf("Peek Expects 1 Argument\n");
        return 1;
    }
    char **endptr = NULL;
    int *address = strtonum(argv[1], endptr);
    if(!address){
        printf("'%s'  is not a valid address (don't forget to include '0x' for a hex address)\n", argv[1]);
        return 1;
    }
    printf("Value at %s: %d\n", argv[1], *address);
    return 0;
}

int cmd_poke(int argc, const char* argv[]){
    if(argc != 3){
        printf("Poke Expects 2 Argument\n");
        return 1;
    }
    char **endptr = NULL;
    unsigned int *address = strtonum(argv[1], endptr);
    if(!address){
        printf("'%s'  is not a valid address (don't forget to include '0x' for a hex address)\n", argv[1]);
        return 1;
    } 
    /*else if ((address%4) != 0){
        printf("Address must be 4-byte-aligned, try again");
        return 1;
    }*/
    int value = strtonum(argv[2], endptr);
    if(!value){
        printf("'%s'  is not a valid integer\n", argv[2]);
        return 1;
    }
    *(unsigned int*)address = (unsigned int)value;
    printf("Value: %d\n", value);
    printf("Stored at address: %d\n", *address);
    return 0; 
}


void shell_init(formatted_fn_t print_fn)
{
    shell_printf = print_fn;
}

void shell_bell(void)
{
    uart_putchar('\a');
}



static char *strndup(const char *src, int n)
{
    char *dup = malloc(n+1);
    for (int i = 0; i < n; i++){
        dup[i] = src[i];
    }
    dup[n] = 0;
    return dup;
}



static int isspace(char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\n';
}



static int tokenize(const char *line, char *array[],  int max)
{
    int ntokens = 0;

    while (*line != '\0' && ntokens < max) {
        while (isspace(*line)) line++;  // skip past spaces
        if (*line == '\0') break; // no more non-white chars
        const char *start = line;
        while (*line != '\0' && !isspace(*line)) line++; // advance to next space/null
        int nchars = line - start;
        array[ntokens++] = strndup(start, nchars);   // make heap-copy, add to array
    }
    return ntokens;
}




void shell_readline(char buf[], int bufsize)
{
    unsigned char ch = 0;
    int i = 0;
    while(ch != '\n' && i < bufsize){
        if (ch == '\b'){
            if (i == 0){
                shell_bell();
            } else {
                uart_putchar(ch);
                uart_putchar(' ');
            }
        }
        uart_putchar(ch);
        ch = keyboard_read_next();
        buf[i] = ch;
        i++;
    }
    buf[i] = 0;
    uart_putchar('\n');
}



int shell_evaluate(const char *line)
{
    char *arguments[LINE_LEN];
    int tokens = tokenize(line, arguments, LINE_LEN);
    char *command = arguments[0];

    if(strcmp(command, "echo") == 0){
        return cmd_echo(tokens, arguments);
    } else if (strcmp(command, "help") == 0){
        return cmd_help(tokens, arguments);
    } else if (strcmp(command, "reboot") == 0){
        return cmd_reboot(tokens, arguments);
    } else if (strcmp(command, "peek") == 0){
        return cmd_peek(tokens, arguments);
    } else if (strcmp(command, "poke") == 0){
        return cmd_poke(tokens, arguments);
    }   

    return 0;
}



void shell_run(void)
{
    shell_printf("Welcome to the CS107E shell. Remember to type on your PS/2 keyboard!\n");
    while (1) 
    {
        char line[LINE_LEN];

        shell_printf("Pi> ");
        shell_readline(line, sizeof(line));
        shell_evaluate(line);
    }
}