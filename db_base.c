#include "db_base.h"
#include <stdio.h>
#include <cstddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
include <sys/types.h>
#include <string.h>
#include <openssl/sha.h>
#include <fcntl.h>
#include "list.h"
#include "bool.h"

#define LONGEST_SIGNATURE 128
#define SIGNATURE_PREFIX_LENGHT 3
#define SIGNATURE_SUFFIX_LENGTH 3
#define SIGNATURE_PREFIX "\0\r\n"
#define SIGNATURE_SUFFIX "\0\r\n"
#define DEFALUT_SHM_SIZE (1024 * 1024 * 2)

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
int current_version = 1;

typedef struct
{
    base_db_t db;
    IF_HAVE_FILE_BASE have;
    char file_base_path[FILE_BASE_PATH_LENGTH];
    DB_HASH_METHOD method;
    char signature[SIGNATURE_PREFIX_LENGHT + SIGNATURE_SUFFIX_LENGTH + LONGEST_SIGNATURE];
    unsigned char *db_content;
    int shm_fd;
    int shm_size;
    struct list_head linker;
}db_struct_t;

typedef struct {
    int version;
    int signature_offset;
    int data_offset;
    int hash_offset;
    int hash_length;
}file_base_header;

static struct list_head *db_collection = NULL;

static STATUS_T compute_sha256(char *data, int data_length, unsigned char *sha256_value)
{
    if (data == NULL || data_length <= 0)
    {
        printf("params are not valid\n");
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
        case DB_HASH_MDTHOD_FIRST_VALID:
        case DB_HASH_METHOD_SHA256:
            size = 32; //bytes
        default:
            size = -1;
    }

    return size;
}

static bool is_generated_db_id_unique(base_db_t id)
{
    bool ret = true;
    db_struct_t *iterator = NULL;
    if (db_collection == NULL)
    {
        ret = true;
    }
    else
    {
        list_for_each_entry(iterator, db_collection, linker)
        {
            if (iterator->db == id)
            {
                ret = false;
                break;
            }
        }
    }
    return ret;
} 

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
    db_struct->shm_fd = -1;

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
    if (db_struct == NULL)
    {
        printf("db_struct is null\n");
        return STATUS_NOK;
    }

    struct list_head *temp = NULL;
    temp = &(db_struct->linker);
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

    free(db_struct);
}

static STATUS_T generate_db_id(char *base_db_name, int base_db_name_len, base_db_t *db_id)
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

    unsigned int id = 0;
    int i=0;
    char max_try = 3;
    bool id_ok = false;

    for (;i < base_db_name_len; i++)
    {
        id += *base_db_name++;
    }

    do
    {
        if (is_generated_db_id_unique(id) == false)
        {
            max_try = max_try - 1;
            id = id + 1;
        }
        else
        {
            id_ok = true;
            break;
        }
    } while (max_try > 0)
    
    if (id_ok == false)
    {
        return STATUS_NOK;
    }

    *db_id = hash;
    return STATUS_OK;
}

static db_struct_t  *find_db_struct(base_db_t id)
{
    db_struct_t *ret = NULL
    db_struct_t *iterator = NULL;
    if (db_collection == NULL)
    {
        printf("no db exist\n");
        ret = NULL;
    }
    else
    {
        list_for_each_entry(iterator, db_collection, linker)
        {
            if (iterator->db == id)
            {
                ret = iterator;
                break;
            }
        }
    }

    return ret;
}

static STATUS_T validate_signature(char *signature_indeed, int signature_indeed_len, char *signature_provided, int signature_provided_len)
{
    if (validate_string_and_length(signature_indeed, signature_indeed_len, 1, LONGEST_SIGNATURE) == STAUTS_NOK)
    {
        return STATUS_NOK;
    }
    if (validate_string_and_length(signature_provided, signature_provided_len, 1, LONGEST_SIGNATURE) == STAUTS_NOK)
    {
        return STATUS_NOK;
    }

    if (strncmp(signature_indeed, signature_provided, LONGEST_SIGNATURE) == 0)
    {
        return STATUS_OK;
    }
    
    return STATUS_NOK;
}

static STATUS_T prepare_content(db_struct_t *db_struct)
{
    STATUS_T ret = STATUS_NOK;
    int shm_fd = -1;
    struct shmid_ds setting;
    void *shm_addr = NULL;

    if (db_struct == NULL)
    {
        printf("db_struct is null\n");
        return STATUS_NOK;
    }

    shm_fd = shmget(db_struct->db, IPC_CREAT | 0666);
    if (shm_fd < 0)
    {
        printf("get shm failed\n");
        return STATUS_NOK;
    }

    if (shmctl(shm_fd, IPC_STAT, &setting) < 0)
    {
        printf("get shm info failed\n");
        return STATUS_NOK;
    }
    setting.shm_segsz = db_struct->shm_size;
    if (shmctl(shm_fd, IPC_SET, &setting))
    {
        printf("set shm params failed\n");
        return STATUS_NOK;
    }

    shm_addr = shmat(shm_fd, NULL, 0);
    if (shm_addr == (void *)-1)
    {
        printf("shm failed\n");
        return STATUS_NOK;
    }

    db_struct->db_content = (char *)shm_addr;

    if (db_struct->have = HAVE_FILE_BASE)
    {
        if (access(db_struct->file_base_path, F_OK) == 0)
        {
            // to do
            // load the file
            char *temp_content = NULL;
            int file_fd = -1;
            int size = -1;
            struct stat stat;

            file_fd = open(db_struct->file_base_path, O_RDONLY);
            if (file_fd < 0)
            {
                printf("open base file failed\n");
                return STATUS_OK;
            }
            else
            {
                if (fstat(file_fd, &stat) < 0)
                {
                    printf("stat file failed\n");
                    return STTUS_OK;
                }
                else
                {
                    if(stat.st_size > db_struct->shm_size)
                    {
                        printf("file base size is greater than shm_size\n");
                        return STATUS_OK;
                    }
                    else
                    {
                        if (read(file_fd, db_struct->db_content, stat.st_size) != stat.st_size)
                        {
                            printf("read failed\n");
                            memset(db_struct->db_content, 0, db_struct->shm_size);
                            return STATUS_OK;
                        }
                        else
                        {
                            
                        }
                    }
                }
            }
            
        }
    }
}

STATUS_T db_get(int shm_key, base_db_t *base_db, int shm_size, IF_HAVE_FILE_BASE have, char *file_base_path, int file_base_path_len, DB_HASH_METHOD method, char *signature, int signaure_len);
{
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

    db_struct_t *db_struct = NULL;
    db_struct = find_db_struct(shm_key);

    if (db_struct != NULL)
    {
        char temp_signature[LONGEST_SIGNATURE] = {0};
        if (signature != NULL || signature_len > 0)
        {
            memcpy(temp_signature, signature, signature_len > LONGEST_SIGNATURE ? LONGEST_SIGNATURE : signature_len);
        }
        else
        {
            memcpy(temp_signature, default_signature, LONGEST_SIGNATURE)
        }

        if (validate_signature(db_struct->signature + SIGNATURE_PREFIX, LONGEST_SIGNATURE, temp_signature, LONGEST_SIGNATURE) == STATUS_NOK)
        {
            // signature is not right, can not access it.
            return STATUS_NOK;
        }
        else
        {
            if (db_struct->have == HAVE_NO_FILE_BASE && have == HAVE_NO_FILE_BASE)
            {
                base_db = db_struct->db;
                return STATUS_OK;
            }
            else if (db_struct->have == HAVE_FILE_BASE && have == HAVE_FILE_BASE)
            {
                if (strncmp(db_struct->file_base_path, file_base_path, FILE_BASE_PATH_LENGTH) == 0)
                {
                    base_db = db_struct->db;
                    return STATUS_OK;
                }
            }
            return STATUS_NOK;
        }
    }

    db_struct = new_db_struct();

    if (db_struct == NULL)
    {
        printf("new_db_struct failed\n");
        return STATUS_NOK;
    }

    db_struct->db = shm_key;
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

    if (shm_size <= 0)
    {
        db_struct->shm_size = DEFALUT_SHM_SIZE;
    }

    base_db = db_struct->db;

    if (prepare_content(db_struct) == STATUS_NOK)
    {
        delete_db_struct(db_struct);
        return STATUS_NOK;
    }

    return STATUS_OK;
}

void  *db_retrieve_access(base_db_t base_db)
{
    db_struct_t *db = NULL;
    
    db = find_db_struct(base_db);
    if (db == NULL)
    {
        printf("doesn't find the db\n");
        return NULL;
    }

    return db->content;
}

STATUS_T db_put(base_db_t base_db)
{
    return STATUS_OK;
}

STATUS_T db_commit(base_db_t base_db)
{
    db_struct_t *db_struct;
    file_base_header header;
    int hash_data_size = -1;
    char *hash_value = NULL;
    size_t bytes_written;

    db_struct = find_db_struct(base_db);
    if (db_struct == NULL)
    {
        printf("can not find db_struct by this id\n");
        return STATUS_NOK;
    }

    hash_data_size = get_hash_data_size(db_struct->method);
    if (hash_data_size <= 0)
    {
        printf("error: hash data size lesss than 0\n");
        return STATUS_NOK;
    }

    hash_value= (char *)malloc(hash_data_size);
    if (hash_value == NULL)
    {
        printf("malloc memory for hash value failed\n");
        return STATUS_NOK;
    }

    memset(hash_value, 0, hash_data_size);
    if (compute_sha256(db_struct->db_content, db_struct->shm_size, hash_value) == STATUS_NOK)
    {
        printf("compute_sha256 failed\n");
        free(hash_value);
        return STATUS_NOK;
    }
    
    header.version = current_version;
    header.signature_offset = sizeof(header);
    header.data_offset = header.signature_offset + SIGNATURE_PREFIX_LENGHT + SIGNATURE_SUFFIX_LENGTH + LONGEST_SIGNATURE;
    header.hash_offset = db_struct->shm_size + header.data_offset;
    header.hash_length = hash_data_size;
    //remvoe(db_struct->file_base_path);

    FILE *file = fopen(db_struct->file_base_path, "wb");
    if (file == NULL) {
        printf("Failed to open file");
        free(hash_value);
        return STATUS_NOK;
    }

    // 将二进制数据写入文件
    bytes_written = fwrite(&header, sizeof(unsigned char), sizeof(header), file);
    if (bytes_written != sizeof(header)) 
    {
        printf("Failed to write data to file");
        fclose(file);
        free(hash_value);
        remvoe(db_struct->file_base_path);
        return STATUS_NOK;
    }

    bytes_written = fwrite(db_struct->signature, sizeof(unsigned char), sizeof(db_struct->signature), file);
    if (bytes_written != sizeof(db_struct->signature))
    {
        printf("Failed to write data to file");
        fclose(file);
        free(hash_value);
        remvoe(db_struct->file_base_path);
        return STATUS_NOK;
    }

    bytes_written = fwrite(db_struct->db_content, sizeof(unsigned char), db_struct->shm_size, file);
    if (bytes_written != db_struct->shm_size) 
    {
        printf("Failed to write data to file");
        fclose(file);
        free(hash_value);
        remvoe(db_struct->file_base_path);
        return STATUS_NOK;
    }

    bytes_written = fwrite(hash_value, sizeof(unsigned char), hash_data_size, file);
    if (bytes_written != hash_data_size) 
    {
        printf("Failed to write data to file");
        fclose(file);
        free(hash_value);
        remvoe(db_struct->file_base_path);
        return STATUS_NOK;
    }

    // 关闭文件
    fclose(file);

    free(hash_value);
    return STATUS_OK;
}
