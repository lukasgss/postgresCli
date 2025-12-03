#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "table.h"

#define STACK_TRESHOLD 1024
#define SPACING_VALUES_BETWEEN_LINES 4

struct cols_data
{
    int count;
    unsigned long *col_widths;
};

static struct cols_data get_total_width(struct print_table_info print_info)
{
    struct cols_data cols_data = {.col_widths = NULL, .count = 0};
    // TODO: resize dinamically
    cols_data.col_widths = malloc(sizeof(unsigned long) * 10);

    for (int i = 0; i < print_info.amount_cols; i++)
    {
        unsigned long biggest_len = strlen(print_info.cols[i]);

        for (int j = 0; j < print_info.amount_rows; j++)
        {
            unsigned long len = strlen(print_info.rows[j][i]);

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

void print_cols_header(char **cols, int num_cols, struct cols_data columns_data)
{
    unsigned long total_width = 1;

    for (int i = 0; i < columns_data.count; i++)
    {
        total_width += columns_data.col_widths[i] - 1;
    }

    char stack_buf[total_width <= STACK_TRESHOLD ? total_width + 1 : 1];
    char *line =
        total_width <= STACK_TRESHOLD ? stack_buf : malloc(total_width + 1);

    // top border
    char *p = line;
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

    // col names
    p = line;
    for (int i = 0; i < columns_data.count; i++)
    {
        unsigned long width = columns_data.col_widths[i];
        size_t text_len = strlen(cols[i]);
        size_t padding = width - SPACING_VALUES_BETWEEN_LINES - text_len;

        *p++ = '|';
        *p++ = ' ';
        memcpy(p, cols[i], text_len);
        p += text_len;
        memset(p, ' ', padding);
        p += padding;
        *p++ = ' ';
    }
    *p++ = '|';
    *p = '\0';
    puts(line);

    // bottom border
    p = line;
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

    if (total_width > STACK_TRESHOLD)
    {
        free(line);
    }
}

void draw_table(struct print_table_info print_info)
{
    struct cols_data columns_data = get_total_width(print_info);

    print_cols_header(print_info.cols, print_info.amount_cols, columns_data);
}
