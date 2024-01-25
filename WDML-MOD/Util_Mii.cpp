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

//-----------------------------------------------------------------------------
// Program Includes:
#include "Util_Mii.h"
#include "WiimoteHID.h"

#include <fstream>
#include <memory>

#include "boost/crc.hpp"
#include "boost/cstdint.hpp"

//------------------------------------------------------------------------------
//  Function:	Util_Mii::GetMIIDataCRC
//  Purpose:	
//  Parameters:	
//  Returns:	
//------------------------------------------------------------------------------
unsigned short Util_Mii::GetMIIDataCRC(const char* filename)
{
	unsigned char buff[ WIIMOTE_MII_SECTION_SIZE ];

	fstream file( filename, ios::in | ios::binary );

	if ( !file )
	{
        return 0;
	}

	file.seekg( WIIMOTE_MII_SECTION1_BEGIN_ADDR );
    file.read ( ( char* )buff, WIIMOTE_MII_SECTION_SIZE );
	file.close( );
	
	return Util_Mii::GetMIIDataCRC(buff);
} // Util_Mii::GetMIIDataCRC

//------------------------------------------------------------------------------
//  Function:	Util_Mii::GetMIIDataCRC
//  Purpose:	
//  Parameters:	
//  Returns:	
//------------------------------------------------------------------------------
unsigned short Util_Mii::GetMIIDataCRC( unsigned char (&miiData)[WIIMOTE_MII_SECTION_SIZE] )
{
	boost::crc_optimal<16, WIIMOTE_MII_CRC16_POLY, WIIMOTE_MII_CRC16_INITIAL, WIIMOTE_MII_CRC16_POSTXOR, false, false>  crc_ccitt;

	crc_ccitt.process_bytes( miiData, WIIMOTE_MII_SECTION_SIZE );

	return crc_ccitt.checksum();
} // Util_Mii::GetMIIDataCRC

//------------------------------------------------------------------------------
//  Function:	Util_Mii::ReadToFile
//  Purpose:	
//  Parameters:	
//  Returns:	
//------------------------------------------------------------------------------
bool Util_Mii::ReadToFile(CWiimoteHID& hid, const int slot, const char* filename)
{
	if (slot >= WIIMOTE_MII_SLOT_NUM || slot < 0)
	{
		return false;
	}

	short miiOffset = WIIMOTE_MII_DATA_BEGIN_1 + (slot * WIIMOTE_MII_DATA_BYTES_PER_SLOT);

	// Read individual MII data from the wiimote
	unsigned char readBuff[WIIMOTE_MII_DATA_BYTES_PER_SLOT];

	if ( !hid.ReadFromWiimote<WIIMOTE_MII_DATA_BYTES_PER_SLOT>(miiOffset, readBuff) )
	{
		return false;
	}

	// Open the output file
	fstream outputFile(filename, ios::out | ios::binary );

	if ( !outputFile )
	{
		return false;
	}

	outputFile.seekp( 0 );

	outputFile.write(reinterpret_cast<char*>(&readBuff[0]), WIIMOTE_MII_DATA_BYTES_PER_SLOT);

	outputFile.close();

	return true;
} // Util_Mii::ReadToFile

//------------------------------------------------------------------------------
//  Function:	Util_Mii::WriteFromFile
//  Purpose:	
//  Parameters:	
//  Returns:	
//------------------------------------------------------------------------------
bool Util_Mii::WriteFromFile(CWiimoteHID& hid, const int slot, const char* filename)
{
	if (slot >= WIIMOTE_MII_SLOT_NUM || slot < 0)
	{
		return false;
	}

	// Read mii file into buffer
	unsigned char miiBuff[ WIIMOTE_MII_DATA_BYTES_PER_SLOT ];

	memset(&miiBuff[0], 0, WIIMOTE_MII_DATA_BYTES_PER_SLOT);

	fstream file( filename, ios::in | ios::binary );

	if ( !file )
	{
        return 0;
	}

	file.seekg( 0 );
    file.read ( ( char* )miiBuff, WIIMOTE_MII_DATA_BYTES_PER_SLOT );
	file.close( );

	// Send new mii data
	unsigned short miiOffset = WIIMOTE_MII_DATA_BEGIN_1 + (slot * WIIMOTE_MII_DATA_BYTES_PER_SLOT);

	if ( !hid.WriteToWiimote<WIIMOTE_MII_DATA_BYTES_PER_SLOT>(miiOffset, miiBuff) )
	{
		return false;
	}

	// Ensure mii is not hidden
	unsigned char paradeData[WIIMOTE_MII_PARADESLOTS_SIZE];

	if ( !hid.ReadFromWiimote<WIIMOTE_MII_PARADESLOTS_SIZE>(WIIMOTE_MII_PARADESLOTS_ADDR, paradeData) )
	{
		return false;
	}

	unsigned short miiVisibility = ((paradeData[0] << 8) | paradeData[1]) & (0xFFFF ^ (0x0001 << slot));

	paradeData[0] = (miiVisibility & 0xFF00) >> 8;
	paradeData[1] = (miiVisibility & 0x00FF);

	if ( !hid.WriteToWiimote<WIIMOTE_MII_PARADESLOTS_SIZE>(WIIMOTE_MII_PARADESLOTS_ADDR, paradeData) )
	{
		return false;
	}

	// Read back entire mii section
	unsigned char miiData[WIIMOTE_MII_SECTION_SIZE];

	if ( !hid.ReadFromWiimote<WIIMOTE_MII_SECTION_SIZE>(WIIMOTE_MII_SECTION1_BEGIN_ADDR, miiData) )
	{
		return false;
	}

	unsigned short crc = Util_Mii::GetMIIDataCRC(miiData);

	// Write crc back
	unsigned char crcBuff[WIIMOTE_MII_CHECKSUM_SIZE];
	memset(&crcBuff[0], 0, WIIMOTE_MII_CHECKSUM_SIZE);

	crcBuff[0] = (crc & 0xFF00) >> 8;
	crcBuff[1] = (crc & 0x00FF);

	if ( !hid.WriteToWiimote<WIIMOTE_MII_CHECKSUM_SIZE>(WIIMOTE_MII_CHECKSUM1_ADDR, crcBuff) )
	{
		return false;
	}

	return true;
} // Util_Mii::WriteFromFile
