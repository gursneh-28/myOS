#ifndef SHELL_H
#define SHELL_H

void shell_init();
void shell_handle_char(char c);
void shell_history_up();
void shell_history_down();

#endif