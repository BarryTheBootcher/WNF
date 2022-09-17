#include <Windows.h>
#include <stdio.h>

int main()
{
	int errorCode = 0;
	BOOL success = FALSE;
	DWORD bytesWritten = 0;
	CHAR str[] = "Successfully ran\n";

	HANDLE hOutputFile = CreateFileA("C:\\payload-output.txt", GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_SYSTEM, NULL);
	if (INVALID_HANDLE_VALUE == hOutputFile)
	{
		printf("Failed to create C:\\payload-output.txt - GLE: %d\n", GetLastError());
		errorCode = 1;
		goto CLEANUP;
	}

	if (FALSE == WriteFile(hOutputFile, str, (DWORD)strlen(str), &bytesWritten, NULL))
	{
		printf("Failed to write to C:\\payload-output.txt - GLE: %d\n", GetLastError());
		errorCode = 2;
		goto CLEANUP;
	}

CLEANUP:
	if (INVALID_HANDLE_VALUE != hOutputFile)
	{
		CloseHandle(hOutputFile);
	}

	return errorCode;
}