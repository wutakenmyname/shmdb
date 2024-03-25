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

static struct list_head *db_collection = NULL;

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

    INIT_LIST_HEAD(&(db_struct->linker));
    if (db_collection == NULL)
    {
        db_collection = &(db_struct->linker);
    }
    else
    {
        list_add(&(db_struct->linker), db_collection);
    }
}

static delete_db_struct(db_struct_t *db_struct)
{
    list_del(&(db_struct->linker));
    free(db_struct);
}

STATUS_T generate_db_id(char *base_db_name, int base_db_name_len, base_db_t *db_id)
{
    if (validate_string_and_length(base_db_name, base_db_name_len, 1, FILE_BASE_NAME_LENGTH) == STAUTS_NOK)
    {
        return STATUS_NOK;
    }

    if (db_id == NULL)
    {
        printf("db_id is null\n");
        return STATUS_NOK;
    }

    unsigned int hash = 0;
    int i=0;
    for (;i < base_db_name_len; i++)
    {
        hash += *base_db_name++;
    }

    *db_id = hash;
    return STATUS_OK;
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

    if (generate_db_id(base_db_name, base_db_name_len, &(db_struct->db)) == STATUS_NOK)
    {
        delete_db_struct(db_struct);
        return STATUS_NOK
    }

    {
        db_strcut_t db_iterator = NULL;
        list_for_each_entry(db_iterator, db_collection, linker)
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

        memcpy(db_struct->file_base_path, file_base_path, file_base_path_len);
    }

    return STATUS_OK;
}

STATUS_T db_retrieve_access(base_db_t base_db, void *content)
{
    if (content == NULL)
    {
        printf("content is null\n");
        return STATUS_NOK;
    }

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
