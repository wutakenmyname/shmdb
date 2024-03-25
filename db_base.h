#ifndef _DB_BASE_H
#define _DB_BASE_H

#include "status.h"

#define FILE_BASE_NAME_LENGTH 128
#define FILE_BASE_DIR_LENGHT 128
#define FILE_BASE_PATH_LENGTH (FILE_BASE_NAME_LENGTH + FILE_BASE_DIR_LENGHT)

typedef enum
{
    DB_HASH_MDTHOD_FIRST_VALID = 0,
    DB_HASH_METHOD_SHA256 = DB_HASH_MDTHOD_FIRST_VALID,
    DB_HASH_METHOD_INVALID
}DB_HASH_METHOD;

typedef enum
{
   HAVE_NO_FILE_BASE = 0,
   HAVE_FILE_BASE
}IF_HAVE_FILE_BASE;

typedef unsigned int base_db_t;

STATUS_T db_get(char *base_db_name, int base_db_name_len, base_db_t *base_db, IF_HAVE_FILE_BASE have, char *file_base_path, int file_base_path_len, DB_HASH_METHOD method, char *signature, int signaure_len);
STATUS_T db_retrieve_access(base_db_t base_db, void *content);
STATUS_T db_deinit(base_db_t base_db);
STATUS_T db_commit(base_db_t base_db);

#endif
