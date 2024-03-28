#include "vmdb.h"
include "customization.h"
#include <stdio.h>
#include <unistd.h>

static void print_help()
{
    printf("###############\n");
    printf("-h  print help\n");
    printf("-t  type\n");
    printf("-l  specify length (1: 1byte, 2:2bytes... n:nbytes)\n");
}

int main(char *argv[], int argc)
{
    data_type_t type = DB_FIRST_MEMBER;
    int length = -1;

    if (argc != 4)
    {
        print_help();
        return -1;
    }

    int opt;

    while ((opt = getopt(argc, argv, "t:l:")) != -1) {
        switch (opt) {
            case 't':
                type = atoi(optarg);
                printf("Option -t detected. Value: %d\n", t_value);
                break;
            case 'l':
                length = atoi(optarg);
                printf("Option -l detected. Value: %d\n", l_value);
                break;
            default:
                fprintf(stderr, "Usage: %s -t value -l value\n", argv[0]);
                return 1;
        }
    }
}