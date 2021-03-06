#include "StrainGauge.h"

#include <iostream>


bool StrainGauge::initializeCom()
{
	_hSerial = ::CreateFile(_sPortName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
	                        FILE_ATTRIBUTE_NORMAL,
	                        nullptr);

	if (_hSerial == INVALID_HANDLE_VALUE)
	{
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			std::cout << "Serial port does not exist" << std::endl;
			return false;
		}
		std::cout << "Some other error occurred" << std::endl;
		return false;
	}
	return true;
}

StrainGauge::StrainGauge(const LPCTSTR port)
	: _sPortName(port),
	  _sSendChar{0x1, 0x0, 0x0, 0x0, 0x0}
{
	initializeCom();
}

bool StrainGauge::connect()
{
	DCB dcbSerialParams;
	/*
	 * Changed "DCB dcbSerialParams = {0}" to "DCB dcbSerialParams", because {0} - it is initialising
	 * of the first attribute of this structure, buuut... That was done in the next string,
	 * because DCBlength - the first parameter.
	 */
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(_hSerial, &dcbSerialParams))
	{
		std::cout << "Getting state error" << std::endl;
		return false;
	}
	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (!SetCommState(_hSerial, &dcbSerialParams))
	{
		std::cout << "Error setting serial port state" << std::endl;
		return false;
	}
	char sStopChar[5] = {0x7, 0x0, 0x0, 0x0, 0x0};
	WriteFile(_hSerial, &sStopChar, 5, &_iSize, nullptr);

	std::cout << "Connected successfully!" << std::endl;
	return true;
}

void StrainGauge::readComStrain(std::array<int, 6>& data)
{
	///char period = 0x000186A0;

	WriteFile(_hSerial, &_sSendChar, 5, &_iSize, nullptr);

	// Recieve 1 byte of answer.
	ReadFile(_hSerial, &_skip, 1, &_iSize, nullptr);
	// std::cout << "Skipped: " << skip + '0' - 48 << std::endl;
	for (size_t i = 0; i < 6; ++i)
	{
		// Recieve 2 bytes of actual data.
		ReadFile(_hSerial, &_sReceivedChar, 2, &_iSize, nullptr);
		data.at(i) = _sReceivedChar;
	}
}

void StrainGauge::whatsPeriod()
{
	uint32_t period;

	char sStopChar[5] = {0x7, 0x0, 0x0, 0x0, 0x0};
	WriteFile(_hSerial, &sStopChar, 5, &_iSize, nullptr);

	char sSendChar[5] = {0x8, 0x0, 0x0, 0x0, 0x0};
	WriteFile(_hSerial, &sSendChar, 5, &_iSize, nullptr);

	// Recieve 1 byte of answer.
	ReadFile(_hSerial, &_skip, 1, &_iSize, nullptr);
	// std::cout << "Skipped: " << _skip + '0' - 48 << std::endl;

	// Recieve 2 bytes of actual data.
	ReadFile(_hSerial, &period, 4, &_iSize, nullptr);
	std::cout << "Period: " << period << std::endl;
}

StrainGauge::~StrainGauge()
{
	CloseHandle(_hSerial);
}
