#pragma once

#define HIGHLIGHTING_OVERHEAD_IN_BYTES 15

enum HIGHLIGHT_COLOR { HIGHLIGHT_GREEN };

void init_readline(void);
void terminal_disable_echo(void);
void terminal_enable_echo(void);

char *highlight_by_color(const char *str, enum HIGHLIGHT_COLOR color);

#define C_GREEN "\001\033[1;92m\002"  // green
#define C_NUMBER "\001\033[0;33m\002" // orange
#define C_FUNCTION "\033[0;33m"       // yellow
#define C_RESET "\001\033[0m\002"
