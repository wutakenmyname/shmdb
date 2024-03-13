#ifndef _DB_BASE_H
#define _DB_BASE_H

#include <status.h>

typedef enum
{
    DB_HASH_MDTHOD_FIRST_VALID = 0,
    DB_HASH_METHOD_SHA256 = HASH_MDTHOD_FIRST_VALID,
    DB_HASH_METHOD_INVALID
}DB_HASH_METHOD;

typedef enum
{
   DB_NEED_TO_LOAD_FROM_FILE,
   DB_NO_NEED_TO_LOAD_FROM_FILE
}DB_IF_NEED_TO_LOAD_FROM_FILE;

STATUS_T db_init(unsigned int id, DB_HASH_METHOD method, char *signature, int signaure_len, DB_IF_NEED_TO_LOAD_FROM_FILE needed);
STATUS_T db_retrieve_access(void *db_ptr);
STATUS_T db_deinit(unsigned int id);
STATUS_T db_commit(char *file_path);
