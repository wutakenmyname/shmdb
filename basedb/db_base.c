#include "db_base.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <openssl/sha.h>
#include <fcntl.h>
#include "list.h"
#include "mbool.h"

#define MY_DEFALUT_SHM_SIZE (1024 * 1024 * 2)

#define mprintf(fmt, ...) printf("[%s,%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)

#define validate_string_and_length(str, len, min_len, max_len) ({ \
    STATUS_T status = STATUS_OK;                                  \
    if (!str)                                                     \
    {                                                             \
        printf("%s is null\n", #str);                            \
        status = STATUS_NOK;                                      \
    }                                                             \
    if (len < min_len)                                            \
    {                                                             \
        printf("%s is less than %d\n", #len, min_len);           \
        status = STATUS_NOK;                                      \
    }                                                             \
    if (len > max_len)                                            \
    {                                                             \
        printf("%s is greater than %d\n", #len, max_len);        \
        status = STATUS_NOK;                                      \
    }                                                             \
    status;                                                       \
})

#define validate_string_and_length_with_log_func(str, len, max_len, min_len, log_func) ({ \
    STATUS_T status = STATUS_OK;                                                          \
    if (!str)                                                                             \
    {                                                                                     \
        log_func("%s is null\n", #str);                                                   \
        status = STATUS_NOK;                                                              \
    }                                                                                     \
    if (len < min_len)                                                                    \
    {                                                                                     \
        log_func("%s is less than %d\n", #len, min_len);                                  \
        status = STATUS_NOK;                                                              \
    }                                                                                     \
    if (len > max_len)                                                                    \
    {                                                                                     \
        log_func(" %s is greater than %d\n", #len, max_len);                              \
        status = STATUS_NOK;                                                              \
    }                                                                                     \
    status;                                                                               \
})

    int current_version = 1;

typedef struct
{
    base_db_t db;
    IF_HAVE_FILE_BASE have;
    char file_base_path[FILE_BASE_PATH_LENGTH];
    DB_HASH_METHOD method;
    unsigned char *db_content;
    int shm_id;
    int shm_size;
    struct list_head linker;
} db_struct_t;

typedef struct
{
    int version;
    int data_offset;
    int hash_method;
    int hash_offset;
    int hash_length;
} file_base_header;

static bool initialized = false;
static struct list_head db_collection;

static void init()
{
    if (initialized == false)
    {
        initialized = true;
        INIT_LIST_HEAD(&db_collection);
    }
}

static STATUS_T compute_sha256(char *data, int data_length, unsigned char *sha256_value)
{
    if (data == NULL || data_length <= 0)
    {
        mprintf("params are not valid\n");
        return STATUS_NOK;
    }

    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, data_length);
    SHA256_Final(sha256_value, &sha256);

    return STATUS_OK;
}

static int get_hash_data_size(DB_HASH_METHOD method)
{
    int size = -1;
    if (method >= DB_HASH_METHOD_INVALID)
    {
        method = DB_HASH_MDTHOD_FIRST_VALID;
    }
    switch (method)
    {
    case DB_HASH_METHOD_SHA256:
        size = 32; // bytes
    default:
        size = -1;
    }

    return size;
}

void iterate_all_db()
{
    db_struct_t *iterator = NULL;
    list_for_each_entry(iterator, &db_collection, linker)
    {
        mprintf("goes here\n");
        if (iterator != NULL)
        {
            mprintf("iterator->db :%d\n", iterator->db);
        }
        else
            break;
    }
}

static db_struct_t *new_db_struct()
{
    db_struct_t *db_struct = NULL;

    db_struct = (db_struct_t *)malloc(sizeof(db_struct_t));

    if (db_struct == NULL)
    {
        mprintf("malloc failed\n");
        return NULL;
    }

    memset(db_struct, 0, sizeof(db_struct_t));
    db_struct->shm_id = -1;
    db_struct->have = HAVE_NO_FILE_BASE;
    db_struct->method = DB_HASH_MDTHOD_FIRST_VALID;
    db_struct->db = -1;

    INIT_LIST_HEAD(&(db_struct->linker));
    mprintf("new db_struct %p, linker: %p\n", db_struct, &(db_struct->linker));
    list_add(&(db_struct->linker), &db_collection);
    db_struct_t *iterator = NULL;
    list_for_each_entry(iterator, &db_collection, linker)
    {
        mprintf("goes here\n");
        if (iterator != NULL)
        {
            mprintf("iterator->db :%d\n", iterator->db);
        }
        else
            break;
    }

    return db_struct;
}

static STATUS_T delete_db_struct(db_struct_t *db_struct)
{
    if (db_struct == NULL)
    {
        mprintf("db_struct is null\n");
        return STATUS_NOK;
    }

    struct list_head *temp = NULL;
    temp = &(db_struct->linker);
    #if 0
    if (db_collection == temp)
    {
        if (temp->next == temp->prev && temp->next == temp)
        {
            list_del(temp);
            db_collection = NULL;
        }
        else
        {
            db_collection = &(temp->next);
            list_del(temp);
        }
    }
    else
    {
        list_del(temp);
    }
    #endif
    list_del(temp);

    free(db_struct);
    return STATUS_OK;
}

static db_struct_t *find_db_struct(base_db_t id)
{
    db_struct_t *ret = NULL;
    db_struct_t *iterator = NULL;
    mprintf("db wanted id %d\n", id);

    list_for_each_entry(iterator, &db_collection, linker)
    {
        mprintf("iterator addr: %p\n", iterator);
        if (iterator->db == id)
        {
            ret = iterator;
            break;
        }
    }

    return ret;
}

static STATUS_T prepare_shm(db_struct_t *db_struct)
{
    STATUS_T ret = STATUS_NOK;
    int shm_id = -1;
    struct shmid_ds setting;
    void *shm_addr = NULL;

    if (db_struct == NULL)
    {
        mprintf("db_struct is null\n");
        return STATUS_NOK;
    }

    shm_id = shmget(db_struct->db, db_struct->shm_size, IPC_CREAT | 0666);
    if (shm_id < 0)
    {
        mprintf("get shm failed\n");
        return STATUS_NOK;
    }

    if (shmctl(shm_id, IPC_STAT, &setting) < 0)
    {
        mprintf("get shm info failed\n");
        return STATUS_NOK;
    }
    setting.shm_segsz = db_struct->shm_size;
    if (shmctl(shm_id, IPC_SET, &setting))
    {
        mprintf("set shm params failed\n");
        return STATUS_NOK;
    }

    shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr == (void *)-1)
    {
        mprintf("shm failed\n");
        return STATUS_NOK;
    }

    db_struct->db_content = shm_addr;
    return STATUS_OK;
}

STATUS_T db_get(int shm_key, base_db_t *base_db, int shm_size, DB_HASH_METHOD method)
{
    init();
    if (base_db == NULL)
    {
        mprintf("base_db is null\n");
        return STATUS_NOK;
    }

    db_struct_t *db_struct = NULL;
    db_struct = find_db_struct(shm_key);

    if (db_struct != NULL)
    {
        return STATUS_OK;
    }

    db_struct = new_db_struct();

    if (db_struct == NULL)
    {
        mprintf("new_db_struct failed\n");
        return STATUS_NOK;
    }

    mprintf("shm_key :%d\n", shm_key);
    mprintf("db_struct addr :%p\n", db_struct);
    db_struct->db = shm_key;

    if (shm_size <= 0)
    {
        db_struct->shm_size = MY_DEFALUT_SHM_SIZE;
    }
    db_struct->shm_size = shm_size;

    if (method < DB_HASH_MDTHOD_FIRST_VALID || method >= DB_HASH_METHOD_INVALID)
    {
        db_struct->method = DB_HASH_MDTHOD_FIRST_VALID;
    }
    db_struct->method = method;

    *base_db = db_struct->db;

    if (prepare_shm(db_struct) == STATUS_NOK)
    {
        mprintf("prepare_shm failed\n");
        return STATUS_NOK;
    }
    iterate_all_db();
    return STATUS_OK;
}

STATUS_T db_set_file_base(base_db_t db, char *file_base_path, int file_base_path_len)
{
    if (validate_string_and_length(file_base_path, file_base_path_len, 0, FILE_BASE_PATH_LENGTH) == STATUS_NOK)
    {
        return STATUS_NOK;
    }

    db_struct_t *db_struct = NULL;
    db_struct = find_db_struct(db);
    if (db_struct == NULL)
    {
        mprintf("can not find db_struct for %d\n", db);
        return STATUS_NOK;
    }

    db_struct->have = HAVE_FILE_BASE;
    memcpy(db_struct->file_base_path, file_base_path, file_base_path_len);
    return STATUS_OK;
}

bool old_file_base_exist(base_db_t db)
{
    db_struct_t *db_struct = NULL;
    db_struct = find_db_struct(db);
    if (db_struct == NULL)
    {
        mprintf("cannot find db_struct for %d\n", db);
        return false;
    }

    if (access(db_struct->file_base_path, F_OK) == 0)
    {
        return true;
    }

    return false;
}

STATUS_T db_get_old_file_base_content(base_db_t db, unsigned char **data, int *data_len)
{
    db_struct_t *db_struct = NULL;
    db_struct = find_db_struct(db);
    if (db_struct == NULL)
    {
        mprintf("cannot find db_struct for %d\n", db);
        return false;
    }

    if (data == NULL || data_len == NULL)
    {
        mprintf("null data or data_len\n");
        return STATUS_NOK;
    }

    char *temp_content = NULL;
    int file_fd = -1;
    int size = -1;
    struct stat stat;

    file_fd = open(db_struct->file_base_path, O_RDONLY);
    if (file_fd < 0)
    {
        mprintf("open base file failed\n");
        return STATUS_OK;
    }
    else
    {
        if (fstat(file_fd, &stat) < 0)
        {
            mprintf("stat file failed\n");
            return STATUS_OK;
        }

        if (stat.st_size == 0)
        {
            mprintf("empty file\n");
            close(file_fd);
            return STATUS_NOK;
        }

        file_base_header header;
        if (read(file_fd, &header, sizeof(header)) != sizeof(header))
        {
            mprintf("read header failed\n");
            close(file_fd);
            return STATUS_NOK;
        }

        mprintf("header: version:%d, data_offset: %d, hash_method: %d, hash_data_length: %d, hash_offset:%d\n",
                header.version, header.data_offset, header.hash_method, header.hash_length, header.hash_offset);

        int content_length = header.hash_offset - header.data_offset;
        mprintf("content length: %d\n", content_length);
        if (content_length <= 0)
        {
            mprintf("content length is not greater than 0\n");
            close(file_fd);
            return STATUS_NOK;
        }

        unsigned char *content = (unsigned char *)malloc(content_length);
        if (data == NULL)
        {
            mprintf("malloc failed \n");
            close(file_fd);
            return STATUS_NOK;
        }

        memset(content, 0, content_length);

        if (read(file_fd, content, content_length) != content_length)
        {
            mprintf("read failed\n");
            close(file_fd);
            free(content);
            return STATUS_NOK;
        }

        *data = content;
        *data_len = content_length;
    }
    return STATUS_OK;
}

void db_release_old_file_base_content(unsigned char *data)
{
    if (data == NULL)
    {
        mprintf("data is null\n");
        return;
    }

    free(data);
    return ;
}

void *db_retrieve_access(base_db_t base_db)
{
    db_struct_t *db = NULL;

    db = find_db_struct(base_db);
    if (db == NULL)
    {
        mprintf("doesn't find the db\n");
        return NULL;
    }

    return db->db_content;
}

STATUS_T db_put(base_db_t base_db)
{
    db_struct_t *db_struct = NULL;
    db_struct = find_db_struct(base_db);
    if (db_struct == NULL)
    {
        mprintf("cannot find a db with id %d\n", base_db);
        return STATUS_NOK;
    }

    if (shmdt(db_struct->db_content) < 0)
    {
        mprintf("shmdt failed for db with id %d\n", base_db);
        return STATUS_NOK;
    }

    db_struct->db_content = NULL;
    return STATUS_OK;
}

STATUS_T db_commit_to_file_base(base_db_t base_db, int *data, int data_length)
{
    db_struct_t *db_struct;
    file_base_header header;
    int hash_data_size = -1;
    char *hash_value = NULL;
    size_t bytes_written;

    db_struct = find_db_struct(base_db);
    if (db_struct == NULL)
    {
        mprintf("can not find db_struct by this id\n");
        return STATUS_NOK;
    }

    hash_data_size = get_hash_data_size(db_struct->method);
    if (hash_data_size <= 0)
    {
        mprintf("error: hash data size lesss than 0\n");
        return STATUS_NOK;
    }

    hash_value = (char *)malloc(hash_data_size);
    if (hash_value == NULL)
    {
        mprintf("malloc memory for hash value failed\n");
        return STATUS_NOK;
    }

    memset(hash_value, 0, hash_data_size);
    if (db_struct->method == DB_HASH_METHOD_SHA256)
    {
        if (compute_sha256(data, data_length, hash_value) == STATUS_NOK)
        {
            mprintf("compute_sha256 failed\n");
            free(hash_value);
            return STATUS_NOK;
        }
    }

    header.version = current_version;
    header.data_offset = sizeof(header);
    header.hash_offset = data_length + header.data_offset;
    header.hash_length = hash_data_size;
    header.hash_method = db_struct->method;
    // remove(db_struct->file_base_path);

    FILE *file = fopen(db_struct->file_base_path, "wb");
    if (file == NULL)
    {
        mprintf("Failed to open file");
        free(hash_value);
        return STATUS_NOK;
    }

    // 将二进制数据写入文件
    bytes_written = fwrite(&header, sizeof(unsigned char), sizeof(header), file);
    if (bytes_written != sizeof(header))
    {
        mprintf("Failed to write data to file");
        fclose(file);
        free(hash_value);
        remove(db_struct->file_base_path);
        return STATUS_NOK;
    }

    bytes_written = fwrite(data, sizeof(unsigned char), data_length, file);
    if (bytes_written != data_length)
    {
        mprintf("Failed to write data to file");
        fclose(file);
        free(hash_value);
        remove(db_struct->file_base_path);
        return STATUS_NOK;
    }

    bytes_written = fwrite(hash_value, sizeof(unsigned char), hash_data_size, file);
    if (bytes_written != hash_data_size)
    {
        mprintf("Failed to write data to file");
        fclose(file);
        free(hash_value);
        remove(db_struct->file_base_path);
        return STATUS_NOK;
    }

    // 关闭文件
    fclose(file);

    free(hash_value);
    return STATUS_OK;
}
