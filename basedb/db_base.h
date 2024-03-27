#ifndef _DB_BASE_H
#define _DB_BASE_H

#include "status.h"
#include "mbool.h"

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

typedef int base_db_t;
typedef void (*parse_func_t)(unsigned char *data, int data_len);
typedef void (*construct_func_t)(unsigned char *data, int data_len);

STATUS_T db_get(int shm_key, base_db_t *base_db, int shm_size, DB_HASH_METHOD method);
void *db_retrieve_access(base_db_t base_db);
STATUS_T db_set_file_base(base_db_t db, char *file_base_path, int file_base_path_len);
bool old_file_base_exist(base_db_t db);
STATUS_T db_get_old_file_base_content(base_db_t db, unsigned char **data, int *data_len);
void db_release_old_file_base_content(unsigned char *data);
STATUS_T db_put(base_db_t base_db);
STATUS_T db_commit_to_file_base(base_db_t base_db, int *data, int data_length);

#endif
