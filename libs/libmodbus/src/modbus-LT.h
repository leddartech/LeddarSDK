
#ifndef _MODBUS_LT_H_
#define _MODBUS_LT_H_

#include "modbus.h"

#ifdef  __cplusplus
# define MODBUS_LT_BEGIN_DECLS  extern "C" {
# define MODBUS_LT_END_DECLS    }
#else
# define MODBUS_LT_BEGIN_DECLS
# define MODBUS_LT_END_DECLS
#endif

MODBUS_LT_BEGIN_DECLS


int modbus_receive_raw_confirmation_timeoutEnd( modbus_t *ctx, uint8_t *rsp );
int modbus_receive_raw_confirmation_sizeEnd( modbus_t *ctx, uint8_t *rsp, int length );
int modbus_receive_raw_confirmation_0x41_LeddarVu( modbus_t *ctx, uint8_t *rsp );
int modbus_receive_raw_confirmation_0x41_0x6A_M16( modbus_t *ctx, uint8_t *rsp );
int modbus_receive_raw_data_timeoutEnd( modbus_t *ctx, uint8_t *rsp );
int modbus_send_raw_data( modbus_t *ctx, uint8_t *data, int length, int crc);


MODBUS_LT_END_DECLS

#endif  /* _MODBUS_LT_H_ */
