#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "flags.h"
#include "ui/terminal.h"

static struct option long_options[] = {
    {"database", required_argument, 0, 'd'},
    {"host", required_argument, 0, 'H'},
    {"user", required_argument, 0, 'u'},
    {"help", no_argument, 0, 'h'},
};

void print_help(void) { printf("print help"); }

void print_usage(void)
{
    printf("Usage: pgcli -H [HOST] -u [USER] -d [DATABASE]\n");
}

char *prompt_password(char *user)
{
    printf("Password for %s: ", user);
    fflush(stdout);

    terminal_disable_echo();

    char *password = NULL;
    size_t len = 0;
    getline(&password, &len, stdin);

    terminal_enable_echo();

    printf("\n");

    // remove trailing new line from password
    if (password)
    {
        password[strcspn(password, "\n")] = '\0';
    }

    return password;
}

struct conn_info parse_flags(int argc, char **argv)
{
    if (argc <= 1)
    {
        print_usage();
        exit(EXIT_FAILURE);
    }

    char *db_name;
    char *user;
    char *host;

    int opt, option_index;

    while ((opt = getopt_long(
                argc, argv, "H:hu:d:", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
            case 'h':
                print_usage();
                exit(EXIT_SUCCESS);
                break;
            case 'd':
                db_name = optarg;
                break;
            case 'u':
                user = optarg;
                break;
            case 'H':
                host = optarg;
                break;
            default:
                fprintf(stderr, "Unknown option '%c'. Use --help.\n", optopt);
                exit(EXIT_FAILURE);
        }
    }

    char *password = prompt_password(user);

    return (struct conn_info){
        .dbname = db_name, .user = user, .host = host, .password = password};
}
