#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "table.h"
#include "ui/terminal.h"

#define STACK_THRESHOLD 1024
#define SPACING_VALUES_BETWEEN_LINES 4

struct cols_data
{
    int count;
    unsigned long *col_widths;
};

static struct cols_data get_total_width(
    const struct print_table_info *print_info)
{
    struct cols_data cols_data = {.col_widths = NULL, .count = 0};
    cols_data.col_widths =
        malloc(sizeof(unsigned long) * print_info->amount_cols);

    for (int i = 0; i < print_info->amount_cols; i++)
    {
        unsigned long biggest_len = strlen(print_info->cols[i]);

        for (int j = 0; j < print_info->amount_rows; j++)
        {
            unsigned long len = strlen(print_info->rows[j][i]);

            if (len > biggest_len)
            {
                biggest_len = len;
            }
        }

        // adds the spacing of the value between the table lines
        // and spaces (2 + 2)
        cols_data.col_widths[cols_data.count++] =
            biggest_len + SPACING_VALUES_BETWEEN_LINES;
    }

    return cols_data;
}

void print_cols_header(
    char **cols, struct cols_data columns_data, unsigned long total_width)
{
    /*
    TODO: leaky abstraction. fix this in a way so that the
    length is calculated without the caller knowing about
    this overhead to highlight text
     */
    size_t ansi_overhead =
        (strlen(C_GREEN) + strlen(C_RESET)) * columns_data.count;
    unsigned long buffer_size = total_width + ansi_overhead + 1;

    char stack_buf[buffer_size <= STACK_THRESHOLD ? buffer_size : 1];
    char *line =
        buffer_size <= STACK_THRESHOLD ? stack_buf : malloc(buffer_size);

    if (line == NULL && buffer_size > STACK_THRESHOLD)
    {
        fprintf(stderr, "failed to allocate buffer size to print columns\n");
        return;
    }

    // top border
    char *p = &line[0];
    for (int i = 0; i < columns_data.count; i++)
    {
        unsigned long width = columns_data.col_widths[i];
        *p++ = '+';
        memset(p, '-', width - 2);
        p += width - 2;
    }
    *p++ = '+';
    *p = '\0';
    puts(line);

    // col names with highlighting
    p = &line[0];
    for (int i = 0; i < columns_data.count; i++)
    {
        unsigned long width = columns_data.col_widths[i];
        char *highlighted_text = highlight_by_color(cols[i], HIGHLIGHT_GREEN);

        if (highlighted_text == NULL)
        {
            continue;
        }

        size_t original_text_len = strlen(cols[i]);
        size_t highlighted_len = strlen(highlighted_text);
        size_t padding =
            width - SPACING_VALUES_BETWEEN_LINES - original_text_len;

        *p++ = '|';
        *p++ = ' ';
        memcpy(p, highlighted_text, highlighted_len);
        p += highlighted_len;
        memset(p, ' ', padding);
        p += padding;
        *p++ = ' ';

        free(highlighted_text);
    }
    *p++ = '|';
    *p = '\0';
    puts(line);

    // bottom border
    p = &line[0];
    for (int i = 0; i < columns_data.count; i++)
    {
        unsigned long width = columns_data.col_widths[i];
        *p++ = '+';
        memset(p, '-', width - 2);
        p += width - 2;
    }
    *p++ = '+';
    *p = '\0';
    puts(line);

    if (buffer_size > STACK_THRESHOLD)
    {
        free(line);
    }
}

static unsigned int get_null_count(
    char ***rows, int num_rows, struct cols_data columns_data)
{
    unsigned int null_counter = 0;
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < columns_data.count; j++)
        {
            if (rows[i][j][0] == '\0')
            {
                null_counter++;
            }
        }
    }

    return null_counter;
}

void print_rows(
    char ***rows, int num_rows, struct cols_data columns_data,
    unsigned long total_width)
{
    unsigned int null_counter = get_null_count(rows, num_rows, columns_data);
    size_t buffer_size = total_width + (null_counter * ANSI_OVERHEAD) + 1;

    char stack_buf[buffer_size <= STACK_THRESHOLD ? buffer_size : 1];
    char *line =
        total_width <= STACK_THRESHOLD ? stack_buf : malloc(buffer_size);

    char *p = line;

    // print contents
    for (int i = 0; i < num_rows; i++)
    {
        p = line;

        for (int j = 0; j < columns_data.count; j++)
        {
            unsigned long width = columns_data.col_widths[j];
            *p++ = '|';
            *p++ = ' ';

            char *row_value = rows[i][j];
            bool is_value_empty = row_value[0] == '\0';

            char *value_str = is_value_empty ? highlight_by_color(
                                                   "<null>", HIGHLIGHT_DIM_GRAY)
                                             : rows[i][j];

            size_t text_len = strlen(value_str);

            size_t original_text_len = strlen(row_value);

            // 6 because it's the length of the "<null>" string,
            // the strlen function is not used to improve performance
            // and save a few ms :^)
            size_t display_len = is_value_empty ? 6 : original_text_len;
            size_t padding = width - SPACING_VALUES_BETWEEN_LINES - display_len;

            memcpy(p, value_str, text_len);
            p += text_len;
            memset(p, ' ', padding);
            p += padding;
            *p++ = ' ';
        }

        *p++ = '|';
        *p = '\0';

        puts(line);
    }

    // print bottom of contents
    p = &line[0];
    *p++ = '+';
    for (int i = 0; i < columns_data.count; i++)
    {
        unsigned long width = columns_data.col_widths[i];
        memset(p, '-', width - 2);
        p += width - 2;

        *p++ = '+';
    }

    *p++ = '\0';
    puts(line);

    if (total_width > STACK_THRESHOLD)
    {
        free(line);
    }
}

void draw_table(const struct print_table_info *print_info)
{
    struct cols_data columns_data = get_total_width(print_info);

    unsigned long total_width = 1;

    for (int i = 0; i < columns_data.count; i++)
    {
        total_width += columns_data.col_widths[i] - 1;
    }

    print_cols_header(print_info->cols, columns_data, total_width);

    print_rows(
        print_info->rows, print_info->amount_rows, columns_data, total_width);

    free(columns_data.col_widths);
}
