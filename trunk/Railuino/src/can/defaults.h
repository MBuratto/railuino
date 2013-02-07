#ifndef	DEFAULTS_H
#define	DEFAULTS_H

#if defined(__UNO__)

#define	P_MOSI	B,3
#define	P_MISO	B,4
#define	P_SCK	B,5

//#define	MCP2515_CS			D,3	// Rev A
#define	MCP2515_CS			B,2 // was 2 Rev B
#define	MCP2515_INT			D,2 // was 2
#define LED2_HIGH			B,0
#define LED2_LOW			B,0

#define CAN_INT  0

#elif defined(__LEONARDO__)

#define	P_MOSI	B,2
#define	P_MISO	B,3
#define	P_SCK	B,1

#define	MCP2515_CS			B,6 // was 2 Rev B
#define	MCP2515_INT			D,1 // was 2
#define LED2_HIGH			B,0
#define LED2_LOW			B,0

#define CAN_INT  1

#endif

#endif	// DEFAULTS_H
