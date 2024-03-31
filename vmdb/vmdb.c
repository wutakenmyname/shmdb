#include "vmdb.h"
#include "db_base.h"
#include <sys/types.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define VMDB_BASE_FILE_PATH "/root/vmdb_file"
#define VMDB_RANDON_ID 0x1234
#define VMDB_SEM_FILE_PATH "/root/vmdb_sem"
#define VMDB_SEM_RANDON_ID 0x4321
#define mprintf(fmt, ...) printf("[%s,%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)

typedef struct
{
    uint32 offset;
    uint16 size;
}data_info_t;

union semun {
    int val;                // 用于 SETVAL 操作
    struct semid_ds *buf;   // 用于 IPC_STAT 和 IPC_SET 操作
    unsigned short *array;  // 用于 GETALL 和 SETALL 操作
};

key_t vmdb_key;
key_t sem_key;
base_db_t base_db;
customization_t *global_data = NULL;
unsigned char *generated_data = NULL;
int generated_data_length = -1;
int vmdb_sem_id = -1;

#define GET_OFFSET(mstruct, member) ((size_t) &(((mstruct *)0)->member))
#define GET_SIZE(mstruct, member) sizeof(((mstruct *)0)->member)
#define GET_DATA_INFO(customization_t, member) {GET_OFFSET(customization_t, member), GET_SIZE(customization_t, member)}

data_info_t vmdb_info[] =
{
    GET_DATA_INFO(customization_t, i8_member),
    GET_DATA_INFO(customization_t, i16_member),
    GET_DATA_INFO(customization_t, i32_member),
    GET_DATA_INFO(customization_t, struct_member),
    GET_DATA_INFO(customization_t, i64_member),
    GET_DATA_INFO(customization_t, f32_member),
    GET_DATA_INFO(customization_t, d64_member),
    GET_DATA_INFO(customization_t, string66),
};

uint16 get_data_size(data_type_t type)
{
    mprintf("type: %d, size: %d\n", type, vmdb_info[type].size);
    return vmdb_info[type].size;
}

uint32 get_data_offset(data_type_t type)
{
    return vmdb_info[type].offset;
}

key_t generate_vmdb_shm_key()
{
    return ftok(VMDB_BASE_FILE_PATH, VMDB_RANDON_ID);
}

key_t generate_vmdb_sem_key()
{
    return ftok(VMDB_SEM_FILE_PATH,VMDB_SEM_RANDON_ID);
}

void restore_data_from_file(char *file_data, int file_data_len)
{
    mprintf("file_data_len %d\n", file_data_len);

    if (file_data == NULL || file_data_len <= 0)
    {
        mprintf("params not valid\n");
        return;
    }
    return;

    // to do, parse data
}

STATUS_T vmdb_lock()
{
    struct sembuf sem_buf;
    sem_buf.sem_num = 0;
    sem_buf.sem_op = -1;
    sem_buf.sem_flg = SEM_UNDO;//系统退出前未释放信号量，系统自动释放
    if (semop(vmdb_sem_id, &sem_buf, 1) == -1) {
        mprintf("lock vmdb sem failed\n");
        return STATUS_NOK;
    }
    return STATUS_OK;
}

STATUS_T vmdb_unlock()
{
    struct sembuf sem_buf;
    sem_buf.sem_num = 0;
    sem_buf.sem_op = 1;
    sem_buf.sem_flg = SEM_UNDO;//系统退出前未释放信号量，系统自动释放
    if (semop(vmdb_sem_id, &sem_buf, 1) == -1) {
        mprintf("unlock vmdb sem failed\n");
        return STATUS_NOK;
    }
    return STATUS_OK;
}

static STATUS_T vm_lock_init()
{
    vmdb_sem_id = semget(sem_key, 1, IPC_CREAT | 0666);
    if (vmdb_sem_id < 0)
    {
        mprintf("semget failed, return value :%d\n", vmdb_sem_id);
        return STATUS_NOK;
    }

    union semun val;
    val.val = 1;
    if (semctl(vmdb_sem_id, 0, SETVAL, val) < 0)
    {
        mprintf("semctl failed\n");
        return STATUS_NOK;
    }

    mprintf("going to get vm lock ready\n");
    vmdb_lock();
    vmdb_unlock();
    mprintf("vm lock is ready\n");
    return STATUS_OK;
}

STATUS_T vmdb_init_once()
{
    STATUS_T ret = STATUS_OK;
    vmdb_key = generate_vmdb_shm_key();
    sem_key = generate_vmdb_sem_key();

    ret = vm_lock_init();
    if (ret != STATUS_OK)
    {
        mprintf("vm_lock_init failed\n");
        return STATUS_NOK;
    }

    printf("vmdb_key :%d\n", vmdb_key);
    mprintf("sizeof(customization_t): %d\n", sizeof(customization_t));

    if (vmdb_key < 0)
    {
        mprintf("generate_vmdb_shm_key failed\n");
        return STATUS_NOK;
    }
    ret = db_get(vmdb_key, &base_db, sizeof(customization_t), DB_HASH_METHOD_SHA256);
    if (ret == STATUS_NOK)
    {
        mprintf("db_get failed\n");
        return STATUS_NOK;
    }

    global_data = db_retrieve_access(base_db);
    if (global_data == NULL)
    {
        mprintf("db_retrieve_access failed\n");
        db_put(base_db);
        return STATUS_NOK;
    }

    global_data->i8_member = 99;
    global_data->i16_member = -66;
    global_data->i32_member = 12343;
    global_data->i64_member = -1234563;
    global_data->struct_member.age = 26;
    memset(global_data->struct_member.name, 0, sizeof(global_data->struct_member.name));
    memcpy(global_data->struct_member.name, "mok", strlen("mok") + 1);
    global_data->struct_member.height = 11;
    global_data->struct_member.weight = 22;
    global_data->f32_member = 1.8537f;
    global_data->d64_member = 7.77777;
    memset(global_data->string66, 0, sizeof(global_data->string66));
    memcpy(global_data->string66, "this is mok", strlen("this is mok"));

    return STATUS_OK;
}

STATUS_T vmdb_get_data(data_type_t type, uint8 *value, int value_length)
{
    if (value == NULL)
    {
        mprintf("value is null\n");
        return STATUS_NOK;
    }

    if (type < DB_FIRST_MEMBER || type >= DB_TOO_BIG_MEMBER || get_data_size(type) != value_length)
    {
        mprintf("type %d not valid or length %d is wrong \n", type, value_length);
        return STATUS_NOK;
    }

    mprintf("type: %d, length: %d, offset:%d, addr: %p\n", type, value_length, get_data_offset(type), (unsigned char *)global_data + get_data_offset(type));
    vmdb_lock();
    memcpy(value, (unsigned char *)global_data + get_data_offset(type), value_length);
    vmdb_unlock();
    return STATUS_OK;
}

STATUS_T vmdb_set_data(data_type_t type, uint8 *value, int value_length)
{
    if (value == NULL)
    {
        mprintf("value is null\n");
        return STATUS_NOK;
    }

    if (type < DB_FIRST_MEMBER || type >= DB_TOO_BIG_MEMBER || get_data_size(type) != value_length)
    {
        mprintf("type %d not valid or length %d is wrong \n", type, value_length);
        return STATUS_NOK;
    }

    vmdb_lock();
    memcpy((unsigned char *)global_data + get_data_offset(type), value, value_length);
    vmdb_unlock();
    return STATUS_OK;
}

STATUS_T vmdb_get_data_at_offset(data_type_t type, uint8 *value, int value_length, uint16 offset)
{
    if (value == NULL)
    {
        mprintf("value is null\n");
        return STATUS_NOK;
    }

    if (type < DB_FIRST_MEMBER || type >= DB_TOO_BIG_MEMBER || (get_data_size(type) - offset) != value_length)
    {
        mprintf("type %d not valid or length %d is wrong or offset %hu is wrong\n", type, value_length, offset);
        return STATUS_NOK;
    }

    vmdb_lock();
    memcpy(value, (unsigned char *)global_data + get_data_offset(type) + offset, value_length);
    vmdb_unlock();
    return STATUS_OK;
}

STATUS_T vmdb_set_data_at_offset(data_type_t type, uint8  *value, int value_length, uint16 offset)
{
    if (value == NULL)
    {
        mprintf("value is null\n");
        return STATUS_NOK;
    }

    if (type < DB_FIRST_MEMBER || type >= DB_TOO_BIG_MEMBER || (get_data_size(type) - offset) != value_length)
    {
        mprintf("type %d not valid or length %d is wrong or offset %hu is wrong\n", type, value_length, offset);
        return STATUS_NOK;
    }

    vmdb_lock();
    memcpy((unsigned char *)global_data + get_data_offset(type) + offset, value, value_length);
    vmdb_unlock();
    return STATUS_OK;
}

STATUS_T vmdb_get_data_ptr(data_type_t type, uint8 **value, int value_length)
{
    if (value == NULL)
    {
        printf("value is null \n");
        return STATUS_NOK;
    }

    if (type < DB_FIRST_MEMBER || type >= DB_TOO_BIG_MEMBER || get_data_size(type) != value_length)
    {
        mprintf("type %d not valid or length %d is wrong \n", type, value_length);
        return STATUS_NOK;
    }

    *value = (unsigned char *)global_data + get_data_offset(type);
    return STATUS_OK;
}

void vmdb_deinit()
{
    return;
}
