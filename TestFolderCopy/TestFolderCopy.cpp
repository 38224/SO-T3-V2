// TestAsyncFileCopy.cpp : define o ponto de entrada para o aplicativo do console.
//

#include "stdafx.h"
#include "../AsyncFileCopy/AsyncFileCopy.h"
#include <windows.h>




#define DllImport   __declspec( dllimport )  

DllImport BOOL AsyncInit();
DllImport BOOL CopyFileAsynch(PCSTR srcFile, PCSTR dstFile, AsyncCallback cb, LPVOID userCtx);
DllImport VOID AsyncTerminate();

VOID mycb(LPVOID userCtx, DWORD status, UINT64 transferedBytes)
{
	printf("Operation completed with status: %d\n", status);
}

int main()
{
<<<<<<< HEAD
	/*
	HANDLE WINAPI FindFirstFile(
		_In_  LPCTSTR           lpFileName,
		_Out_ LPWIN32_FIND_DATA lpFindFileData
	);
	*/
	LPCTSTR in = TEXT("inFolder");
	LPCTSTR out = TEXT("outFolder");
	LPWIN32_FIND_DATA data;
	HANDLE h = FindFirstFile(in, data);
	CONTEXT myctx;
	GetThreadContext(GetCurrentThread(), &myctx);
	AsyncInit();
	//BOOL CopyFileAsynch(PCSTR srcFile, PCSTR dstFile, AsyncCallback cb, LPVOID userCtx) {


	LPCTSTR source = TEXT("inFile.txt"); // SO - T3 - V2\TestAsyncFileCopy
	LPCTSTR destination = TEXT("outFile.txt");
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	size_t length_of_arg;


	StringCchLength((STRSAFE_LPCWSTR)"E:\\PROJ", MAX_PATH, &length_of_arg);
	StringCchCopy(szDir, MAX_PATH, (STRSAFE_LPCWSTR)"E:\\PROJ");
	StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

	hFind = FindFirstFile(szDir, &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		printf("invalid hFind handle");
		getchar();
		exit(1);
	}


	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			_tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
		}
		else
		{
			filesize.LowPart = ffd.nFileSizeLow;
			filesize.HighPart = ffd.nFileSizeHigh;
			_tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
			getchar();
		}
	} while (FindNextFile(hFind, &ffd) != 0);


	 
=======
	LPCTSTR source1 = TEXT("inFile1.txt"); // SO - T3 - V2\TestAsyncFileCopy
	LPCTSTR source2 = TEXT("inFile2.txt");
	LPCTSTR source3 = TEXT("inFile3.txt");
	LPCTSTR destination1 = TEXT("outFile1.txt");
	LPCTSTR destination2 = TEXT("outFile2.txt");
	LPCTSTR destination3 = TEXT("outFile3.txt");

>>>>>>> e00299f607a9206dd1595ac7a19197590ef76c03

	CONTEXT myctx;
	GetThreadContext(GetCurrentThread(), &myctx);

	AsyncInit();
	CopyFileAsynch((PCSTR)source1, (PCSTR)destination1, mycb, &myctx);
	CopyFileAsynch((PCSTR)source2, (PCSTR)destination2, mycb, &myctx);
	CopyFileAsynch((PCSTR)source3, (PCSTR)destination3, mycb, &myctx);
	printf("copying. enter any key to terminate\n");



	getchar();
	AsyncTerminate();
	return 0;
}

/*



#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#pragma comment(lib, "User32.lib")


WIN32_FIND_DATA ffd;
LARGE_INTEGER filesize;
TCHAR szDir[MAX_PATH];
HANDLE hFind = INVALID_HANDLE_VALUE;
size_t length_of_arg;
TCHAR *argg[] = { "filesToRead" };


StringCchLength((STRSAFE_LPCWSTR)"filesToRead", MAX_PATH, &length_of_arg);
StringCchCopy(szDir, MAX_PATH, (STRSAFE_LPCWSTR)"filesToRead");
StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

hFind = FindFirstFile(szDir, &ffd);

if (INVALID_HANDLE_VALUE == hFind)
{
printf("invalid hFind handle");
getchar();
exit(1);
}


do
{
if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
{
_tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
}
else
{
filesize.LowPart = ffd.nFileSizeLow;
filesize.HighPart = ffd.nFileSizeHigh;
_tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
getchar();
}
} while (FindNextFile(hFind, &ffd) != 0);
*/

