#include "uio_manager.h"


#include "config.h"
#include "utils.h"


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

#include <getopt.h>
#include <signal.h>

#include <poll.h>


static int running = false;


static void sig_handler(int signum);



void sig_handler(int signum)
{
    printf("\n[%s] SIGNUM: %d\n", PROG_NAME, signum);

    if (signum == SIGINT)
    {
        running = false;
    }

    if (signum == SIGKILL)
    {
        running = false;
    }
}

void uiom_print_args_usage(FILE *fs)
{
    fprintf(fs, "Usage: %s [-h] [-a tcp server ip] [-j joystick]\n"
            "  -a   tcp server ip address (default %s)\n"
            "  -j   joystick device   (default %s)\n"
            "  -h   display help\n",
            PROG_NAME,
            DEFAULT_MAV_CONN_TCP_SERVER_ADDR,
            DEFAULT_JS_DEVICE);

    fprintf(fs,"\n\n");
}

void uiom_print_config(FILE *fs, config_t* config)
{
    fprintf(fs,"\n");
    fprintf(fs,"\n");

    fprintf(fs,"------------------------------------\n");
    fprintf(fs,"    [%s] Settings                   \n", PROG_NAME);
    fprintf(fs,"------------------------------------\n");
    fprintf(fs, "\n");
    fprintf(fs, "\n");

    fprintf(fs, "joystick:\n");
    fprintf(fs, "    device              = %s\n",config->js.dev);
    fprintf(fs, "\n");
    fprintf(fs, "\n");

    fprintf(fs, "mav connection (px4 quadrotor):\n");
    fprintf(fs, "    tcp server address  = %s\n",config->mav_conn.server.ip);
    fprintf(fs, "    tcp server port     = %d\n",config->mav_conn.server.port);
    fprintf(fs, "\n");
    fprintf(fs, "\n");

    fprintf(fs, "qgroundcontrol connection:\n");
    fprintf(fs, "    udp server address  = %s\n",config->qgc_conn.server.ip);
    fprintf(fs, "    udp server port     = %d\n",config->qgc_conn.server.port);
    fprintf(fs, "    udp client address  = %s\n",config->qgc_conn.client.ip);
    fprintf(fs, "    udp client port     = %d\n",config->qgc_conn.client.port);
    fprintf(fs, "\n");
    fprintf(fs, "\n");


    fflush(fs);
}


void uiom_parse_args(int argc, char *argv[], config_t* config)
{
    int opt, slen;

    char *temp;

    while ((opt = getopt(argc, argv, "hj:a:")) != -1)
    {
        switch (opt)
        {
        case 'a':
            slen = strlen(optarg);
            temp= (char*) calloc(slen + 1, sizeof(char));
            strncpy(temp, optarg, slen);
            config->mav_conn.server.ip = temp;
            break;

        case 'j':
            slen = strlen(optarg);
            temp = (char*) calloc(slen + 1, sizeof(char));
            strncpy(temp, optarg, slen);
            config->js.dev = temp;
            break;

        case 'h':
            uiom_print_args_usage(stdout);
            exit(EXIT_SUCCESS);

        default:
            uiom_print_args_usage(stderr);
            exit(EXIT_FAILURE);
        }
    }
}


void uiom_init_sig_handler()
{
    running = true;

    /** register sigint handler for ordered exit */
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = sig_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGKILL, &sa, NULL);
}


int uiom_is_exit_key_pressed()
{
    char c;

    struct pollfd fds;
    int ret;

    int fd_stdin = 0;  //file descriptor number of stdin file pointer is always 0

    fds.fd = fd_stdin; /* stdin */
    fds.events = POLLIN;
    ret = poll(&fds, 1, 0);

    if (ret > 0)
    {
        printf("key detected...\n");

        read(fd_stdin, &c, 1);

        switch (c)
        {
        case 0x03: // ctrl-c
        case 0x1b: // esc
        case 'c':
        case 'q':
            return true;
            /* not reached */
        }
    }

    return false;
}

int uiom_is_prog_running(void)
{
    return running;
}

void uiom_stop_running(void)
{
    running = false;
}


void uiom_cleanup(config_t* config)
{
    printf("\n[%s] EXIT program ...\n", PROG_NAME);
    fflush(stdout);

    if(config->mav_conn.sd > 0)
    {
        close(config->mav_conn.sd);
    }

    if(config->qgc_conn.sd > 0)
    {
        close(config->qgc_conn.sd);
    }

    if(config->js.fd > 0)
    {
        close(config->js.fd);
    }

    if(config->tracker_conn.sd > 0)
    {
        close(config->tracker_conn.sd);
    }

    ml_logger_close_files(&config->mll);
    np_logger_close_files(&config->npl);

    utils_make_dir("log");

    printf("[%s] clean log dir ...\n", PROG_NAME);
    utils_delete_files_from_dir("log");

    printf("[%s] copy log files to log dir ...\n", PROG_NAME);
    utils_copy_files_of_dir(config->log_dir_path,"log");

    printf("\n");
    printf("\n");

}
