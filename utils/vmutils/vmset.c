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
    printf("-f  specify format (1: char, 2:int8, 3:int16, 4:int32, 5:int64, 6:float, 7:double, 8:string)\n");
    printf("-v  specify value\n");
}

int main(int argc, char *argv[])
{
    STATUS_T ret = STATUS_NOK;
    data_type_t type = DB_FIRST_MEMBER;
    int length = -1;
    uint8 *value = NULL;
    int format = -1;
    

    if (argc != 9)
    {
	    printf("argc:%d is not right\n", argc);
        print_help();
        return -1;
    }

    int opt;

    while ((opt = getopt(argc, argv, "t:l:f:h:v:")) != -1) {
        switch (opt) {
            case 't':
                type = atoi(optarg);
                printf("Option -t detected. Value: %d\n", type);
                break;
            case 'l':
                length = atoi(optarg);
                value = (uint8 *)malloc(length+1);
                if (value == NULL)
                {
                    printf("malloc failed, length :%d\n", length);
                    return -1;
                }

                memset(value, 0, length + 1);
                printf("Option -l detected. Value: %d\n", length);
                break;
            case 'f':
                format = atoi(optarg);
                printf("Option -f detected. Value: %d\n", format);
                break;
            case 'v':
                if (format == -1 || length == -1)
                {
                    printf("please set -f and -v first\n");
                    return -1;
                }
                switch (format)
                {
                    case 1:
                    {
                        *((char *)(value)) = *optarg;
                        printf("value: %c\n", *((int8 *)value));
                        break;
                    }
                    case 2:
                    {
                        *((int8 *)(value)) = atoi(optarg);
                        printf("value: %hhd\n", *((int8 *)value));
                        break;
                    }
                    case 3:
                    {
                        *((int16 *)(value)) = atoi(optarg);
                        printf("value: %hd\n", *((int16 *)value));
                        break;
                    }
                    case 4:
                    {
                        *((int32 *)(value)) = atoi(optarg);
                        //printf("value: %d\n", *((int32 *)value));
                        break;
                    }
                    case 5:
                    {
                        *((int64 *)(value)) = atoi(optarg);
                        //printf("value: %lld\n", *((int64 *)value));
                        break;
                    }
                    case 6:
                    {
                        *((float *)(value)) = atof(optarg);
                        //printf("value: %f\n", *((float *)value));
                        break;
                    }
                    case 7:
                    {
                        *((float *)(value)) = strtod(optarg);
                        //printf("value: %lf\n", *((double *)value));
                        break;
                    }
                    case 8:
                    {
                        memcpy(value, optarg, length);
                        //printf("value: %s\n", (char *)value);
                        break;
                    }
                }
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
        free(value);
        return -1;
    }
    
    if (vmdb_set_data(type, value, length) != STATUS_OK)
    {
        printf("set data failed\n");
    }
    free(value);
    return 0;
}
