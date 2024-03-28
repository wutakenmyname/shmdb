#ifndef _CUSTOMIZATION_H
#define _CUSTOMIZATION_H

#include "byte_types.h"

typedef enum
{
    DB_FIRST_MEMBER = 0,
    DB_UINT8_MEMBER = 0,
    DB_UINT16_MEMBER,
    DB_UINT32_MEMBER,
    DB_STRUCT_MEMBER,
    DB_UINT64_MEMBER,
    DB_FLOAT_MEMBER,
    DB_DOUBLE_MEMBER,
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
}customization_t;

#endif