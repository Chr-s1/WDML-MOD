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
// Changelog:
//	13.Dec.2006	-	Added fallback to WriteFile method of sending reports if
//					HidD_SetOutputReport call fails.  
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Program Includes:
#include "WiimoteHID.h"

//-----------------------------------------------------------------------------
// Constant Definitions:
#define WIIMOTE_HID_READINPUTREPORT_TIMEOUT		3000
#define WIIMOTE_HID_NUM_READ_BUFFERS			250


//------------------------------------------------------------------------------
//  Function:	CWiimoteHID::CWiimoteHID
//  Purpose:	Constructor
//  Parameters:	None.
//  Returns:	None.
//------------------------------------------------------------------------------
CWiimoteHID::CWiimoteHID()
:	m_HIDDeviceHandle(INVALID_HANDLE_VALUE)
,	m_HIDWriteHandle(INVALID_HANDLE_VALUE)
,	m_HIDReadHandle(INVALID_HANDLE_VALUE)
,	m_HIDEventObjHandle(INVALID_HANDLE_VALUE)
{
	this->Connect();

	// Attempt to increase the number of buffers
	BOOLEAN result = HidD_SetNumInputBuffers(m_HIDDeviceHandle, WIIMOTE_HID_NUM_READ_BUFFERS);

	return;
} // CWiimoteHID::CWiimoteHID

//------------------------------------------------------------------------------
//  Function:	CWiimoteHID::~CWiimoteHID
//  Purpose:	Destructor
//  Parameters:	None.
//  Returns:	None.
//------------------------------------------------------------------------------
CWiimoteHID::~CWiimoteHID()
{
	this->Disconnect();	
	
	return;
} // CWiimoteHID::~CWiimoteHID

//------------------------------------------------------------------------------
//  Function:	CWiimoteHID::Connect
//  Purpose:	Connect to the first Wiimote found on the system.
//  Parameters:	None.
//  Returns:	bool - true if a wiimote was found and connected to, false otherwise.
//------------------------------------------------------------------------------
bool CWiimoteHID::Connect()
{
	bool						deviceFound = false;	// Was the device found?
	GUID						hidGUID;				// GUID for HID system
	SP_DEVICE_INTERFACE_DATA	devInfoData;			// Device Info Data struct

	HidD_GetHidGuid(&hidGUID);	

	HANDLE hDevInfo = SetupDiGetClassDevs (&hidGUID, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

	devInfoData.cbSize = sizeof(devInfoData);

	// Enumerate all system devices trying to find the Wiimote.
	int				memberIndex		= 0;	// Device index
	ULONG			devBufferLength = 0;	// Device buffer length
	HIDD_ATTRIBUTES deviceAttributes;

	while (SetupDiEnumDeviceInterfaces(hDevInfo, 0, &hidGUID, memberIndex++, &devInfoData) != 0)
	{
		// This call will fail, but will give us the device buffer length to call this function again with
		LONG devResult = SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInfoData, NULL, 0, &devBufferLength, NULL);

		PSP_DEVICE_INTERFACE_DETAIL_DATA detailData = static_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>( malloc(devBufferLength) );

		detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		devResult = SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInfoData, detailData, devBufferLength, &devBufferLength, NULL);

		// Open device handle to get device attributes
		m_HIDDeviceHandle		= CreateFile(detailData->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, static_cast<LPSECURITY_ATTRIBUTES>(NULL), OPEN_EXISTING, 0, NULL);
		deviceAttributes.Size	= sizeof(deviceAttributes);

		// Is it a Wiimote?		
		if ( HidD_GetAttributes(m_HIDDeviceHandle, &deviceAttributes)	&&
			(deviceAttributes.VendorID  == WIIMOTE_HID_VENDOR_ID)		&& 
			(deviceAttributes.ProductID == WIIMOTE_HID_PRODUCT_ID) )
		{				
			// Get a handle for writing Output reports.
			m_HIDWriteHandle = CreateFile(
				detailData->DevicePath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 
				static_cast<LPSECURITY_ATTRIBUTES>(NULL), OPEN_EXISTING, 0, NULL);

			// Get a handle for reading Input reports.
			m_HIDReadHandle = CreateFile(
				detailData->DevicePath,  GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
				static_cast<LPSECURITY_ATTRIBUTES>(NULL), OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
					
			// Get device capabilities
			PHIDP_PREPARSED_DATA	preparsedData;

			HidD_GetPreparsedData	(m_HIDDeviceHandle,  &preparsedData);
			HidP_GetCaps			(preparsedData,		 &m_HIDDeviceCaps);
			HidD_FreePreparsedData	(preparsedData);

			m_HIDEventObjHandle = CreateEvent(NULL, TRUE, TRUE, reinterpret_cast<LPCWSTR>(""));

			//Set the members of the overlapped structure.
			if (m_HIDEventObjHandle != INVALID_HANDLE_VALUE)
			{
				m_OverlappedEventHandle.hEvent		= m_HIDEventObjHandle;
				m_OverlappedEventHandle.Offset		= 0;
				m_OverlappedEventHandle.OffsetHigh	= 0;
			}

			deviceFound	= true;
		}
		else
		{
			CloseHandle(m_HIDDeviceHandle);

			m_HIDDeviceHandle = INVALID_HANDLE_VALUE;
		}

		// Free the memory used by the detailData structure (no longer needed).
		free(detailData);

		if (deviceFound)
		{
			break;
		}
	}	

	// Free the memory reserved for hDevInfo by SetupDiClassDevs.
	SetupDiDestroyDeviceInfoList(hDevInfo);

	return deviceFound;
} // CWiimoteHID::Connect

//------------------------------------------------------------------------------
//  Function:	CWiimoteHID::Disconnect
//  Purpose:	Close all device read/write handles, disconnecting from any Wiimotes.
//  Parameters:	None.
//  Returns:	Nothing.
//------------------------------------------------------------------------------
void CWiimoteHID::Disconnect()
{
	if (m_HIDDeviceHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_HIDDeviceHandle);

		m_HIDDeviceHandle = INVALID_HANDLE_VALUE;
	}

	if (m_HIDWriteHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_HIDWriteHandle);

		m_HIDWriteHandle = INVALID_HANDLE_VALUE;
	}

	if (m_HIDReadHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_HIDReadHandle);

		m_HIDReadHandle = INVALID_HANDLE_VALUE;
	}

	return;
} // CWiimoteHID::Disconnect

//------------------------------------------------------------------------------
//  Function:	CWiimoteHID::HIDReport_Send
//  Purpose:	
//  Parameters:	
//  Returns:	
//------------------------------------------------------------------------------
bool CWiimoteHID::HIDReport_Send(const int reportID, const unsigned char* reportData, const int reportDataLength)
{
	if ( !this->IsConnected() && !this->Connect() )
	{
		return false;
	}

	char outputReport[WIIMOTE_HID_REPORT_LENGTH];

	memset(&outputReport, 0, WIIMOTE_HID_REPORT_LENGTH);

	// Set Report ID
	outputReport[0] = reportID;

	// Copy report data to buffer
	int dataLength = reportDataLength > WIIMOTE_HID_REPORT_PAYLOAD_LENGTH ? WIIMOTE_HID_REPORT_PAYLOAD_LENGTH : reportDataLength;

	memcpy(&outputReport[1], reportData, dataLength);

	// Send report
	BOOLEAN result = HidD_SetOutputReport(m_HIDWriteHandle, outputReport, WIIMOTE_HID_REPORT_LENGTH);

	if ( result != TRUE )
	{	// SetOutputReport failed, so try writefile method
		DWORD numberOfBytesWritten = 0;

		result = WriteFile(
			m_HIDWriteHandle, outputReport, m_HIDDeviceCaps.InputReportByteLength, 
			&numberOfBytesWritten, static_cast<LPOVERLAPPED>(&m_OverlappedEventHandle));
	}

	return (result == TRUE);
} // CWiimoteHID::HIDReport_Send

//------------------------------------------------------------------------------
//  Function:	CWiimoteHID::HIDReport_Send
//  Purpose:	
//  Parameters:	
//  Returns:	
//------------------------------------------------------------------------------
bool CWiimoteHID::HIDReport_Receive(unsigned char (&reportData)[WIIMOTE_HID_REPORT_LENGTH])
{
	if ( !this->IsConnected() && !this->Connect() )
	{
		return false;
	}

	bool	readSuccess			= false;
	DWORD	numberOfBytesRead	= 0;

	/*
	if ( HidD_GetInputReport(m_HIDReadHandle, reportData, m_HIDDeviceCaps.InputReportByteLength) )
	{
		readSuccess = true;
	}
	*/
	
	DWORD readResult = ReadFile(
		m_HIDReadHandle, reportData, m_HIDDeviceCaps.InputReportByteLength, 
		&numberOfBytesRead, static_cast<LPOVERLAPPED>(&m_OverlappedEventHandle)); 

	DWORD waitResult = WaitForSingleObject(m_HIDEventObjHandle, WIIMOTE_HID_READINPUTREPORT_TIMEOUT);

	switch ( waitResult )
	{
	case WAIT_OBJECT_0:
		{
			readSuccess = true;
		}
		break;
	case WAIT_TIMEOUT:
		{	
			// Cancel the Read operation.
			DWORD cancelResult = CancelIo(m_HIDReadHandle);
		
			this->Disconnect();
		}
		break;
	default:
		{	
			// Undefined error
			this->Disconnect();
		}
		break;
	}

	BOOL resetOK = ResetEvent(m_HIDEventObjHandle);

	return readSuccess;
} // CWiimoteHID::HIDReport_Receive

//------------------------------------------------------------------------------
//  Function:	CWiimoteHID::Send_ReadRequest
//  Purpose:	
//  Parameters:	
//  Returns:	True if the report was sent successfully
//------------------------------------------------------------------------------
bool CWiimoteHID::Send_ReadRequest(const short offset, const short bytes)
{
	if ( !this->IsConnected() && !this->Connect() )
	{
		return false;
	}

	unsigned char sendData[WIIMOTE_HID_REPORT_PAYLOAD_LENGTH];

	memset(&sendData[0], 0, WIIMOTE_HID_REPORT_PAYLOAD_LENGTH);
	
	sendData[0] = 0x00;
	sendData[1] = 0x00;

	sendData[2] = (offset & 0xFF00) >> 8;
	sendData[3] = (offset & 0x00FF);

	sendData[4] = (bytes & 0xFF00) >> 8;
	sendData[5] = (bytes & 0x00FF);

	bool success = this->HIDReport_Send(WIIMOTE_HID_CONTROL_READ, &sendData[0], WIIMOTE_HID_REPORT_PAYLOAD_LENGTH);

	return success;
} // CWiimoteHID::Send_ReadRequest

//------------------------------------------------------------------------------
//  Function:	CWiimoteHID::Send_WriteData
//  Purpose:	
//  Parameters:	
//  Returns:	True if the report was sent successfully
//------------------------------------------------------------------------------
bool CWiimoteHID::Send_WriteData(const short offset, const short bytes, unsigned char (&writeData)[WIIMOTE_HID_WRITE_DATA_SIZE], const bool flushQueue)
{
	if ( !this->IsConnected() && !this->Connect() )
	{
		return false;
	}
	unsigned char sendData[WIIMOTE_HID_REPORT_PAYLOAD_LENGTH];

	if (flushQueue)
	{
		HidD_FlushQueue(m_HIDDeviceHandle);
	}

	memset(&sendData[0], 0, WIIMOTE_HID_REPORT_PAYLOAD_LENGTH);
	
	sendData[0] = 0x00;
	sendData[1] = 0x00;

	sendData[2] = (offset & 0xFF00) >> 8;
	sendData[3] = (offset & 0x00FF);

	sendData[4] = (bytes & 0x00FF);

	memcpy(&sendData[5], &writeData[0], bytes);

	bool success = this->HIDReport_Send(WIIMOTE_HID_CONTROL_WRITE, &sendData[0], WIIMOTE_HID_REPORT_PAYLOAD_LENGTH);

	return success;
} // CWiimoteHID::Send_WriteData

//------------------------------------------------------------------------------
//  Function:	CWiimoteHID::MarkDataSegment
//  Purpose:	Using a linked list of sReadSectionNodes, this function will mark
//				sections of data as cleared by removing those sections from the 
//				linked list's covered data segment.
//
//				When called with a linked list with a single node representing
//				bytes 0 - 100, and given the arguments of 10 bytes at offset 50,
//				this function will modify the list to have two nodes, one 
//				representing bytes 0 - 49 and a second represending bytes 60 - 100.
//  Parameters:	
//  Returns:	bool - true if all data has been received.
//------------------------------------------------------------------------------
bool CWiimoteHID::MarkDataSegment(sReadSectionNode*& readNodeHead, unsigned short dataSize, unsigned short dataOffset)
{
	sReadSectionNode* currNode = readNodeHead;
	sReadSectionNode* nextNode = NULL;

	while (currNode != NULL)
	{
		nextNode = currNode->Next;

		if (dataOffset == currNode->Offset)
		{
			if (dataSize >= currNode->Size)
			{
				// Delete current node
				sReadSectionNode* delNode = currNode;

				if (delNode->Last != NULL)
				{
					delNode->Last->Next = delNode->Next;
				}

				if (delNode->Next != NULL)
				{
					delNode->Next->Last = delNode->Last;
				}

				if (delNode == readNodeHead)
				{
					readNodeHead = delNode->Next;
				}

				delete delNode;
			}
			else
			{
				// Advance currnode's offset
				currNode->Offset += dataSize;
				currNode->Size   -= dataSize;
			}
		}
		else if ( (dataOffset > currNode->Offset) && (dataOffset < currNode->Offset + currNode->Size) )
		{
			if (dataOffset + dataSize < currNode->Offset + currNode->Size)
			{
				// Make the beginning of current node its own node
				sReadSectionNode* newNode = new sReadSectionNode(currNode->Offset, dataOffset - currNode->Offset);

				newNode->Next	= currNode;
				newNode->Last   = currNode->Last;
				if (currNode->Last != NULL)
				{
					currNode->Last->Next = newNode;
				}
				currNode->Last  = newNode;
				
				currNode->Offset = dataOffset + dataSize;
				currNode->Size   = currNode->Size - dataSize - newNode->Size;
			}
			else
			{
				// Change current node size
				currNode->Size -= ((currNode->Offset + currNode->Size) - dataOffset);
			}
		}
		else if ( (dataOffset < currNode->Offset) && (dataOffset + dataSize > currNode->Offset) )
		{
			if (dataOffset + dataSize >= currNode->Offset + currNode->Size)
			{
				// delete current node
				if (currNode->Last != NULL)
				{
					currNode->Last->Next = currNode->Next;
				}

				if (currNode->Next != NULL)
				{
					currNode->Next->Last = currNode->Last;
				}

				if (currNode == readNodeHead)
				{
					readNodeHead = currNode->Next;
				}

				delete currNode;
			}
			else
			{
				currNode->Offset = dataOffset + dataSize;
			}
		}

		currNode = nextNode;
	}

	return (readNodeHead == NULL);
} // CWiimoteHID::MarkDataSegment

//------------------------------------------------------------------------------
//  Function:	CWiimoteHID::GetDeviceString
//  Purpose:	Get a wide string describing the connected Wiimote
//  Parameters:	
//  Returns:	
//------------------------------------------------------------------------------
bool CWiimoteHID::GetDeviceString(wchar_t (&name)[WIIMOTE_STRING_IDENTIFIER_MAX_LENGTH])
{
	if ( !this->IsConnected() && !this->Connect() )
	{
		return false;
	}

	char nameBuff  [WIIMOTE_STRING_IDENTIFIER_MAX_LENGTH / 2];
	char serialBuff[WIIMOTE_STRING_IDENTIFIER_MAX_LENGTH / 2];

	if ( !HidD_GetProductString(m_HIDDeviceHandle, &nameBuff[0], WIIMOTE_STRING_IDENTIFIER_MAX_LENGTH / 2) )
	{
		return false;
	}

	if ( !HidD_GetSerialNumberString(m_HIDDeviceHandle, &serialBuff[0], WIIMOTE_STRING_IDENTIFIER_MAX_LENGTH / 2) )
	{
		return false;
	}

	if ( swprintf_s<WIIMOTE_STRING_IDENTIFIER_MAX_LENGTH>(name, L"%s (%s)", nameBuff, serialBuff) == -1 )
	{
		return false;
	}

	return true;
} // CWiimoteHID::GetDeviceString
