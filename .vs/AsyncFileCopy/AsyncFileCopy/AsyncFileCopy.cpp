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
		GetQueuedCompletionStatus(
			iocp,
			&NumberOfBytes,
			&CompletionKey,
			&OverlappedPtr,
			INFINITE
		);
	printf(
		"IOCP (%d): %d, %llu, %p\n",
		GetCurrentThreadId(),
		NumberOfBytes,
		CompletionKey,
		OverlappedPtr
	);
	printf(
		"IOCP (%d): %d, %llu, %p\n",
		GetCurrentThreadId(),
		NumberOfBytes,
		CompletionKey,
		OverlappedPtr
	);
	printf(
		"IOCP (%d): %d, %llu, %p\n",
		GetCurrentThreadId(),
		NumberOfBytes,
		CompletionKey,
		OverlappedPtr
	);
	getchar();
		PASYNC_CTX pAsyncCtx = (PASYNC_CTX)CompletionKey;
	}
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

	printf("creating");
	getchar();
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
	printf("created");
	getchar();
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



