/*
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 *
 * This library implements the Modbus protocol.
 * http://libmodbus.org/
 */
#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS






/* ==============================================================================================

Workaround fix to solve a Modbus user command function not managed by the
"compute_data_length_after_meta" function.

Instead use "modbus_receive_confirmation" function to get answer from a "modbus_send_raw_request"
call, use the function "modbus_receive_raw_confirmation_timeoutEnd". Beware: adjust response and byte timeout
to avoid extra lag...

See thread: https://github.com/stephane/libmodbus/issues/343

A function version based on known length to receive, use the function "modbus_receive_raw_confirmation_sizeEnd".

For the "Get Detection" command (Modbus 0x41), use "modbus_receive_raw_confirmation_0x41_LeddarVu" or
"modbus_receive_raw_confirmation_0x41_0x6A_M16" to get confirmation.

New alternative functions added:
-> modbus_receive_raw_confirmation_timeoutEnd
-> modbus_receive_raw_confirmation_sizeEnd
-> modbus_receive_raw_confirmation_0x41_LeddarVu
-> modbus_receive_raw_confirmation_0x41_0x6A_M16

Not recommended function:
-> modbus_receive_confirmation

============================================================================================== */




#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

//#include <config.h>

#include "modbus-LT.h"
#include "modbus-private.h"

/* Internal use */
#define MSG_LENGTH_UNDEFINED -1

/* Max between RTU and TCP max adu length (so TCP) */
#define MAX_MESSAGE_LENGTH 260

/* 3 steps are used to parse the query */
typedef enum
{
    _STEP_FUNCTION,
    _STEP_META,
    _STEP_DATA
} _step_t;



/*
---------- Request     Indication ----------
| Client | ---------------------->| Server |
---------- Confirmation  Response ----------
*/

extern void _error_print( modbus_t *ctx, const char *context );
#if (LIBMODBUS_VERSION_MAJOR >= 3 && LIBMODBUS_VERSION_MINOR >=1 && LIBMODBUS_VERSION_MICRO >= 4)
//copied from modbus.c file
static void _sleep_response_timeout( modbus_t *ctx )
{
    /* Response timeout is always positive */
#ifdef _WIN32
    /* usleep doesn't exist on Windows */
    Sleep( ( ctx->response_timeout.tv_sec * 1000 ) +
           ( ctx->response_timeout.tv_usec / 1000 ) );
#else
    /* usleep source code */
    struct timespec request, remaining;
    request.tv_sec = ctx->response_timeout.tv_sec;
    request.tv_nsec = ( ( long int )ctx->response_timeout.tv_usec ) * 1000;

    while( nanosleep( &request, &remaining ) == -1 && errno == EINTR )
    {
        request = remaining;
    }

#endif
}
#else
extern int _sleep_and_flush( modbus_t *ctx );
#endif

static uint8_t compute_meta_length_after_function_LT( int function, msg_type_t msg_type );



/* Waits a raw response from a modbus server or a request from a modbus client.
This function blocks if there is no replies (3 timeouts).

The function shall return the number of received characters and the received
message in an array of uint8_t if successful. Otherwise it shall return -1
and errno is set to one of the values defined below:
- ECONNRESET
- EMBBADDATA
- EMBUNKEXC
- ETIMEDOUT
- read() or recv() error codes
*/
int modbus_receive_raw_msg_LT( modbus_t *ctx, uint8_t *msg, msg_type_t msg_type )
{
    int rc = 0;
    fd_set rset;
    struct timeval tv;
    struct timeval *p_tv;
    int length_to_read;
    int msg_length = 0;
    _step_t step = _STEP_FUNCTION;

    if( ctx->debug )
    {
        if( msg_type == MSG_INDICATION )
        {
            printf( "Waiting for a raw indication...\n" );
        }
        else
        {
            printf( "Waiting for a raw confirmation...\n" );
        }
    }

    /* Add a file descriptor to the set */
    FD_ZERO( &rset );
    FD_SET( ctx->s, &rset );

    length_to_read = ctx->backend->header_length + 1;

    if( msg_type == MSG_INDICATION )
    {
        /* Wait for a message, we don't know when the message will be
        * received */
        p_tv = NULL;
    }
    else
    {
        tv.tv_sec = ctx->response_timeout.tv_sec;
        tv.tv_usec = ctx->response_timeout.tv_usec;
        p_tv = &tv;
    }

    while( rc != -1 )
    {
        rc = ctx->backend->select( ctx, &rset, p_tv, length_to_read );

        if( rc == -1 )
        {
            /* Timeout at end of msg */
            if( step == _STEP_DATA && errno == ETIMEDOUT )
            {
                break;
            }

            _error_print( ctx, "select" );

            if( ctx->error_recovery & MODBUS_ERROR_RECOVERY_LINK )
            {
                int saved_errno = errno;

                if( errno == ETIMEDOUT )
                {
#if LIBMODBUS_VERSION_MAJOR >= 3 && LIBMODBUS_VERSION_MINOR >=1 && LIBMODBUS_VERSION_MICRO >= 4
                    // use in libmodbus ver. 3.1.4, not in 3.0.6 ......
                    _sleep_response_timeout( ctx );
                    modbus_flush( ctx );
#else
                    // ...instead, use this for 3.0.6
                    _sleep_and_flush( ctx );
#endif
                }
                else if( errno == EBADF )
                {
                    modbus_close( ctx );
                    modbus_connect( ctx );
                }

                errno = saved_errno;
            }

            return -1;
        }

        rc = ctx->backend->recv( ctx, msg + msg_length, length_to_read );

        if( rc == 0 )
        {
            errno = ECONNRESET;
            rc = -1;
        }

        if( rc == -1 )
        {
            _error_print( ctx, "read" );

            if( ( ctx->error_recovery & MODBUS_ERROR_RECOVERY_LINK ) &&
                    ( errno == ECONNRESET || errno == ECONNREFUSED ||
                      errno == EBADF ) )
            {
                int saved_errno = errno;
                modbus_close( ctx );
                modbus_connect( ctx );
                /* Could be removed by previous calls */
                errno = saved_errno;
            }

            return -1;
        }

        /* Display the hex code of each character received */
        if( ctx->debug )
        {
            int i;

            for( i = 0; i < rc; i++ )
                printf( "<%.2X>", msg[ msg_length + i ] );
        }

        /* Sums bytes received */
        msg_length += rc;

        if( step == _STEP_FUNCTION )
        {
            /* Set byte timeout */
            if( ctx->byte_timeout.tv_sec > 0 || ctx->byte_timeout.tv_usec > 0 )
            {
                tv.tv_sec = ctx->byte_timeout.tv_sec;
                tv.tv_usec = ctx->byte_timeout.tv_usec;
                p_tv = &tv;
            }

            /* Read one byte */
            length_to_read = 1;
            /* Data step */
            step = _STEP_DATA;
        }
    }

    if( ctx->debug )
        printf( "\n" );

    return ctx->backend->check_integrity( ctx, msg, msg_length );
}

/* Receives the raw confirmation. A timeout event determine the end of Modbus transaction.

The function shall store the read response in rsp and return the number of
values (bits or words). Otherwise, its shall return -1 and errno is set.

The function doesn't check the confirmation is the expected response to the
initial request.
*/
int modbus_receive_raw_confirmation_timeoutEnd( modbus_t *ctx, uint8_t *rsp )
{
    if( ctx == NULL )
    {
        errno = EINVAL;
        return -1;
    }

    return modbus_receive_raw_msg_LT( ctx, rsp, MSG_CONFIRMATION );
}


/* Computes the length to read after the function received */
static uint8_t compute_meta_length_after_function_LT( int function,
        msg_type_t msg_type )
{
    int length;

    if( msg_type == MSG_INDICATION )
    {
        if( function <= MODBUS_FC_WRITE_SINGLE_REGISTER )
        {
            length = 4;
        }
        else if( function == MODBUS_FC_WRITE_MULTIPLE_COILS ||
                 function == MODBUS_FC_WRITE_MULTIPLE_REGISTERS )
        {
            length = 5;
        }
        else if( function == MODBUS_FC_WRITE_AND_READ_REGISTERS )
        {
            length = 9;
        }
        else
        {
            /* _FC_READ_EXCEPTION_STATUS, _FC_REPORT_SLAVE_ID */
            length = 0;
        }
    }
    else
    {
        /* MSG_CONFIRMATION */
        switch( function )
        {
            case MODBUS_FC_WRITE_SINGLE_COIL:
            case MODBUS_FC_WRITE_SINGLE_REGISTER:
            case MODBUS_FC_WRITE_MULTIPLE_COILS:
            case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
                length = 4;
                break;

            default:
                length = 1;
        }
    }

    return length;
}


/* Computes the length to read after the meta information (address, count, etc) */
static int compute_data_length_after_meta_LT( modbus_t *ctx, uint8_t *msg,
        msg_type_t msg_type, int alength )
{
    int function = msg[ctx->backend->header_length];
    int length;

    if( msg_type == MSG_INDICATION )
    {
        switch( function )
        {
            case MODBUS_FC_WRITE_MULTIPLE_COILS:
            case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
                length = msg[ctx->backend->header_length + 5];
                break;

            case MODBUS_FC_WRITE_AND_READ_REGISTERS:
                length = msg[ctx->backend->header_length + 9];
                break;

            default:
                length = 0;
        }
    }
    else
    {
        /* MSG_CONFIRMATION */
        if( function <= MODBUS_FC_READ_INPUT_REGISTERS ||
                function == MODBUS_FC_REPORT_SLAVE_ID ||
                function == MODBUS_FC_WRITE_AND_READ_REGISTERS )
        {
            length = msg[ctx->backend->header_length + 1];
        }
        else if( ( msg_type == MSG_CONFIRMATION_0x41_LEDDARVU ) && ( function == 0x41 ) )
        {
            // Modification to support the custom Leddar 0x41 command on the LeddarVu:
            length = ( 6 * msg[ctx->backend->header_length + 1] ) + 7;
        }
        else if( ( msg_type == MSG_CONFIRMATION_0x41_0x6A_M16 ) && ( function == 0x6A ) )
        {
            // Modification to support the custom Leddar 0x6A command on the Evalkit/IS16/M16:
            length = ( 6 * msg[ctx->backend->header_length + 1] ) + 6;
        }
        else if( ( msg_type == MSG_CONFIRMATION_0x41_0x6A_M16 ) && ( function == 0x41 ) )
        {
            // Modification to support the custom Leddar 0x41 command on the Evalkit/IS16/M16:
            length = ( 5 * msg[ctx->backend->header_length + 1] ) + 6;
        }
        else
        {
            length = alength;
        }
    }

    length += ctx->backend->checksum_length;

    return length;
}


/* Waits a response from a modbus server or a request from a modbus client.
This function blocks if there is no replies (3 timeouts).

The function shall return the number of received characters and the received
message in an array of uint8_t if successful. Otherwise it shall return -1
and errno is set to one of the values defined below:
- ECONNRESET
- EMBBADDATA
- EMBUNKEXC
- ETIMEDOUT
- read() or recv() error codes
*/

static int receive_msg_LT( modbus_t *ctx, uint8_t *msg, msg_type_t msg_type, int length )
{
    int rc;
    fd_set rfds;
    struct timeval tv;
    struct timeval *p_tv;
    int length_to_read;
    int msg_length = 0;
    _step_t step;

    if( ctx->debug )
    {
        if( msg_type == MSG_INDICATION )
        {
            printf( "Waiting for a indication...\n" );
        }
        else
        {
            printf( "Waiting for a confirmation...\n" );
        }
    }

    /* Add a file descriptor to the set */
    FD_ZERO( &rfds );
    FD_SET( ctx->s, &rfds );

    /* We need to analyse the message step by step.  At the first step, we want
    * to reach the function code because all packets contain this
    * information. */
    step = _STEP_FUNCTION;
    length_to_read = ctx->backend->header_length + 1;

    if( msg_type == MSG_INDICATION )
    {
        /* Wait for a message, we don't know when the message will be
        * received */
        p_tv = NULL;
    }
    else
    {
        tv.tv_sec = ctx->response_timeout.tv_sec;
        tv.tv_usec = ctx->response_timeout.tv_usec;
        p_tv = &tv;
    }

    while( length_to_read != 0 )
    {
        rc = ctx->backend->select( ctx, &rfds, p_tv, length_to_read );

        if( rc == -1 )
        {
            _error_print( ctx, "select" );

            if( ctx->error_recovery & MODBUS_ERROR_RECOVERY_LINK )
            {
                int saved_errno = errno;

                if( errno == ETIMEDOUT )
                {
#if LIBMODBUS_VERSION_MAJOR >= 3 && LIBMODBUS_VERSION_MINOR >=1 && LIBMODBUS_VERSION_MICRO >= 4
                    _sleep_response_timeout( ctx );
                    modbus_flush( ctx );
#else
                    _sleep_and_flush( ctx );
#endif
                }
                else if( errno == EBADF )
                {
                    modbus_close( ctx );
                    modbus_connect( ctx );
                }

                errno = saved_errno;
            }

            return -1;
        }

        rc = ctx->backend->recv( ctx, msg + msg_length, length_to_read );

        if( rc == 0 )
        {
            errno = ECONNRESET;
            rc = -1;
        }

        if( rc == -1 )
        {
            _error_print( ctx, "read" );

            if( ( ctx->error_recovery & MODBUS_ERROR_RECOVERY_LINK ) &&
                    ( errno == ECONNRESET || errno == ECONNREFUSED ||
                      errno == EBADF ) )
            {
                int saved_errno = errno;
                modbus_close( ctx );
                modbus_connect( ctx );
                /* Could be removed by previous calls */
                errno = saved_errno;
            }

            return -1;
        }

        /* Display the hex code of each character received */
        if( ctx->debug )
        {
            int i;

            for( i = 0; i < rc; i++ )
                printf( "<%.2X>", msg[msg_length + i] );
        }

        /* Sums bytes received */
        msg_length += rc;
        /* Computes remaining bytes */
        length_to_read -= rc;

        if( length_to_read == 0 )
        {
            switch( step )
            {
                case _STEP_FUNCTION:
                    /* Function code position */
                    length_to_read = compute_meta_length_after_function_LT(
                                         msg[ctx->backend->header_length],
                                         msg_type );

                    if( length_to_read != 0 )
                    {
                        step = _STEP_META;
                        break;
                    } /* else switches straight to the next step */

                case _STEP_META:
                    length_to_read = compute_data_length_after_meta_LT(
                                         ctx, msg, msg_type, length );

                    if( ( msg_length + length_to_read ) > ( signed )ctx->backend->max_adu_length )
                    {
                        errno = EMBBADDATA;
                        _error_print( ctx, "too many data" );
                        return -1;
                    }

                    step = _STEP_DATA;
                    break;

                default:
                    break;
            }
        }

        if( length_to_read > 0 && ctx->byte_timeout.tv_sec != -1 )
        {
            /* If there is no character in the buffer, the allowed timeout
            interval between two consecutive bytes is defined by
            byte_timeout */
            tv.tv_sec = ctx->byte_timeout.tv_sec;
            tv.tv_usec = ctx->byte_timeout.tv_usec;
            p_tv = &tv;
        }
    }

    if( ctx->debug )
        printf( "\n" );

    return ctx->backend->check_integrity( ctx, msg, msg_length );
}


#define MODBUS_HEADER_SIZE      2       // Address + function code size (1+1 bytes)
#define MODBUS_CRC_SIZE         2       // Modbus crc size (2 bytes)
#define MODBUS_PAYLOAD          (MODBUS_HEADER_SIZE + MODBUS_CRC_SIZE + 1)

/* Receives the raw confirmation. Receive the corresponding length determine the end of Modbus transaction.

The function shall store the read response in rsp and return the number of
values (bits or words). Otherwise, its shall return -1 and errno is set.

The function doesn't check the confirmation is the expected response to the
initial request.
*/
int modbus_receive_raw_confirmation_sizeEnd( modbus_t *ctx, uint8_t *rsp, int length )
{
    if( ctx == NULL )
    {
        errno = EINVAL;
        return -1;
    }

    return receive_msg_LT( ctx, rsp, MSG_CONFIRMATION, ( length > MODBUS_PAYLOAD ) ? length - MODBUS_PAYLOAD : 0 );
}

/* Receives the confirmation from the custom command 0x41 send to a LeddarVu product.

The function shall store the read response in rsp and return the number of
values (bits or words). Otherwise, its shall return -1 and errno is set.

The function doesn't check the confirmation is the expected response to the
initial request.
*/
int modbus_receive_raw_confirmation_0x41_LeddarVu( modbus_t *ctx, uint8_t *rsp )
{
    if( ctx == NULL )
    {
        errno = EINVAL;
        return -1;
    }

    return receive_msg_LT( ctx, rsp, MSG_CONFIRMATION_0x41_LEDDARVU, 0 );
}

/* Receives the confirmation from the custom command 0x41 sent to an evalkit/IS16/M16 product.

The function shall store the read response in rsp and return the number of
values (bits or words). Otherwise, its shall return -1 and errno is set.

The function doesn't check the confirmation is the expected response to the
initial request.
*/
int modbus_receive_raw_confirmation_0x41_0x6A_M16( modbus_t *ctx, uint8_t *rsp )
{
    if( ctx == NULL )
    {
        errno = EINVAL;
        return -1;
    }

    return receive_msg_LT( ctx, rsp, MSG_CONFIRMATION_0x41_0x6A_M16, 0 );
}

