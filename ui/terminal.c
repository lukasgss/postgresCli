#include <ctype.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>

#include "ui/terminal.h"

struct termios original_tty;

void terminal_disable_echo(void)
{
    struct termios tty;
    tcgetattr(STDIN_FILENO, &original_tty);
    tty = original_tty;

    tty.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

void terminal_enable_echo(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tty);
}

char *highlight_by_color(const char *str, enum HIGHLIGHT_COLOR color)
{
    if (str == NULL)
    {
        return NULL;
    }

    size_t str_len = strlen(str);
    size_t ansi_len = strlen(C_GREEN) + strlen(C_RESET);
    char *formatted_str = malloc(str_len + ansi_len + 1);

    if (formatted_str == NULL)
    {
        fprintf(stdout, "error allocating memory to highlight by color\n");
        return NULL;
    }

    switch (color)
    {
        case HIGHLIGHT_GREEN:
            sprintf(formatted_str, "%s%s%s", C_GREEN, str, C_RESET);
            break;
        default:
            strcpy(formatted_str, str);
    }

    return formatted_str;
}

char *highlight(const char *input)
{
    // TODO: use dynamic sized buffer
    static char buffer[4096];
    char *write_pos = buffer;
    const char *p = input;

    while (*p != '\0')
    {
        if (isspace(*p))
        {
            *write_pos++ = *p++;
            continue;
        }

        if (*p == '\'' || *p == '"')
        {
            char quote = *p;
            write_pos += sprintf(write_pos, "%s%c", C_GREEN, *p++);

            while (*p && *p != quote)
            {
                *write_pos++ = *p++;
            }
            if (*p)
            {
                *write_pos++ = *p++;
            }

            write_pos += sprintf(write_pos, "%s", C_RESET);
            continue;
        }

        if (isalpha(*p))
        {
            size_t word_max_size = 32;
            char *word = malloc(word_max_size);
            if (word == NULL)
            {
                fprintf(stderr, "failed to allocate memory for word");
                exit(1);
            }

            bool contains_left_paren = false;
            bool contains_right_paren = false;
            size_t len = 0;

            while (*p && (isalnum(*p) || *p == '(' || *p == ')'))
            {
                if (len >= word_max_size - 1)
                {
                    word_max_size *= 2;
                    char *new_word = realloc(word, word_max_size);
                    if (new_word == NULL)
                    {
                        free(word);
                        fprintf(stderr, "failed to allocate memory for word");
                        return NULL;
                    }

                    word = new_word;
                }

                if (*p == '(')
                {
                    contains_left_paren = true;
                }
                else if (*p == ')')
                {
                    contains_right_paren = true;
                }
                word[len++] = *p++;
            }
            word[len] = '\0';

            if (contains_left_paren && contains_right_paren)
            {
                write_pos +=
                    sprintf(write_pos, "%s%s%s", C_FUNCTION, word, C_RESET);
            }
            else
            {
                write_pos += sprintf(write_pos, "%s", word);
            }

            free(word);

            continue;
        }

        if (isdigit(*p))
        {
            write_pos += sprintf(write_pos, "%s", C_NUMBER);

            while (*p && (isdigit(*p)))
            {
                *write_pos++ = *p++;
            }

            write_pos += sprintf(write_pos, "%s", C_RESET);
            continue;
        }

        *write_pos++ = *p++;
    }

    *write_pos = '\0';
    return buffer;
}

void custom_display(void)
{
    char *highlighted = highlight(rl_line_buffer);
    // clear line and redraw
    printf("\r\033[K%s%s", rl_prompt, highlighted);

    int cursor_pos = rl_point;
    int line_len = strlen(rl_line_buffer);
    if (cursor_pos < line_len)
    {
        printf("\033[%dD", line_len - cursor_pos);
    }

    fflush(stdout);
}

void init_readline(void) { rl_redisplay_function = custom_display; }
