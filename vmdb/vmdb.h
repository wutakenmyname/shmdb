#ifndef _VMDB_H

#include "list.h"
#include "mbool.h"
#include "status.h"
#include "customization_vm.h"
#include "byte_types.h"


STATUS_T vmdb_init_once();
STATUS_T vmdb_get_data(data_type_t type, uint8 *value, int value_length);
STATUS_T vmdb_set_data(data_type_t type, uint8  *value, int value_length);
STATUS_T vmdb_get_data_at_offset(data_type_t type, uint8 *value, int value_length, uint16 offset);
STATUS_T vmdb_set_data_at_offset(data_type_t type, uint8  *value, int value_length, uint16 offset);
STATUS_T vmdb_get_data_ptr(data_type_t type, uint8 **value, int value_length);
void vmdb_deinit();
#endif