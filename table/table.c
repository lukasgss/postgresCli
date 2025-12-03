#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "table.h"

struct cols_data
{
    int count;
    unsigned long *col_widths;
};

static struct cols_data get_total_width(struct print_table_info print_info)
{
    struct cols_data cols_data = {.col_widths = NULL, .count = 0};
    cols_data.col_widths = malloc(sizeof(unsigned long) * 10);

    for (int i = 0; i < print_info.amount_cols; i++)
    {
        unsigned long biggest_len = 0;
        for (int j = 0; j < print_info.amount_rows; j++)
        {
            unsigned long len;
            if ((len = strlen(print_info.rows[j])) > biggest_len)
            {
                biggest_len = len;
            }
        }

        // adds the spacing of the value between the table lines
        // and spaces (2 + 2)
        cols_data.col_widths[cols_data.count++] = biggest_len + 4;

        biggest_len = 0;
    }

    return cols_data;
}

void print_cols_header(char **cols, int num_cols, struct cols_data columns_data)
{
    for (int i = 0; i < columns_data.count; i++)
    {
        unsigned long curr_col_width = columns_data.col_widths[i];
        char line[curr_col_width];

        bool is_first_col = i == 0;

        if (is_first_col)
        {
            line[0] = '+';
        }
        memset(&line[is_first_col ? 1 : 0], '-', curr_col_width - 3);
        line[curr_col_width - 2] = '+';
        line[curr_col_width - 1] = '\0';

        printf("%s", line);
    }

    printf("\n");

    for (int i = 0; i < columns_data.count; i++)
    {
        unsigned long curr_col_width = columns_data.col_widths[i];
        char line[curr_col_width];

        bool is_first_col = i == 0;

        if (is_first_col)
        {
            line[0] = '|';
            line[1] = ' ';
        }
        else
        {
            line[0] = ' ';
        }

        size_t text_len = strlen(cols[i]);
        memcpy(&line[is_first_col ? 2 : 1], cols[i], curr_col_width);
        if (text_len < curr_col_width)
        {
            memset(
                &line[text_len + 1 + is_first_col],
                ' ',
                curr_col_width - (text_len + 1 + is_first_col));
        }

        line[curr_col_width - 2] = '|';
        line[curr_col_width - 1] = '\0';

        printf("%s", line);
    }

    printf("\n");
}

void draw_table(struct print_table_info print_info)
{
    struct cols_data columns_data = get_total_width(print_info);

    print_cols_header(print_info.cols, print_info.amount_cols, columns_data);
}
