#include "db_base.h"
#include <stdio.h>
#include <cstddef.h>
#include <stdlib.h>
#include "list.h"

#define LONGEST_SIGNATURE 128
#define SIGNATURE_PREFIX_LENGHT 3
#define SIGNATURE_SUFFIX_LENGTH 3
#define SIGNATURE_PREFIX "\0\r\n"
#define SIGNATURE_SUFFIX "\0\r\n"

#define validate_string_and_length(str, len, min_len, max_len) ({\
    STATUS_T status = STATUS_OK;\
    if (!str) \
    { \
        printf("%s is null\n", #str); \
        status = STATUS_NOK; \
    } \
    if (len < min_len) \
    { \
        printf("%s is less than %d\n", #len, min_len); \
        status = STATUS_NOK; \
    }\
    if (len > max_len) \
    { \
        printf("%s is greater than %d\n", #len, max_len); \
        status = STATUS_NOK; \
    }\
    status; \
})

#define validate_string_and_length_with_log_func(str, len, max_len, min_len, log_func) ({\
    STATUS_T status = STATUS_OK;\
    if (!str) \
    { \
        log_func("%s is null\n", #str); \
        status = STATUS_NOK; \
    } \
    if (len < min_len) \
    { \
        log_func("%s is less than %d\n", #len, min_len); \
        status = STATUS_NOK; \
    }\
    if (len > max_len) \
    { \
        log_func(" %s is greater than %d\n", #len, max_len); \
        status = STATUS_NOK; \
    }\
    status; \
})


char default_signature[LONGEST_SIGNATURE] = {0};

typedef struct
{
    base_db_t db;
    IF_HAVE_FILE_BASE have;
    char file_base_path[FILE_BASE_PATH_LENGTH];
    DB_HASH_METHOD method;
    char signature[SIGNATURE_PREFIX_LENGHT + SIGNATURE_SUFFIX_LENGTH + LONGEST_SIGNATURE];
    char *db_content;
    struct list_head linker;
}db_struct_t;

static LIST_HEAD(db_collection);

static db_struct_t *new_db_struct()
{
    db_struct_t *db_struct = NULL;

    db_struct = (db_struct_t *)malloc(sizeof(db_struct_t));

    if (db_struct == NULL)
    {
        printf("[%s,%d] malloc failed\n");
        return NULL;
    }

    memset(db_struct, 0, sizeof(db_struct_t));

    memcpy(db_struct->signature, SIGNATURE_PREFIX, SIGNATURE_PREFIX_LENGHT);
    memcpy(db_struct->signature + SIGNATURE_PREFIX_LENGHT + LONGEST_SIGNATURE, SIGNATURE_SUFFIX, SIGNATURE_SUFFIX_LENGTH);

    LIST_HEAD_INIT(db_struct->linker);
    list_add(&(db_struct->linker), db_collection);
}

static delete_db_struct(db_struct_t *db_struct)
{
    list_del(&(db_struct->linker));
    free(db_struct);
}

STATUS_T db_get(char *base_db_name, int base_db_name_len, base_db_t *base_db, IF_HAVE_FILE_BASE have, char *file_base_path, int file_base_path_len, DB_HASH_METHOD method, char *signature, int signaure_len);
{
    if (validate_string_and_length(base_db_name, base_db_name_len, 1, 9999) == STAUTS_NOK)
    {
            return STATUS_NOK;
    }

    if (base_db == NULL)
    {
        printf("base_db is null\n");
        return STATUS_NOK;
    }

    if (have == HAVE_FILE_BASE)
    {
        if (validate_string_and_length(file_base_path, file_base_path_len, 1, FILE_BASE_PATH_LENGTH) == STAUTS_NOK)
        {
            return STATUS_NOK;
        }
    }


    db_struct_t *db_struct = new_db_struct();

    if (db_struct == NULL)
    {
        printf("new_db_struct failed\n");
        return STATUS_NOK;
    }

    if (IF_HAVE_FILE_BASE == HAVE_FILE_BASE)
    {
        db_struct->have = HAVE_FILE_BASE;
        if (method == DB_HASH_METHOD_INVALID)
        {
            printf("hash method provided is invalid, use first valid method\n");
            db_struct->method = DB_HASH_MDTHOD_FIRST_VALID;
        }

        if (signature == NULL || signature_len <= 0)
        {
            printf("db_signature is null, use default signature\n");
            memcpy(db_struct->signature + SIGNATURE_PREFIX, default_signature, sizeof(default_signature) > LONGEST_SIGNATURE ? LONGEST_SIGNATURE : sizeof(default_signature));
        }
        else
        {
            memcpy(db_struct->signature + SIGNATURE_PREFIX, signature, signature_len);
        }
    }
    return STATUS_OK;
}

STATUS_T generate_file_base_path(char *base_db_name, int base_db_name_len, char *file_base_dir, int file_base_dir_len, char *file_base_path_generated, int *file_base_path_generated_len);
{
    if (validate_string_and_length(base_db_name, base_db_name_len, 1, FILE_BASE_NAME_LENGTH) == STAUTS_NOK ||
        validate_string_and_length(file_base_dir, file_base_dir_len, 1, FILE_BASE_DIR_LENGTH) == STAUTS_NOK)
    {
        return STATUS_NOK;
    }

    if (file_base_path_generated == NULL)
    {
        printf("file_base_path_generated is NULL\n");
        returun STATUS_NOK;
    }
    if (file_base_path_generated_len == NULL)
    {
        printf("file_base_path_generated_len is NULL\n");
        returun STATUS_NOK;
    }
    return STATUS_OK;
}

STATUS_T db_retrieve_access(base_db_t base_db, void *content)
{
    return STATUS_OK;
}

STATUS_T db_deinit(base_db_t base_db)
{
    return STATUS_OK;
}

STATUS_T db_commit(base_db_t base_db)
{
    return STATUS_OK;
}
