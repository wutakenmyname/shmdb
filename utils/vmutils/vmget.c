#include "vmdb.h"
#include "customization_vm.h"
#include <stdio.h>
#include <unistd.h>

static void print_help()
{
    printf("################\n");
    printf("-h  print help\n");
    printf("-t  type\n");
    printf("-l  specify length (1: byte, 2:2bytes, 3:3bytes...n:nbytes)\n");
    printf("-f  specify format (1: char, 2:uint8, 3:uint16, 4:uint32, 5:uint64, 6:float, 7:double, 8:string)\n");
}

int main(int argc, char *argv[])
{
    STATUS_T ret = STATUS_NOK;
    data_type_t type = DB_FIRST_MEMBER;
    int length = -1;
    uint8 *value = NULL;
    int format = -1;
    

    if (argc != 7)
    {
	printf("argc:%d is not right\n", argc);
        print_help();
        return -1;
    }

    int opt;

    while ((opt = getopt(argc, argv, "t:l:f:h:")) != -1) {
        switch (opt) {
            case 't':
                type = atoi(optarg);
                printf("Option -t detected. Value: %d\n", type);
                break;
            case 'l':
                length = atoi(optarg);
                printf("Option -l detected. Value: %d\n", length);
                break;
            case 'f':
                format = atoi(optarg);
                printf("Option -f detected. Value: %d\n", format);
                break;
            case 'h':
                print_help();
                return 1;
            default:
                fprintf(stderr, "Usage: %s -t value -l value\n", argv[0]);
                return 1;
        }
    }

    ret = vmdb_init_once();
    if (ret == STATUS_NOK)
    {
        printf("vmdb_init_once failed\n");
        return -1;
    }
    
    value = (uint8 *)malloc(length+1);
    if (value == NULL)
    {
        printf("malloc failed, length :%d\n", length);
        return -1;
    }

    memset(value, 0, length + 1);
    if (vmdb_get_data(type, value, length) == STATUS_OK)
    {
        switch (format)
        {
            case 1:
            {
                printf("value: %c\n", (char) *value);
                break;
            }
            case 2:
            {
                printf("value: %hhu\n", (uint8) *value);
                break;
            }
            case 3:
            {
                printf("value: %hu\n", (uint16) *value);
                break;
            }
            case 4:
            {
                printf("value: %u\n", (uint32) *value);
                break;
            }
            case 5:
            {
                printf("value: %llu\n", (uint64) *value);
                break;
            }
            case 6:
            {
                printf("value: %f\n", (float) *value);
                break;
            }
            case 7:
            {
                printf("value: %lf\n", (double) *value);
                break;
            }
            case 8:
            {
                printf("value: %s\n", (char *) *value);
                break;
            }
        }
    }
}
