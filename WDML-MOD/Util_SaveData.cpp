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
#include "Util_SaveData.h"
#include "WiimoteHID.h"


//------------------------------------------------------------------------------
//  Function:	Util_SaveData::ReadAllToFile
//  Purpose:	Read all save data to the given file
//  Parameters:	
//  Returns:	
//------------------------------------------------------------------------------
bool Util_SaveData::ReadAllToFile(CWiimoteHID& hid, const char* filename)
{
	// Read all save data from the wiimote
	unsigned char readBuff[WIIMOTE_SAVEDATA_SIZE];

	if ( !hid.ReadFromWiimote<WIIMOTE_SAVEDATA_SIZE>(WIIMOTE_SAVEDATA_BEGIN, readBuff) )
	{
		return false;
	}

	// Open the output file
	fstream outputFile( filename, ios::out | ios::binary );

	if ( !outputFile )
	{
		return false;
	}

	outputFile.seekp( 0 );

	outputFile.write(reinterpret_cast<char*>(&readBuff[0]), WIIMOTE_SAVEDATA_SIZE);

	outputFile.close();

	return true;
} // Util_SaveData::ReadToFile

//------------------------------------------------------------------------------
//  Function:	Util_SaveData::WriteAllFromFile
//  Purpose:	
//  Parameters:	
//  Returns:	
//------------------------------------------------------------------------------
bool Util_SaveData::WriteAllFromFile(CWiimoteHID& hid, const char* filename)
{

	return false;
} // Util_SaveData::WriteAllFromFile

