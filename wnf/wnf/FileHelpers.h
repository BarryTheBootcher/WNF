#pragma once

#include <Windows.h>

BOOL writeBufferToFile(
	_In_ const LPVOID fileInMemBuf,
	_In_ const LONGLONG fileSize
);

BOOL readFileIntoBuffer(
	_In_ const LPCSTR payloadExePath,
	_Out_ LPVOID* fileInMemBuf,
	_Out_ PLARGE_INTEGER fileSize
);