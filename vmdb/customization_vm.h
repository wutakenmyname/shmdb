#ifndef _CUSTOMIZATION_H
#define _CUSTOMIZATION_H

#include "byte_types.h"

typedef enum
{
    DB_FIRST_MEMBER = 0,
    DB_UINT8_MEMBER = 0,
    DB_UINT16_MEMBER = 1,
    DB_UINT32_MEMBER = 2,
    DB_STRUCT_MEMBER = 3,
    DB_UINT64_MEMBER = 4,
    DB_FLOAT_MEMBER = 5,
    DB_DOUBLE_MEMBER = 6,
    DB_STRING_MEMBER = 7,
    DB_TOO_BIG_MEMBER
}data_type_t;

typedef struct
{
    char name[64];
    uint32 age;
    int gender;
    float height;
    float weight;
}person_info __attribute__((aligned));

typedef struct
{
    uint8 i8_member;
    uint16 i16_member;
    uint32 i32_member;
    person_info struct_member;
    uint64 i64_member;
    float f32_member;
    double d64_member;
    char string66[66];
}customization_t;

#endif