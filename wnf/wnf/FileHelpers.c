#include "FileHelpers.h"

#include <stdio.h>

BOOL writeBufferToFile(
	_In_ const LPVOID fileInMemBuf,
	_In_ const LONGLONG fileSize
)
{
	BOOL success = FALSE;
	DWORD numBytesWritten = 0;

	HANDLE hDroppedFile = CreateFileA("C:\\droppedPayload.exe", GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_SYSTEM, NULL);
	if (INVALID_HANDLE_VALUE == hDroppedFile)
	{
		printf("Failed to open dropped file -- GLE: %d\n", GetLastError());
		goto CLEANUP;
	}

	if (FALSE == WriteFile(hDroppedFile, fileInMemBuf, (DWORD)fileSize, &numBytesWritten, NULL))
	{
		printf("Failed to write dropped file -- GLE: %d\n", GetLastError());
		goto CLEANUP;
	}

	success = TRUE;

CLEANUP:
	if (INVALID_HANDLE_VALUE != hDroppedFile)
	{
		CloseHandle(hDroppedFile);
	}

	return success;
}

//Caller needs to HeapFree() fileInMemBuf
BOOL readFileIntoBuffer(
	_In_ const LPCSTR payloadExePath,
	_Out_ LPVOID* fileInMemBuf,
	_Out_ PLARGE_INTEGER fileSize
)
{
	BOOL success = FALSE;
	DWORD numBytesRead = 0;

	HANDLE hPayloadExeFile = CreateFileA(payloadExePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, NULL);
	if (INVALID_HANDLE_VALUE == hPayloadExeFile)
	{
		printf("Failed to open payload exe -- GLE: %d\n", GetLastError());
		goto CLEANUP;
	}


	if (FALSE == GetFileSizeEx(hPayloadExeFile, fileSize))
	{
		printf("Failed to get file size of payload exe\n");
		goto CLEANUP;
	}

	*fileInMemBuf = HeapAlloc(GetProcessHeap(), 0, fileSize->QuadPart);
	if (NULL == *fileInMemBuf)
	{
		printf("Failed to allocate memory for fileInMemBuf using HeapAlloc\n");
		goto CLEANUP;
	}

	if (FALSE == ReadFile(hPayloadExeFile, *fileInMemBuf, (DWORD)fileSize->QuadPart, &numBytesRead, NULL))
	{
		printf("Failed to read payload exe\n");
		goto CLEANUP;
	}

	success = TRUE;

CLEANUP:
	if (INVALID_HANDLE_VALUE != hPayloadExeFile)
	{
		CloseHandle(hPayloadExeFile);
	}

	return success;
}