#include <ctype.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/cdefs.h>
#include <termios.h>
#include <unistd.h>

#include "sql/keywords.h"
#include "sql/postgres.h"
#include "ui/terminal.h"

#define PROMPT_TEMPLATE_STR "%s@%s > "

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

static __always_inline size_t
get_ansi_length_by_color(enum HIGHLIGHT_COLOR color)
{
    size_t color_size;

    switch (color)
    {
        case HIGHLIGHT_GREEN:
            color_size = strlen(C_GREEN);
            break;
        case HIGHLIGHT_DIM_GRAY:
            color_size = strlen(C_DIM_GRAY);
            break;
        default:
            // biggest one
            color_size = strlen(C_GREEN);
    }

    return color_size + strlen(C_RESET);
}

char *highlight_by_color(const char *str, enum HIGHLIGHT_COLOR color)
{
    if (str == NULL)
    {
        return NULL;
    }

    size_t str_len = strlen(str);
    size_t ansi_len = get_ansi_length_by_color(color);
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
        case HIGHLIGHT_DIM_GRAY:
            sprintf(formatted_str, "%s%s%s", C_DIM_GRAY, str, C_RESET);
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

static char *get_word_before_current(const char *line, int cursor_pos)
{
    if (line == NULL || cursor_pos <= 0)
    {
        return NULL;
    }

    // skip back over the current incomplete word
    int pos = cursor_pos - 1;
    while (pos >= 0 && !isspace(line[pos]))
    {
        pos--;
    }

    // skip whitespace
    while (pos >= 0 && isspace(line[pos]))
    {
        pos--;
    }

    if (pos < 0)
    {
        return NULL;
    }

    int end = pos;

    // find the start of this word
    while (pos >= 0 && !isspace(line[pos]))
    {
        pos--;
    }

    int start = pos + 1;
    int word_len = end - start + 1;

    if (word_len <= 0)
    {
        return NULL;
    }

    char *word = malloc(word_len + 1);
    if (word == NULL)
    {
        fprintf(stderr, "error allocating memory for word\n");
        return NULL;
    }

    strncpy(word, line + start, word_len);
    word[word_len] = '\0';

    return word;
}

static inline bool is_after_from()
{
    char *last_word = get_word_before_current(rl_line_buffer, rl_point);

    if (last_word == NULL)
    {
        return false;
    }

    bool result = strcasecmp(last_word, "from") == 0;
    free(last_word);

    return result;
}

static char *get_suggestion(const char *text)
{
    if (text == NULL || *text == '\0')
    {
        return NULL;
    }

    int len = strlen(text);

    // suggest table names
    if (is_after_from())
    {
        const struct database_metadata *db_metadata = get_db_metadata();

        for (size_t i = 0; i < db_metadata->amount_tables; i++)
        {
            if (strncasecmp(db_metadata->tables[i], text, len) == 0)
            {
                return db_metadata->tables[i] + len;
            }
        }
    }
    // suggest keywords
    else
    {
        for (int i = 0; vocabulary[i] != NULL; i++)
        {
            if (strncasecmp(vocabulary[i], text, len) == 0)
            {
                if (islower(*text))
                {
                    return lower_vocabulary[i] + len;
                }

                return vocabulary[i] + len;
            }
        }
    }

    return NULL;
}

static int accept_suggestion(int count, int key)
{
    (void)count;
    (void)key;

    char *line = rl_line_buffer;
    int cursor = rl_point;

    int word_start = cursor;
    while (word_start > 0 && !isspace(line[word_start - 1]))
    {
        word_start--;
    }

    int word_len = cursor - word_start;
    char current_word[word_len + 1];
    strncpy(current_word, line + word_start, word_len);
    current_word[word_len] = '\0';

    char *suggestion = get_suggestion(current_word);

    if (suggestion != NULL && cursor == (int)strlen(line))
    {
        rl_insert_text(suggestion);
    }

    return 0;
}

void custom_display(void)
{
    char *highlighted = highlight(rl_line_buffer);

    char *line = rl_line_buffer;
    int cursor = rl_point;

    int word_start = cursor;
    while (word_start > 0 && !isspace(line[word_start - 1]))
    {
        word_start--;
    }

    int word_len = cursor - word_start;
    char current_word[word_len + 1];
    strncpy(current_word, line + word_start, word_len);
    current_word[word_len] = '\0';

    char *suggestion = get_suggestion(current_word);

    printf("\r\033[K%s%s", rl_prompt, highlighted);

    if (suggestion != NULL && cursor == (int)strlen(line))
    {
        printf("%s%s%s", C_DIM_GRAY, suggestion, C_RESET);
    }

    int display_cursor = cursor;
    if (suggestion != NULL && cursor == (int)strlen(line))
    {
        int suggestion_len = strlen(suggestion);
        printf("\033[%dD", suggestion_len);
    }

    int line_len = strlen(rl_line_buffer);
    if (display_cursor < line_len)
    {
        printf("\033[%dD", line_len - display_cursor);
    }

    fflush(stdout);
}

char *command_generator(const char *text, int state)
{
    static int list_index, len;
    static bool checking_tables;
    char *name;

    if (!state)
    {
        list_index = 0;
        len = strlen(text);
        checking_tables = is_after_from();
    }

    if (checking_tables)
    {
        const struct database_metadata *db_metadata = get_db_metadata();

        while ((name = db_metadata->tables[list_index++]))
        {
            if (strncasecmp(name, text, len) == 0)
            {
                return strdup(name);
            }
        }
    }
    else
    {
        while ((name = vocabulary[list_index++]))
        {
            if (strncasecmp(name, text, len) == 0)
            {
                if (islower(*text))
                {
                    // subtract one because of the previous list_index++ in the
                    // while loop
                    return strdup(lower_vocabulary[list_index - 1]);
                }

                return strdup(name);
            }
        }
    }

    return NULL;
}

char **command_completion(const char *text, int start, int end)
{
    (void)start;
    (void)end;

    rl_attempted_completion_over = 1;

    return rl_completion_matches(text, command_generator);
}

void init_readline(void)
{
    rl_redisplay_function = custom_display;
    rl_attempted_completion_function = command_completion;

    //            right arrow
    rl_bind_keyseq("\\e[C", accept_suggestion);
}

char *get_readline_prompt(char *dbname, char *host)
{
    int len = snprintf(NULL, 0, PROMPT_TEMPLATE_STR, dbname, host) + 1;
    char *readline_str = malloc(len * sizeof(char));
    if (readline_str == NULL)
    {
        fprintf(stderr, "failed to allocate memory for readline string\n");
        exit(EXIT_FAILURE);
    }

    snprintf(readline_str, len, PROMPT_TEMPLATE_STR, dbname, host);

    return readline_str;
}
