// AsyncFileCopy.cpp: define as funções exportadas para o aplicativo DLL.
//
#include "stdafx.h"

#include "AsyncFileCopy.h"
#include <windows.h>
#include <stdio.h>

#define MAX_PROCESSING 4

#define NUM_WORKERS (MAX_PROCESSING * 2)

#define BUFFER_SIZE 2048

#define forever for(;;)


DWORD WINAPI WorkerThread(LPVOID lpParameter) {
	HANDLE iocp = (HANDLE)lpParameter;

	DWORD NumberOfBytes;
	ULONG_PTR CompletionKey;
	LPOVERLAPPED OverlappedPtr;

	forever{
		printf("Reporting for duty\n");
		GetQueuedCompletionStatus(
			iocp,
			&NumberOfBytes,
			&CompletionKey,
			&OverlappedPtr,
			INFINITE
		);
		
		printf("number: %d\n", NumberOfBytes);
		if ((int)NumberOfBytes <= 0) break;

		PASYNC_CTX pAsyncCtx = (PASYNC_CTX)CompletionKey;
		if (NumberOfBytes == 0)
		{
			printf("Read 0, gonna terminate\n");
			pAsyncCtx->cb(
				pAsyncCtx->userCtx,
				0,
				NumberOfBytes);

			CloseHandle(pAsyncCtx->inFile);
			CloseHandle(pAsyncCtx->outFile);
			free(pAsyncCtx);
			break;
		}
		if (pAsyncCtx->lastRead)
		{
			printf("Just got done reading. Gonna write.\n");
			pAsyncCtx->lastRead = false;
			//  Register the context with the iocp
			HANDLE assoc = CreateIoCompletionPort(
				pAsyncCtx->outFile,
				iocp,
				(ULONG_PTR)pAsyncCtx,
				0
			);

			//	Start the operation
			BOOL res = WriteFile(
				pAsyncCtx->outFile,
				pAsyncCtx->buffer,
				BUFFER_SIZE,
				&(pAsyncCtx->numberOfBytesRead),
				&(pAsyncCtx->overlapped)
			);
		}
		/*else {
			printf("Finished writing. Gonna try to read more.\n");
			pAsyncCtx->lastRead = true;
			//  Register the context with the iocp
			HANDLE assoc = CreateIoCompletionPort(
				pAsyncCtx->inFile,
				iocp,
				(ULONG_PTR)pAsyncCtx,
				0
			);

			//	Start the operation
			BOOL res = ReadFile(
				pAsyncCtx->inFile,
				pAsyncCtx->buffer,
				BUFFER_SIZE,
				&(pAsyncCtx->numberOfBytesRead),
				&(pAsyncCtx->overlapped)
			);
			
		}*/
		else {
			return;
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

BOOL CopyFileAsynch(PCSTR srcFile, PCSTR dstFile, AsyncCallback cb, LPVOID userCtx) {
	//  Create the handles 
	HANDLE inFile = CreateFile(
		(LPCTSTR)srcFile,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,	
		OPEN_ALWAYS,
		FILE_FLAG_OVERLAPPED,
		NULL);
	if (inFile == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "FAIL: CREATE IN FILE\n");
		exit(1);
	}
	HANDLE outFile = CreateFile(
		(LPCTSTR)dstFile,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_FLAG_OVERLAPPED,
		NULL);

	if (outFile == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "FAIL: CREATE OUT FILE\n");
		exit(1);
	}
 
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
		BUFFER_SIZE,
		&(pAsyncCtx->numberOfBytesRead),
		&(pAsyncCtx->overlapped)
	);

	return true;
}

VOID AsyncTerminate() {
	free(pAsyncCtx);
	CloseHandle(iocp);
}



