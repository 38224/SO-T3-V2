// AsyncFileCopy.cpp: define as funções exportadas para o aplicativo DLL.
//
#include "stdafx.h"

#include "AsyncFileCopy.h"
#include <windows.h>
#include <stdio.h>

#define MAX_PROCESSING 4

#define NUM_WORKERS 1 /*(MAX_PROCESSING * 2)*/

#define BUFFER_SIZE 4

#define forever for(;;)


DWORD WINAPI WorkerThread(LPVOID lpParameter) {
	HANDLE iocp = (HANDLE)lpParameter;

	DWORD NumberOfBytes;
	ULONG_PTR CompletionKey;
	LPOVERLAPPED OverlappedPtr;

	forever{
		printf("%d: Reporting for duty\n", GetCurrentThreadId());
		GetQueuedCompletionStatus(
			iocp,
			&NumberOfBytes,
			&CompletionKey,
			&OverlappedPtr,
			INFINITE
		);
		
		//printf("number: %d\n", NumberOfBytes);

		PASYNC_CTX pAsyncCtx = (PASYNC_CTX)CompletionKey;
		if ((int)NumberOfBytes == 0)
		{
			printf("Read 0, gonna terminate\n");
			pAsyncCtx->cb(
				pAsyncCtx->userCtx,
				0,
				NumberOfBytes);
			CloseHandle(pAsyncCtx->inFile);
			CloseHandle(pAsyncCtx->outFile);
			break;
		}
		if (pAsyncCtx->lastRead)
		{
			printf("Finished reading. Gonna write.\n");
			pAsyncCtx->lastRead = false;
			
			//  Register the context with the iocp
			HANDLE assoc = CreateIoCompletionPort(
				pAsyncCtx->outFile,
				iocp,
				(ULONG_PTR)pAsyncCtx,
				0
			);
			//  Start the operation
			BOOL res = WriteFile(
				pAsyncCtx->outFile,
				pAsyncCtx->buffer,
				(int)NumberOfBytes,
				&(pAsyncCtx->numberOfBytesRead),
				&(pAsyncCtx->overlapped)
			);
		}
		else {
			pAsyncCtx->lastRead = true;
			printf("Finished writing. Gonna try to read more.\n");
			pAsyncCtx->overlapped.Offset += BUFFER_SIZE;
			BOOL res = ReadFile(
				pAsyncCtx->inFile,
				pAsyncCtx->buffer,
				BUFFER_SIZE,
				&(pAsyncCtx->numberOfBytesRead),
				&(pAsyncCtx->overlapped)
			);
		}
	}
	return 0;
}

HANDLE iocp;
PASYNC_CTX pAsyncCtx;

BOOL AsyncInit() {
	// Create the iocp
	iocp = CreateIoCompletionPort(
		INVALID_HANDLE_VALUE,
		NULL,
		0,
		MAX_PROCESSING
	);
	//  Create the threads
	for (int i = 0; i < NUM_WORKERS; i++) {
		CreateThread(
			NULL,
			0,
			WorkerThread,
			iocp,
			0,
			NULL
		);
	}
	//  Create the context
	pAsyncCtx = (PASYNC_CTX)malloc(sizeof(ASYNC_CTX));
	memset(pAsyncCtx, 0, sizeof(ASYNC_CTX));
	return true;
}


HANDLE inFile, outFile;
BOOL CopyFileAsynch(PCSTR srcFile, PCSTR dstFile, AsyncCallback cb, LPVOID userCtx) {
	//  Create the handles 
	inFile = CreateFile(
		(LPCTSTR)srcFile,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,	
		OPEN_ALWAYS,
		FILE_FLAG_OVERLAPPED,
		NULL);
	
	outFile = CreateFile(
		(LPCTSTR)dstFile,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_FLAG_OVERLAPPED,
		NULL);

	//  Adjust the context
	
	pAsyncCtx->inFile = inFile;
	pAsyncCtx->outFile = outFile;
	pAsyncCtx->cb = cb;
	pAsyncCtx->userCtx = userCtx;
	pAsyncCtx->lastRead = true;

	//  Register the context with the iocp
	HANDLE assoc = CreateIoCompletionPort(
		inFile,
		iocp,
		(ULONG_PTR)pAsyncCtx,
		0
	);

	//	Start the operation
	BOOL res = ReadFile(
		inFile,
		pAsyncCtx->buffer,
		BUFFER_SIZE,//BUFFER_SIZE,
		&(pAsyncCtx->numberOfBytesRead),
		&(pAsyncCtx->overlapped)
	);
	return true;
}

VOID AsyncTerminate() {
	free(pAsyncCtx);
	CloseHandle(iocp);
}



