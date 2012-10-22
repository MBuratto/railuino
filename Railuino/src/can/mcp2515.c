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


#include <avr/io.h>
#include <util/delay.h>
#include "Arduino.h"
#include "global.h"
#include "mcp2515.h"
#include "mcp2515_defs.h"


#include "defaults.h"

// -------------------------------------------------------------------------
// Schreibt/liest ein Byte ueber den Hardware SPI Bus

uint8_t spi_putc( uint8_t data )
{
	// put byte in send-buffer
	SPDR = data;
	
	// wait until byte was send
	while( !( SPSR & (1<<SPIF) ) )
		;
	
	return SPDR;
}

// -------------------------------------------------------------------------
void can_write_register( uint8_t adress, uint8_t data )
{
	RESET(MCP2515_CS);
	
	spi_putc(SPI_WRITE);
	spi_putc(adress);
	spi_putc(data);
	
	SET(MCP2515_CS);
}

// -------------------------------------------------------------------------
uint8_t can_read_register(uint8_t adress)
{
	uint8_t data;
	
	RESET(MCP2515_CS);
	
	spi_putc(SPI_READ);
	spi_putc(adress);
	
	data = spi_putc(0xff);	
	
	SET(MCP2515_CS);
	
	return data;
}

// -------------------------------------------------------------------------
void can_bit_modify(uint8_t adress, uint8_t mask, uint8_t data)
{
	RESET(MCP2515_CS);
	
	spi_putc(SPI_BIT_MODIFY);
	spi_putc(adress);
	spi_putc(mask);
	spi_putc(data);
	
	SET(MCP2515_CS);
}

// ----------------------------------------------------------------------------
uint8_t can_read_status(uint8_t type)
{
	uint8_t data;
	
	RESET(MCP2515_CS);
	
	spi_putc(type);
	data = spi_putc(0xff);
	
	SET(MCP2515_CS);
	
	return data;
}

// -------------------------------------------------------------------------
uint8_t can_init(uint8_t speed, bool loopback)
{
		
	
	SET(MCP2515_CS);
	SET_OUTPUT(MCP2515_CS);
	
	RESET(P_SCK);
	RESET(P_MOSI);
	RESET(P_MISO);
	
	SET_OUTPUT(P_SCK);
	SET_OUTPUT(P_MOSI);
	SET_INPUT(P_MISO);
	
	SET_INPUT(MCP2515_INT);
	SET(MCP2515_INT);
	
	// active SPI master interface
	SPCR = (1<<SPE)|(1<<MSTR) | (0<<SPR1)|(1<<SPR0);
	SPSR = 0;
	
	// reset MCP2515 by software reset.
	// After this he is in configuration mode.
	RESET(MCP2515_CS);
	spi_putc(SPI_RESET);
	SET(MCP2515_CS);
	
	// wait a little bit until the MCP2515 has restarted
	_delay_us(10);
	
	// load CNF1..3 Register
	RESET(MCP2515_CS);
	spi_putc(SPI_WRITE);
	spi_putc(CNF3);
	
/*	spi_putc((1<<PHSEG21));		// Bitrate 125 kbps at 16 MHz
	spi_putc((1<<BTLMODE)|(1<<PHSEG11));
	spi_putc((1<<BRP2)|(1<<BRP1)|(1<<BRP0));
*/
/*	
	spi_putc((1<<PHSEG21));		// Bitrate 250 kbps at 16 MHz
	spi_putc((1<<BTLMODE)|(1<<PHSEG11));
	spi_putc((1<<BRP1)|(1<<BRP0));
*/	
	spi_putc((1<<PHSEG21));		// Bitrate 250 kbps at 16 MHz
	spi_putc((1<<BTLMODE)|(1<<PHSEG11));
	spi_putc((1<<BRP1)|(1<<BRP0));

//	spi_putc(0x03);
//	spi_putc(0xac);
//	spi_putc(0x81);

	// spi_putc(1<<BRP0);

//    spi_putc(speed);

	// activate interrupts
	spi_putc((1<<RX1IE)|(1<<RX0IE));
	SET(MCP2515_CS);
	
	// test if we could read back the value => is the chip accessible?
	if (can_read_register(CNF1) != 3) {

		Serial.println(can_read_register(CNF1), HEX);

		SET(LED2_HIGH);

		return false;
	}
	
	// deaktivate the RXnBF Pins (High Impedance State)
	can_write_register(BFPCTRL, 0);
	
	// set TXnRTS as inputs
	can_write_register(TXRTSCTRL, 0);
	
	// turn off filters => receive any message
	can_write_register(RXB0CTRL, (1<<RXM1)|(1<<RXM0));
	can_write_register(RXB1CTRL, (1<<RXM1)|(1<<RXM0));
	
	// reset device to normal mode
	can_write_register(CANCTRL, loopback ? 64 : 0);
//	SET(LED2_HIGH);



	return true;
}

// ----------------------------------------------------------------------------
// check if there are any new messages waiting

uint8_t can_check_message(void) {
	return (!IS_SET(MCP2515_INT));
}

// ----------------------------------------------------------------------------
// check if there is a free buffer to send messages

uint8_t can_check_free_buffer(void)
{
	uint8_t status = can_read_status(SPI_READ_STATUS);
	
	if ((status & 0x54) == 0x54) {
		// all buffers used
		return false;
	}
	
	return true;
}

// ----------------------------------------------------------------------------
uint8_t can_get_message(tCAN *message)
{
	// read status
	uint8_t status = can_read_status(SPI_RX_STATUS);
	uint8_t addr;
	uint8_t t;
	if (bit_is_set(status,6)) {
		// message in buffer 0
		addr = SPI_READ_RX;
	}
	else if (bit_is_set(status,7)) {
		// message in buffer 1
		addr = SPI_READ_RX | 0x04;
	}
	else {
		// Error: no message available
		return 0;
	}

	RESET(MCP2515_CS);
	spi_putc(addr);
	
    uint32_t id1 = spi_putc(0xff);
    uint32_t id2 = spi_putc(0xff);
    uint32_t id3 = spi_putc(0xff);
    uint32_t id4 = spi_putc(0xff);

    /*
    Serial.println("--- ID ---");
    Serial.println(id1, HEX);
    Serial.println(id2, HEX);
    Serial.println(id3, HEX);
    Serial.println(id4, HEX);
    */

    message->flags.extended = bit_is_set(id2, 3) ? 1 : 0;

	// read id
	message->id  = id1 << 21;
	message->id |= (((id2 & 0xE0) >> 3) | (id2 & 0x03)) << 16;
    message->id |= id3 << 8;
    message->id |= id4;

    // message->id = (id1 << 24) | (id2 << 16 | id3 << 8 | id4;


	// read DLC
	uint8_t length = spi_putc(0xff) & 0x0f;
	
	message->length = length;
	message->flags.rtr = (bit_is_set(status, 3)) ? 1 : 0;
	
	// read data
	for (t=0;t<length;t++) {
		message->data[t] = spi_putc(0xff);
	}
	SET(MCP2515_CS);
	
	// clear interrupt flag
	if (bit_is_set(status, 6)) {
		can_bit_modify(CANINTF, (1<<RX0IF), 0);
	}
	else {
		can_bit_modify(CANINTF, (1<<RX1IF), 0);
	}
	
	return (status & 0x07) + 1;
}

// ----------------------------------------------------------------------------
uint8_t can_send_message(tCAN *message)
{
	uint8_t status = can_read_status(SPI_READ_STATUS);
	
	/* Statusbyte:
	 *
	 * Bit	Function
	 *  2	TXB0CNTRL.TXREQ
	 *  4	TXB1CNTRL.TXREQ
	 *  6	TXB2CNTRL.TXREQ
	 */
	uint8_t address;
	uint8_t t;
//	SET(LED2_HIGH);
	if (bit_is_clear(status, 2)) {
		address = 0x00;
	}
	else if (bit_is_clear(status, 4)) {
		address = 0x02;
	} 
	else if (bit_is_clear(status, 6)) {
		address = 0x04;
	}
	else {
		// all buffer used => could not send message
		return 0;
	}
	
	RESET(MCP2515_CS);
	spi_putc(SPI_WRITE_TX | address);
	
        uint32_t id1 = message->id >> 21;

        uint32_t id2 = ((message->id >> 13) & 0xE0) | ((message->id >> 16) & 0x03);

        if (message->flags.extended) {
            id2 |= 0x08;
        }

        uint32_t id3 = message->id >> 8;
        uint32_t id4 = message->id;

	spi_putc(id1); // 3
    spi_putc(id2); // 5
	
	spi_putc(id3);
	spi_putc(id4);
	
	uint8_t length = message->length & 0x0f;
	
	if (message->flags.rtr) {
		// a rtr-frame has a length, but contains no data
		spi_putc((1<<RTR) | length);
	}
	else {
		// set message length
		spi_putc(length);
		
		// data
		for (t=0;t<length;t++) {
			spi_putc(message->data[t]);
		}
	}
	SET(MCP2515_CS);
	
	_delay_us(1);
	
	// send message
	RESET(MCP2515_CS);
	address = (address == 0) ? 1 : address;
	spi_putc(SPI_RTS | address);
	SET(MCP2515_CS);
	
	return address;
}
