/*
	Copyright (c) 2006 Wiimote Data Management Library

	Permission is hereby granted, free of charge, to any person obtaining a copy of this
	software and associated documentation files (the "Software"), to deal in the Software
	without restriction, including without limitation the rights to use, copy, modify,
	merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
	permit persons to whom the Software is furnished to do so, subject to the following
	conditions:

	The above copyright notice and this permission notice shall be included in all copies or
	substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
	LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
	OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#ifndef _DEFINES_H_
#define _DEFINES_H_

#define WDML_VERSION_STRING						"0.9.2"

//-----------------------------------------------------------------------------
// Type Definitions
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

//-----------------------------------------------------------------------------
// HID General Defines
#define WIIMOTE_HID_VENDOR_ID					0x057E
#define WIIMOTE_HID_PRODUCT_ID					0x0306
#define WIIMOTE_HID_REPORT_LENGTH				22
#define WIIMOTE_HID_REPORT_PAYLOAD_LENGTH		WIIMOTE_HID_REPORT_LENGTH - 1

#define WIIMOTE_HID_WRITE_DATA_SIZE				16

//-----------------------------------------------------------------------------
// HID Control Defines
	// Output Report IDs
#define WIIMOTE_HID_CONTROL_LED_FF				0x11
#define WIIMOTE_HID_CONTROL_CONTROLLER_STATUS	0x15
#define WIIMOTE_HID_CONTROL_WRITE				0x16
#define WIIMOTE_HID_CONTROL_READ				0x17

	// Input Report IDs
#define WIIMOTE_HID_RID_INPUT_READ				0x21
#define WIIMOTE_HID_RID_INPUT_WRITE_ACK			0x22

//-----------------------------------------------------------------------------
// Save Data Defines
#define WIIMOTE_SAVEDATA_BEGIN					0x0000
#define WIIMOTE_SAVEDATA_SIZE					0x1700

//-----------------------------------------------------------------------------
// MII Data Defines
#define WIIMOTE_MII_DATA_BEGIN_ADDR			0x0FCA

#define WIIMOTE_MII_DATA_BEGIN_1			0x0FD2
#define WIIMOTE_MII_DATA_BYTES_PER_SLOT		74
#define WIIMOTE_MII_SLOT_NUM				10

#define WIIMOTE_MII_SECTION1_BEGIN_ADDR		0x0FCA
#define WIIMOTE_MII_SECTION2_BEGIN_ADDR		0x12BA
#define WIIMOTE_MII_SECTION_SIZE			750		// Size in bytes

#define WIIMOTE_MII_CHECKSUM1_ADDR			0x12B8
#define WIIMOTE_MII_CHECKSUM2_ADDR			0x15A8
#define WIIMOTE_MII_CHECKSUM_SIZE			0x0002	// Size in bytes

#define WIIMOTE_MII_PARADESLOTS_ADDR		0x0FCE
#define WIIMOTE_MII_PARADESLOTS_SIZE		0x0002

//-----------------------------------------------------------------------------
// MII CRC16 Defines
#define WIIMOTE_MII_CRC16_POLY			0x1021
#define WIIMOTE_MII_CRC16_INITIAL		0xFFFF
#define WIIMOTE_MII_CRC16_POSTXOR		0xEF4C

//-----------------------------------------------------------------------------
// MII Data Structure Defines
#define MII_NAME_LENGTH				10
#define MII_CREATOR_NAME_LENGTH		10

#define MII_HEIGHT_MIN				0x00
#define MII_HEIGHT_MAX				0x7F

#define MII_WEIGHT_MIN				0x00
#define MII_WEIGHT_MAX				0x7F

#if 0
typedef struct
{
// addr: 0x00 & 0x01
//				11110110 fav color 12
//				11110100 fav color 11
//0 1 0000 0000 000000
//0 1 1100 1111 110110
//0 1 0001 1111 110110
//	 		    010110
//0 1 0000 0000 001100
//0 1 0000 0000 001100
//0 1 1111 1111 110111
//0 0 1001 1000 111110
	u16 invalid:1;
	u16 isGirl:1;
	u16 month:4;
	u16 day:5;
	u16 favColor:4;		// 0 - 11
	u16 unknown:1;

// addr: 0x02 through 0x15
	u16 name[MII_NAME_LENGTH];					// Unicode name

// addr: 0x16
	u8 height;

// addr: 0x17
	u8 weight;

// addr: 0x18 - 0x1B
	u8 miiID1;  // Unique Mii identifier
	u8 miiID2;
	u8 miiID3;
	u8 miiID4;

// addr: 0x1C & 0x1D & 0x1E & 0x1F
	u8 systemID0;	// Checksum8 of first 3 bytes of mac addr
	u8 systemID1;	// mac addr 3rd-to-last byte
	u8 systemID2;	// mac addr 2nd-to-last byte
	u8 systemID3;	// mac addr last byte

// addr: 0x20
//	0010 0000	face  1
//	1110 0000	face  7 
//	0100 0000   face  2

//	0000 0000		- skin color 0
//	0000 0100		- skin color 1

//	0010 0000		= skin color white 0
//	0011 0100		= skin color black 7 ??
	u16 faceshape:3;
	u16 skinColor:3; // 0 - 5

	u16 facialFeature:4; // 0 - 11


// addr: 0x20 & 0x21 & 0x22 & 0x23
// 00000100 00000100 	- no facial features
// 00000110 11000100 	- last facial feature (16?)
//							   1000000	= hair part 1
//							   1100000  = hair part 1
//					  10000100 1000000	= hair color 1
//					  10000111 1000000	= hair color 7
//					  00011000 			= hair style 36
//					  00111010			= hair style 63


//							   01000000	= hair color 1
//						       00000000	= hair color 0
// 
	
//	00000000 00000100		- facial feature 0			
//			 01000100		- facial feature 1

//  01000000 00000100		- facial feature 0
//  01000001 10000100		- facial feature 7
	00000000 00000111

	u16 unknown;
// addr: 0x22 & 0x23
//  01000010	- hair type 0
//  00111110	- hair type 11
//	00011000	- hair type
//	00011010	- hair type
//	00011010 01000000	hair color 1
//	00011011 10000000	hair color 6
//			 01000000	hair part 1
//			 01100000	hair part 2
//	01010000 01000000	hair color 1
//	01010001 10000000	hair color 6
//  01000010			hair 0/0  33?
//  10001000			hair 0/10 68?
	u16 unknown:7:
	u16 hairColor:3;
	u16 hairPart:1;

// addr: 0x24 & 0x25
// 00110001				- eyebrow type 0/0
// 10010001				- eyebrow type 1/9
// 00000001				- eyebrow type 0/1
// 01010001				- eyebrow type 1/1
// 01100001 01000000	- eyebrow rotate mid
// 01100010 11000000	- eyebrow rotate max 
		 
// 00000001 10000000	- eyebrow type 1
// 10111001 01000000	- eyebrow type max (32?)

	u16 unknown:?;
	u16 eyebrowrotation:?;
	u16 unknown:?;
	
// addr: 0x26 & 0x27
// 00110000				= eyebrow max size
// 00100000				= eyebrow min size
// 00101000 10100010	= eyebrow normal 
// 00101001 00100010	= eyebrows down all the way
// 00101000				= eyebrow color 1
// 11101000				= eyebrow color 7
// 00101000				= eyebrow size mid
// 00110000				= eyebrow size max
//			10100010	- mid
//			00110010	- max (mid + 7)
//			10100010	- outset normal 
//			10100000	- outset to max
//			00110010	- inset normal
//		    00111100	- inset to max
	u16 eyebrowColor:3;
	u16 eyebrowSize:5;		// min 0, max 16 (depends on eyebrow type?)
	u16 eyebrowVOffset:?;
	u16 eyebrowSpacing:?;

// addr: 0x28 & 0x29
// 00001000			  - eyes 0/0
// 00010100			  - eyes 2/0
// 00011100 1000 1100 - eyes 4/0
// 00111000 0110 1100 - eyes 4/10
// 00010000 0110 1100 - eyes 1
// 01101100 1000 1100 - eyes 9
// 00001000 1000 1100 - eyes 0
// 10111100 0110 1100 - eyes 48     
// 01101100 1000 1100 - eyes 9  (page 0, option 
// 10000100 0110 1100 - eyes 12 (page 1, option 0)
// 10111100 0110 1100 - eyes 4
// 00011100 1000 1100 - eyes 36
//			0110 1100	- eyes rotated normal
//			0000 1100	- eyes rotated all the way in
//			1000 1100	- eyes at mid height
//			1001 0010	- eyes moved down all the way
//			0110 1100	- eyes at mid height
//			0110 0000    - eyes up to max height
//			0000 1100	- eyes rotate min
//			1110 1100   - eyes rotate max (7)
	u16 uknown:8;
	u16 eyeRotation:3;
	u16 unknown:1;
	u16 eyeHeight:4; // 12 == mid, 0 == up

// addr: 0x2A & 0x2B
// 000 00000 0100 0000 - eyes outset to mid
// 000 00000 0000 0000 - eyes inset all the way to max
// 000 01000 0100 0000 - eyes outset to mid 
// 000 01001 1000 0000 - eyes outset to max
// 000 01000		   - eyes medium size
// 000 01110		   - eyes to max size
// 000 01000		   - eye color 0 (black)
// 101 01000		   - eye color 5 (green)

	u16	eyeColor:3;

// addr: 0x2C & 0x2D
// 0001 0100				- nose type 0
// 1011 0100				- nose type 11
// 			  0100 1000		- nose mid height
//			  0000 0000		- nose max height
// 0001 0100				- nose mid size
// 0001 0000				- nose min size
// 0001 0100				- nose mid size
// 0001 1000				- nose max size
//			  0100 1000		- nose mid height
//			  1001 0000		- nose min height
	u16 noseType:4;			// 0 - 11
	u16 noseSize:4;			// 0 - 8
	u16 noseHeight:4;		// 0 - 9?
	u16 unknown:3;

// addr: 0x2E & 2F
// 0011 1000	- lip type 1/0
// 0101 1000	- lip type 1/7
// 0110 1000	- lip type 0/11
// 0010 1000	- lip type 0/5
// 1011 1000	- lip type 0/0 (23?)
// 0110 0000	- lip type 1/11 (12?)
// 1011 1000	- lips type 0/0
// 0000 0000	- lips type 0/6 (2 pages, 12 per page)
// 1011 1000	- lips type 0
// 0110 1000	- lips type 11
// 1011 1000	- lip color 0	  
// 1011 1100	- lip color 2
// 1011 1000	- lip type page 0, 0
// 0011 1000	- lip type page 1, 0
//			 1000 1101	- lips at mid
//			 1001 0010	- lips up max
//			 1000 1101	- lips at mid
//			 1000 0000	- lips down max
	u16 lipType:5;

// addr: 0x30
//	00000000	- glasses 0 (none)
//	01110000	- glasses 7
//  00000000	- glasses color 0
//  00001010	- glasses color 5
	u8 unknown:1;
	u8 glassesType:3;
	u8 glassesColor:3;
	u8 unknown:1;

// addr: 0x31 & 32
// 1110 0000			- glasses at max height 
// 1111 0100			- glasses at min height (-20)
// 1000 1010			- glasses at mid height
// 1000 0000			- glasses up to max height
//           0000 0000	- mustache 0
//		     1100 0000  - mustache 3
// 10000000				- glasses mid size
// 11100000				- glasses max size (3 notches)
//			 0000 0000  - beard 0
//			 0011 0000  - beard 3
//			 0000 0000	- beard color 0
//			 0000 1110  - beard color 7
	u16 glassesSize:3;
	u16 unknown:1;
	u16 glassesHeight:4;
	u16 mustacheType:2;
	u16 beardType:2;
	u16 facialHairColor:3;
	u16 unknown:2;

// addr: 0x33
// 1000 1010	- mustache mid height
// 1000 0000	- mustache max height (-10)
// 1000 1010	- mustache med height
// 1000 0000	- mustache max height (-10)
// 1000 1010	- mustach mid size
// 0000 1010	- mustache max size
// 1000 1010	- mustache mid height
// 1001 0000	- mustache down all the way (+6)
	u8 mustacheSize:3;		// 0 (max) - ? (min)
	u8 unknown:1;
	u8 mustacheHeight:4;	// 0 (high) - 16 (low)

// addr: 0x34 & 0x35
// 0010 0101	- mole off
// 1100 0101	- mole on and max size (+4)
// 0010 0101	- mole off
// 1010 0101	- mole on
// 1010 0101	- mole off
// 1010 0000    - mole on and moved to max height (-20)
// 0010 0101 00000100 - mole middle
// 1010 0101 00000000 - mole left max
	u16 moleOn:1;
	u16 moleSize:3;			// ? - 8
	u16 moleHeight:5;		// 0 - 
	u16 unknown:4;
	u16 moleHorizontal:3;	// 0 (left) - ? (right)

// addr: 0x36
	u16 creatorName[MII_CREATOR_NAME_LENGTH];

} MII_DATA_STRUCT;
#endif



#endif // _DEFINES_H_