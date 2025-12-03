#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "table.h"

#define STACK_THRESHOLD 1024
#define SPACING_VALUES_BETWEEN_LINES 4

struct cols_data
{
    int count;
    unsigned long *col_widths;
};

static struct cols_data get_total_width(struct print_table_info print_info)
{
    struct cols_data cols_data = {.col_widths = NULL, .count = 0};
    cols_data.col_widths =
        malloc(sizeof(unsigned long) * print_info.amount_cols);

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

void print_cols_header(
    char **cols, struct cols_data columns_data, unsigned long total_width)
{
    char stack_buf[total_width <= STACK_THRESHOLD ? total_width + 1 : 1];
    char *line =
        total_width <= STACK_THRESHOLD ? stack_buf : malloc(total_width + 1);

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

    // col names
    p = &line[0];
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

    if (total_width > STACK_THRESHOLD)
    {
        free(line);
    }
}

void print_rows(
    char ***rows, int num_rows, struct cols_data columns_data,
    unsigned long total_width)
{
    char stack_buf[total_width <= STACK_THRESHOLD ? total_width + 1 : 1];
    char *line =
        total_width <= STACK_THRESHOLD ? stack_buf : malloc(total_width + 1);

    char *p = line;

    // print contents
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < columns_data.count; j++)
        {
            unsigned long width = columns_data.col_widths[j];
            *p++ = '|';
            *p++ = ' ';

            size_t text_len = strlen(rows[i][j]);
            size_t padding = width - SPACING_VALUES_BETWEEN_LINES - text_len;

            memcpy(p, rows[i][j], text_len);
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

void draw_table(struct print_table_info print_info)
{
    struct cols_data columns_data = get_total_width(print_info);

    unsigned long total_width = 1;

    for (int i = 0; i < columns_data.count; i++)
    {
        total_width += columns_data.col_widths[i] - 1;
    }

    print_cols_header(print_info.cols, columns_data, total_width);

    print_rows(
        print_info.rows, print_info.amount_rows, columns_data, total_width);

    free(columns_data.col_widths);
}
