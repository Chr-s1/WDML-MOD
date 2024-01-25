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
#include "Util_LED.h"
#include "WiimoteHID.h"

//------------------------------------------------------------------------------
//  Function:	Util_LED::SetState
//  Purpose:	Set the state of the LEDs on the Wiimote.
//  Parameters:	
//  Returns:	bool - true if the new status was successfully sent to the device.
//------------------------------------------------------------------------------
bool Util_LED::SetState(CWiimoteHID& hid, const int led1On, const int led2On, const int led3On, const int led4On)
{
	unsigned char sendData[WIIMOTE_HID_REPORT_PAYLOAD_LENGTH];

	memset(&sendData[0], 0, WIIMOTE_HID_REPORT_PAYLOAD_LENGTH);
	
	sendData[0] = 0x00			| 
		(led1On ? 0x10 : 0x00)	| 
		(led2On ? 0x20 : 0x00)	|
		(led3On ? 0x40 : 0x00)	|
		(led4On ? 0x80 : 0x00);

	return hid.HIDReport_Send(WIIMOTE_HID_CONTROL_LED_FF, &sendData[0], WIIMOTE_HID_REPORT_PAYLOAD_LENGTH);
} // Util_LED::SetState

//------------------------------------------------------------------------------
//  Function:	Util_LED::DisplayNumber
//  Purpose:	Sets the state of the LEDs on the Wiimote so that the given number is displayed
//  Parameters:	int displayNumber - Number from 0 to 15 to display on the LEDs.
//  Returns:	bool - true if the new status was successfully sent to the device.
//------------------------------------------------------------------------------
bool Util_LED::DisplayNumber(CWiimoteHID& hid, const int displayNumber)
{
	if (displayNumber > 15 || displayNumber < 0)
	{
		return false;
	}

	unsigned char sendData[WIIMOTE_HID_REPORT_PAYLOAD_LENGTH];

	memset(&sendData[0], 0, WIIMOTE_HID_REPORT_PAYLOAD_LENGTH);
	
	sendData[0] = (displayNumber << 4) & 0xF0;

	return hid.HIDReport_Send(WIIMOTE_HID_CONTROL_LED_FF, &sendData[0], WIIMOTE_HID_REPORT_PAYLOAD_LENGTH);
} // Util_LED::DisplayNumber
