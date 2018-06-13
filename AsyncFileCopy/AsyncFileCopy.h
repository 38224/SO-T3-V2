#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#ifdef ASYNCFILECOPYDLL
#define ASYNCFILECOPYDLL __declspec(dllexport)
#else
#define ASYNCFILECOPYDLL __declspec(dllimport)
#endif	

#ifndef UNICODE
#define UNICODE
#endif

#define MAX_PROCESSING 4

#define NUM_WORKERS (MAX_PROCESSING * 2)

#define BUFFER_SIZE 2048


typedef VOID(*AsyncCallback)(
	LPVOID userCtx,	
	DWORD status,	
	UINT64 transferedBytes);


typedef struct AsyncContext {
	OVERLAPPED overlapped;
	CHAR buffer[BUFFER_SIZE];
	DWORD numberOfBytesRead;
	HANDLE inFile;
	HANDLE outFile;
	AsyncCallback cb;
	LPVOID userCtx;
	BOOL lastRead;
} ASYNC_CTX, *PASYNC_CTX;


ASYNCFILECOPYDLL BOOL AsyncInit();
ASYNCFILECOPYDLL BOOL CopyFileAsynch(PCSTR srcFile, PCSTR dstFile, AsyncCallback cb, LPVOID userCtx);
ASYNCFILECOPYDLL VOID AsyncTerminate();

