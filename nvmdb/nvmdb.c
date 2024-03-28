#include "nvmdb.h"
#include "customization.h"
#include "db_base.h"

#define NVMDB_BASE_FILE_PATH “/root/nvmdb"
#define NVMDB_RANDON_ID 0x1234
#define mprintf(fmt, ...) printf("[%s,%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)

typedef struct
{
    uint32 offset;
    uint16 size;
}data_info_t;

key_t nvmdb_key;
base_db_t base_db;
customization_t *global_data = NULL;
unsigned char *generated_data = NULL;
int generated_data_length = -1;

#define GET_OFFSET(mstruct, member) ((mstruct *)0)->member
#define GET_SIZE(mstruct, member) sizeof(((mstruct *)0)->member)
#define GET_DATA_INFO(customization_t, member) {GET_OFFSET(customization_t, member), GET_SIZE(customization_t, member)}

data_info_t nvmdb_info[] =
{
    GET_DATA_INFO(customization_t, i8_member),
    GET_DATA_INFO(customization_t, i16_member),
    GET_DATA_INFO(customization_t, i32_member),
    GET_DATA_INFO(customization_t, struct_member),
    GET_DATA_INFO(customization_t, i64_member),
    GET_DATA_INFO(customization_t, f32_member),
    GET_DATA_INFO(customization_t, d32_member),
}

key_t generate_nvmdb_shm_key() {
    return ftok(NVMDB_BASE_FILE_PATH, NVMDB_RANDON_ID);
}

void restore_data_from_file(char *file_data, int file_data_len)
{
    mprintf("file_data_len %d\n", file_data_len);

    if (file_data == NULL || file_data_len <= 0)
    {
        mprintf("params not valid\n");
        return;
    }

    // to do, parse data
}



STATUS_T nvmdb_init_once()
{
    STATUS_T ret = STATUS_OK;
    nvmdb_key = generate_nvmdb_shm_key();

    ret = db_get(nvmdb_key, &base_db, sizeof(customization_t), DB_HASH_METHOD_SHA256);
    if (ret == STATUS_NOK)
    {
        mprintf("db_get failed\n");
        return STATUS_NOK;
    }

    ret = db_set_file_base(base_db, NVMDB_BASE_FILE_PATH, strlen(NVMDB_BASE_FILE_PATH));
    if (ret == STATUS_NOK)
    {
        mprintf("db_set_file_base failed\n");
        db_put(base_db);
        return STATUS_NOK;
    }

    global_data = db_retrieve_access(base_db);
    if (global_data == NULL)
    {
        mprintf("db_retrieve_access failed\n");
        db_put(base_db);
        return STATUS_NOK;
    }

    if (old_file_base_exist(base_db) != false)
    {
        unsigned char *file_data = NULL;
        int file_data_len = -1;
        if(db_get_old_file_base_content(base_db, &file_data, &file_data_len) == STATUS_OK)
        {
            restore_data_from_file(file_data, file_data_len);
            db_release_old_file_base_content(file_data);
        }  
    }

    global_data->i8_member = 99;
    global_data->i16_member = 66;
    global_data->i32_member = 12343;
    global_data->i64_member = 1234563;
    global_data->struct_member.age = 26;
    memcpy(global_data->struct_member.name, "mok", strlen("mok"));
    global_data->struct_member.height = 11;
    global_data->struct_member.weight = 22;
    global_data->f32_member = 1.8537;
    global_data->d64_member = 7.77777;

}

STATUS_T nvmdb_commit()
