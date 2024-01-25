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
#ifndef _WIIMOTE_HID_H_
#define _WIIMOTE_HID_H_

//-----------------------------------------------------------------------------
// Program Includes:
#include "Defines.h"

//-----------------------------------------------------------------------------
// System Includes:
#include <wtypes.h>
#include <initguid.h>

extern "C" 
{
#include "C:\WINDDK\3790.1830\inc\ddk\wdm\wxp\hidsdi.h"
}

#include <setupapi.h>
#include <dbt.h>

#pragma comment(lib, "C://WINDDK//3790.1830//lib//wxp//i386//hid.lib")
#pragma comment(lib, "setupapi.lib")

#include <fstream>
#include <memory>

using namespace std;

//-----------------------------------------------------------------------------
// Defines:
#define WIIMOTE_STRING_IDENTIFIER_MAX_LENGTH	256

//-----------------------------------------------------------------------------
// Class Definitions:
class CWiimoteHID
{
public:
	CWiimoteHID();
	virtual ~CWiimoteHID();
	
	inline bool IsConnected	( ) const { return m_HIDDeviceHandle != INVALID_HANDLE_VALUE; }

	bool GetDeviceString	(wchar_t (&name)[WIIMOTE_STRING_IDENTIFIER_MAX_LENGTH]);

	bool Send_ReadRequest	(const short offset, const short bytes);
	bool Send_WriteData		(const short offset, const short bytes, unsigned char (&writeData)[WIIMOTE_HID_WRITE_DATA_SIZE], const bool flushQueue = false);

	bool HIDReport_Send		(const int reportID, const unsigned char* reportData, const int reportDataLength);
	bool HIDReport_Receive	(unsigned char (&reportData)[WIIMOTE_HID_REPORT_LENGTH]);


	//------------------------------------------------------------------------------
	//  Function:	CWiimoteHID::ReadFromWiimote
	//  Purpose:	
	//  Parameters:	
	//  Returns:	
	//------------------------------------------------------------------------------
	template <unsigned short T_bytes>
	bool ReadFromWiimote(unsigned short offset, unsigned char (&readBuffer)[T_bytes])
	{
		memset(&readBuffer[0], 0, T_bytes);

		// Send initial read data request
		if ( !this->Send_ReadRequest(offset, T_bytes) )
		{
			return false;
		}

		// The sReadSectionNode is used to ensure we receive all the data
		// we ask for, so we can re-request whatever we don't receive, and 
		// fail if we just can't receive all the requested data.
		sReadSectionNode* readNodeHead = new sReadSectionNode(0, T_bytes);

		unsigned char receiveData[WIIMOTE_HID_REPORT_LENGTH];
		memset(&receiveData[0], 0, WIIMOTE_HID_REPORT_LENGTH);

		// Start receiving data
		while ( this->HIDReport_Receive(receiveData) )
		{
			if (receiveData[0] == WIIMOTE_HID_RID_INPUT_READ)
			{
				int dataOffset = ((receiveData[4] << 8) + receiveData[5]) - offset;
				int dataSize   = ((receiveData[3] & 0xF0) >> 4) + 1;

				// Write the received data to the buffer
				memcpy(&readBuffer[dataOffset], &receiveData[6], dataSize);

				// Save a record of what we just received
				if ( this->MarkDataSegment(readNodeHead, dataSize, dataOffset) )
				{
					break;
				}
			}
		}

		bool success = (readNodeHead == NULL);

		if (!success)
		{
			// TODO: Attempt to re-receive any data we missed
			sReadSectionNode* currNode = readNodeHead;
			sReadSectionNode* nextNode = NULL;

			while (currNode != NULL)
			{
				nextNode = currNode->Next;

				delete currNode;

				currNode = nextNode;
			}
		}

		return success;
	} // CWiimoteHID::ReadFromWiimote

	//------------------------------------------------------------------------------
	//  Function:	CWiimoteHID::WriteToWiimote
	//  Purpose:	
	//  Parameters:	
	//  Returns:	
	//------------------------------------------------------------------------------
	template <unsigned short T_bytes>
	bool WriteToWiimote(unsigned short offset, const unsigned char (&writeBuffer)[T_bytes])
	{
		if (T_bytes == 0)
		{
			return false;
		}

		sReadSectionNode* readNodeHead = new sReadSectionNode(0, T_bytes);

		unsigned char sendBuff[WIIMOTE_HID_WRITE_DATA_SIZE];
		unsigned char receiveBuff[WIIMOTE_HID_REPORT_LENGTH];

		int dataIdx = 0;

		// Send the data
		while (dataIdx < T_bytes)
		{
			int bytesLeft = T_bytes - dataIdx;
			int writeSize = WIIMOTE_HID_WRITE_DATA_SIZE > bytesLeft ? bytesLeft : WIIMOTE_HID_WRITE_DATA_SIZE;

			memset(&sendBuff[0], 0, WIIMOTE_HID_WRITE_DATA_SIZE);
			memcpy(&sendBuff[0], &writeBuffer[dataIdx], writeSize);

			if ( !this->Send_WriteData(offset + dataIdx, writeSize, sendBuff, true) )
			{
				return false;
			}

			// Receive write acknowledgement
			memset(&receiveBuff[0], 0, WIIMOTE_HID_REPORT_LENGTH);

			while ( this->HIDReport_Receive(receiveBuff) )
			{
				if (receiveBuff[0] == WIIMOTE_HID_RID_INPUT_WRITE_ACK)
				{
					this->MarkDataSegment(readNodeHead, writeSize, dataIdx);
					
					break;
				}
			}

			dataIdx += writeSize;
		}

		bool success = (readNodeHead == NULL);

		if (!success)
		{
			// TODO: Attempt to re-send any data we didnt get an ack for
			sReadSectionNode* currNode = readNodeHead;
			sReadSectionNode* nextNode = NULL;

			while (currNode != NULL)
			{
				nextNode = currNode->Next;

				delete currNode;

				currNode = nextNode;
			}
		}

		return success;
	} // CWiimoteHID::WriteToWiimote

private:
	struct sReadSectionNode
	{
		unsigned short Offset;
		unsigned short Size;

		sReadSectionNode* Next;
		sReadSectionNode* Last;

		sReadSectionNode(const unsigned short offset, const unsigned short size)
		{
			this->Offset	= offset;
			this->Size		= size;
			this->Next		= NULL;
			this->Last		= NULL;
		}
	};

	bool Connect	( );
	void Disconnect ( );

	bool MarkDataSegment(sReadSectionNode*& readNodeHead, unsigned short dataSize, unsigned short dataOffset);

	HANDLE		m_HIDDeviceHandle;
	HANDLE		m_HIDWriteHandle;
	HANDLE		m_HIDReadHandle;
	HIDP_CAPS	m_HIDDeviceCaps;
	HANDLE		m_HIDEventObjHandle;
	OVERLAPPED	m_OverlappedEventHandle;

};

#endif // _WIIMOTE_HID_H_