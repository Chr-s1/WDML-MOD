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


//-----------------------------------------------------------------------------
// MiiTransfer - A Wiimote Data Management Library command line utility for the
//				 transfer of Mii data to and from the Wiimote.
//
//	To compile this program:
//
//	1 - Visual Studio 2005 is most likely required.
//	2 - Install the Windows Driver Development Kit (DDK) from here:
//			http://www.microsoft.com/whdc/DevTools/ddk/default.mspx
//	3 - Download boost and include it in the project.  Boost can 
//		be found here:
//			http://sourceforge.net/projects/boost/
//	4 - You may need to fix paths to the Windows DDK in the WiimoteHID.h, as the
//		DDK does not appear to add the correct paths in Visual Studio 2005 when
//		it is installed.
//	5 - You should now be ready and able to compile this program.
//
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Includes:
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#include "WiimoteHID.h"

#include "Util_LED.h"
#include "Util_Mii.h"
#include "Util_SaveData.h"

//------------------------------------------------------------------------------
// Constants:
#define MAX_FILENAME_LENGTH				512
#define MII_TRANSFER_VERSION_STRING		"0.9.1"

typedef enum
{
	PROGRAM_MODE_NONE,
	PROGRAM_MODE_READ_MII,
	PROGRAM_MODE_READ_ALL_MIIS,
	PROGRAM_MODE_WRITE_MII,

} PROGRAM_MODE;

//------------------------------------------------------------------------------
// Structure Definitions:
typedef struct
{
	PROGRAM_MODE	mode;
	int				miiSlotNum;
	char			filename[MAX_FILENAME_LENGTH];
	char			dirname[MAX_FILENAME_LENGTH];
	char			fulldumpFilename[MAX_FILENAME_LENGTH];

} PROGRAM_OPTIONS;

//------------------------------------------------------------------------------
// Variable Declarations:
static PROGRAM_OPTIONS	gProgramOptions;

//------------------------------------------------------------------------------
// Function Prototypes:
static void	PrintUsage		(const char* programName);
static bool ParseArguments	(int argc, char *argv[]);

//------------------------------------------------------------------------------
//  Function:	main
//  Purpose:	Main program entry point.
//  Parameters:	
//  Returns:	
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	if ( !ParseArguments(argc, argv) )
	{
		PrintUsage(argv[0]);

		return 1;
	}
	
	// Connect to the HID device
	CWiimoteHID hidConnection;

	if ( !hidConnection.IsConnected() )
	{
		printf("Could not establish Wiimote connection.\n");

		return 1;
	}

	// Print device string
	wchar_t nameBuff[WIIMOTE_STRING_IDENTIFIER_MAX_LENGTH];
	
	wprintf(L"Connected to: %s\n", hidConnection.GetDeviceString(nameBuff) ? nameBuff : L"Unknown");

	// Read miis to file
	switch (gProgramOptions.mode)
	{
	//-------------------------------------------------------------------------
	// Read a single Mii to file.
	//-------------------------------------------------------------------------
	case PROGRAM_MODE_READ_MII:
		{
			printf("Receiving Mii Slot %d Data...", gProgramOptions.miiSlotNum);

			Util_LED::DisplayNumber(hidConnection, gProgramOptions.miiSlotNum);

			if ( Util_Mii::ReadToFile(hidConnection, gProgramOptions.miiSlotNum, &gProgramOptions.filename[0]) )
			{
				printf("\tOK\n");
			}
			else
			{
				printf("\tERROR\n");
			}
		}
		break;
	//-------------------------------------------------------------------------
	// Read all Miis to file.
	//-------------------------------------------------------------------------
	case PROGRAM_MODE_READ_ALL_MIIS:
		{
			char miiFileName[MAX_FILENAME_LENGTH];

			for (int miiNum = 0; miiNum < WIIMOTE_MII_SLOT_NUM; miiNum++)
			{
				printf("Receiving Mii Slot %d Data...", miiNum);	

				Util_LED::DisplayNumber(hidConnection, miiNum);

				memset(&miiFileName[0], 0, MAX_FILENAME_LENGTH);

				sprintf_s<MAX_FILENAME_LENGTH>(miiFileName, "%s\\mii%d.mii", gProgramOptions.dirname, miiNum);

				if ( Util_Mii::ReadToFile(hidConnection, miiNum, &miiFileName[0]) )
				{
					printf("\tOK\n");
				}
				else
				{
					printf("\tERROR\n");
				}
			}
		}
		break;
	//-------------------------------------------------------------------------
	// Write a Mii from file.
	//-------------------------------------------------------------------------
	case PROGRAM_MODE_WRITE_MII:
		{
			printf("Sending Mii Slot %d Data...", gProgramOptions.miiSlotNum);

			Util_LED::DisplayNumber(hidConnection, gProgramOptions.miiSlotNum);

			if ( Util_Mii::WriteFromFile(hidConnection, gProgramOptions.miiSlotNum, gProgramOptions.filename) )
			{
				printf("\tOK\n");
			}
			else
			{
				printf("\tERROR\n");
			}
		}
		break;
	}

	// Full save data dump option
	if ( strlen(gProgramOptions.fulldumpFilename) > 0 )
	{
		printf("Receiving Full Save Data...");

		Util_LED::DisplayNumber(hidConnection, 0xF);

		if ( !Util_SaveData::ReadAllToFile(hidConnection, gProgramOptions.fulldumpFilename) )
		{
			printf("\tERROR\n");
		}
		else
		{
			printf("\tOK (0x%x)\n", Util_Mii::GetMIIDataCRC(gProgramOptions.fulldumpFilename));
		}
	}

	return 0;
} // main

//------------------------------------------------------------------------------
//  Function:	ParseArguments
//  Purpose:	Parse given arguments into options structure
//  Parameters:	
//  Returns:	
//------------------------------------------------------------------------------
bool ParseArguments(int argc, char *argv[])
{
	bool success = true;

	gProgramOptions.mode		= PROGRAM_MODE_NONE;
	gProgramOptions.miiSlotNum	= 0;
	
	memset(&gProgramOptions.filename[0],		  0, MAX_FILENAME_LENGTH);
	memset(&gProgramOptions.dirname[0],			  0, MAX_FILENAME_LENGTH);
	memset(&gProgramOptions.fulldumpFilename[0],  0, MAX_FILENAME_LENGTH);

	for (int i=0; i<argc; i++)
	{
		size_t stringLen = strlen(argv[i]);

		if (stringLen > 1 && argv[i][0] == '-')
		{
			switch (argv[i][1])
			{
			//--------------------------------
			// -r(:N) option
			//--------------------------------
			case 'r':
				{
					if (stringLen > 2)
					{
						if ( (argv[i][2] == ':') &&  (sscanf_s(&argv[i][3], "%d", &gProgramOptions.miiSlotNum) != 0) )
						{
							gProgramOptions.mode = PROGRAM_MODE_READ_MII;
						}
						else
						{
							success = false;
						}
					}
					else
					{
						gProgramOptions.mode = PROGRAM_MODE_READ_ALL_MIIS;
					}
				}
				break;
			//--------------------------------
			// -w(:N) option
			//--------------------------------
			case 'w':
				{
					if ( (stringLen > 2) && (argv[i][2] == ':') && (sscanf_s(&argv[i][3], "%d", &gProgramOptions.miiSlotNum) != 0) )
					{
						gProgramOptions.mode = PROGRAM_MODE_WRITE_MII;
					}
					else
					{
						success = false;
					}
				}
				break;
			//--------------------------------
			// -f:FILENAME option
			//--------------------------------
			case 'f':
				{
					if ( (stringLen > 2) && (argv[i][2] == ':') && (sprintf_s<MAX_FILENAME_LENGTH>(gProgramOptions.filename, "%s", &argv[i][3]) != 0) )
					{
						// Success
					}
					else
					{
						printf("Invalid filename specified\n");
						success = false;
					}
				}
				break;
			//--------------------------------
			// -d:DIRECTORY option
			//--------------------------------
			case 'd':
				{
					if ( (stringLen > 2) && (argv[i][2] == ':') && (sprintf_s<MAX_FILENAME_LENGTH>(gProgramOptions.dirname, "%s", &argv[i][3]) != 0) )
					{
						// Success
					}
					else
					{
						printf("Invalid directory specified\n");
						success = false;
					}
				}
				break;
			//--------------------------------
			// -d:FILENAME option
			//--------------------------------
			case 's':
				{
					if ( (stringLen > 2) && (argv[i][2] == ':') && (sprintf_s<MAX_FILENAME_LENGTH>(gProgramOptions.fulldumpFilename, "%s", &argv[i][3]) != 0) )
					{
						// Success
					}
					else
					{
						printf("Invalid filename specified\n");
						success = false;
					}
				}
				break;
			}
		}
	}

	if (gProgramOptions.mode == PROGRAM_MODE_NONE)
	{
		printf("No mode specified\n");

		success = false;
	}

	if (gProgramOptions.mode == PROGRAM_MODE_READ_MII && strlen(gProgramOptions.filename) == 0)
	{
		printf("No filename specified\n");

		success = false;
	}

	if (gProgramOptions.mode == PROGRAM_MODE_READ_ALL_MIIS && strlen(gProgramOptions.dirname) == 0)
	{
		printf("No filename specified\n");

		success = false;
	}


	return success;
} // ParseArguments

//------------------------------------------------------------------------------
//  Function:	PrintUsage
//  Purpose:	Print proper program usage
//  Parameters:	
//  Returns:	
//------------------------------------------------------------------------------
void PrintUsage(const char* programName)
{
	printf("Usage: \n\t%s -[r|w]:N  -f:FILENAME\n\n", programName);
	printf("-r:N\t\tRead Mii in slot N from Wiimote to file (-r reads all slots).\n");
	printf("-w:N\t\tWrite Mii from file to slot N on Wiimote.\n");
	printf("-f:FILENAME\tFile to read from/write to (when using -r:N or -w:N option).\n");
	printf("-d:DIRNAME\tDirectory to write Mii files to (when using -r option).\n");
	printf("-s:FILENAME\tFile to dump full save data contents to.\n");
	
	printf("\n\nMiiTransfer Version:\t %s\n",	MII_TRANSFER_VERSION_STRING);
	printf("WDML Version:\t\t %s\n",			WDML_VERSION_STRING);

	return;
} // PrintUsage

