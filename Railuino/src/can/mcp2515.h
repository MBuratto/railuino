#ifndef	MCP2515_H
#define	MCP2515_H

// ----------------------------------------------------------------------------
/* Copyright (c) 2007 Fabian Greif
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
// ----------------------------------------------------------------------------

#include <inttypes.h>

#include "mcp2515_defs.h"
#include "global.h"
#ifdef __cplusplus

extern "C"
{
	

#endif
// ----------------------------------------------------------------------------
typedef struct
{
	uint32_t id;				//!< ID der Nachricht (11 oder 29 Bit)
	struct {
		int rtr : 1;			//!< Remote-Transmit-Request-Frame?
		int extended : 1;		//!< extended ID?
	} flags;
    uint8_t length;				//!< Anzahl der Datenbytes
    uint8_t data[8];			//!< Die Daten der CAN Nachricht
} tCAN;

typedef tCAN can_t;

// ----------------------------------------------------------------------------
uint8_t spi_putc( uint8_t data );

// ----------------------------------------------------------------------------
void can_write_register( uint8_t adress, uint8_t data );

// ----------------------------------------------------------------------------
uint8_t can_read_register(uint8_t adress);

// ----------------------------------------------------------------------------
void can_bit_modify(uint8_t adress, uint8_t mask, uint8_t data);

// ----------------------------------------------------------------------------
uint8_t can_read_status(uint8_t type);

// ----------------------------------------------------------------------------

uint8_t can_init(uint8_t speed);

// ----------------------------------------------------------------------------
// check if there are any new messages waiting
uint8_t can_check_message(void);

// ----------------------------------------------------------------------------
// check if there is a free buffer to send messages
uint8_t can_check_free_buffer(void);

// ----------------------------------------------------------------------------
uint8_t can_get_message(tCAN *message);

// ----------------------------------------------------------------------------
uint8_t can_send_message(tCAN *message);


#ifdef __cplusplus
}
#endif

#endif	// MCP2515_H
