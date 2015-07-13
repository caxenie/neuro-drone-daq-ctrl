#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "config.h"
#include "joystick.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif



void uiom_print_args_usage(FILE *fs);
void uiom_print_config(FILE *fs, config_t* config);

void uiom_parse_args(int argc, char *argv[], config_t* config);

void uiom_init_sig_handler(void);


int uiom_is_exit_key_pressed(void);
int uiom_is_prog_running(void);
void uiom_stop_running(void);


void uiom_cleanup(config_t* config);

#ifdef __cplusplus
}
#endif

#endif // INPUT_MANAGER_H

