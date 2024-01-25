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
#ifndef _UTIL_LED_H_
#define _UTIL_LED_H_

//-----------------------------------------------------------------------------
// Program Includes:
#include "Defines.h"

//-----------------------------------------------------------------------------
// Forward References:
class CWiimoteHID;

//-----------------------------------------------------------------------------
// Class Definitions:
class Util_LED
{
public:
	static bool SetState(CWiimoteHID& hid, const int led1On = true, const int led2On = false, const int led3On = false, const int led4On = false);

	static bool DisplayNumber(CWiimoteHID& hid, const int displayNumber);
};

#endif /* _UTIL_LED_H_ */