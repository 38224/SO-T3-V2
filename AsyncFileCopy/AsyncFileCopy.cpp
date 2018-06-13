// AsyncFileCopy.cpp: define as funções exportadas para o aplicativo DLL.
//
#include "stdafx.h"

#include "AsyncFileCopy.h"
#include <windows.h>
#include <stdio.h>

#define MAX_PROCESSING 4

#define NUM_WORKERS (MAX_PROCESSING * 2)

#define BUFFER_SIZE 4

#define forever for(;;)


HANDLE iocp, inFile, outFile;
PASYNC_CTX pAsyncCtx;
int total_bytes;


VOID FinishCleanly(BOOL success) {
	DWORD code = success ? 0 : GetLastError();
	// free resources
	if (pAsyncCtx) {
		CloseHandle(pAsyncCtx->inFile);
		CloseHandle(pAsyncCtx->outFile);
	}
	// signal workers
	for (int i = 0; i < NUM_WORKERS; i++)
		PostQueuedCompletionStatus(
			iocp,
			0,
			0,
			NULL);
	// call user callback
	pAsyncCtx->cb(pAsyncCtx->userCtx, code, total_bytes);
}

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
		
		// work is done
		if (CompletionKey == NULL)
			break;

		PASYNC_CTX pAsyncCtx = (PASYNC_CTX)CompletionKey;
		if ((int)NumberOfBytes == 0) {
			FinishCleanly(true);
			continue;
		}
		if (pAsyncCtx->lastRead)
		{
			pAsyncCtx->lastRead = false;
			
			//  Register the context with the iocp
			HANDLE assoc = CreateIoCompletionPort(
				pAsyncCtx->outFile,
				iocp,
				(ULONG_PTR)pAsyncCtx,
				0
			);

			if (INVALID_HANDLE_VALUE == assoc) {
				FinishCleanly(false);
				continue;
			}

			//  Start the write
			BOOL res = WriteFile(
				pAsyncCtx->outFile,
				pAsyncCtx->buffer,
				(int)NumberOfBytes,
				&(pAsyncCtx->numberOfBytesRead),
				&(pAsyncCtx->overlapped)
			);

			if (res) {
				FinishCleanly(false);
				continue;
			}
			total_bytes += (int)NumberOfBytes;
			printf("Wrote %d bytes\n", total_bytes);
		}
		else {
			pAsyncCtx->lastRead = true;
			// ADJUST OVERLAPPED SO READ/WRITE OPERATIONS CONTINUE FROM LAST READ POSITION
			pAsyncCtx->overlapped.Offset += (int)NumberOfBytes;
			BOOL res = ReadFile(
				pAsyncCtx->inFile,
				pAsyncCtx->buffer,
				BUFFER_SIZE,
				&(pAsyncCtx->numberOfBytesRead),
				&(pAsyncCtx->overlapped)
			);
			if (res) {
				FinishCleanly(false);
				continue;
			}
		}
	}
	return 0;
}

BOOL AsyncInit() {
	total_bytes = 0;
	BOOL result = true;
	// Create the iocp
	iocp = CreateIoCompletionPort(
		INVALID_HANDLE_VALUE,
		NULL,
		0,
		MAX_PROCESSING
	);

	if (!iocp)
		return false;

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
	inFile = CreateFile(
		(LPCTSTR)srcFile,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,	
		OPEN_ALWAYS,
		FILE_FLAG_OVERLAPPED,
		NULL);

	if (INVALID_HANDLE_VALUE == inFile)
		FinishCleanly(false);
	
	outFile = CreateFile(
		(LPCTSTR)dstFile,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_FLAG_OVERLAPPED,
		NULL);

	if (INVALID_HANDLE_VALUE == outFile)
		FinishCleanly(false);
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

	if (INVALID_HANDLE_VALUE == assoc)
		FinishCleanly(false);

	//	Start the operation
	BOOL res = ReadFile(
		inFile,
		pAsyncCtx->buffer,
		BUFFER_SIZE,//BUFFER_SIZE,
		&(pAsyncCtx->numberOfBytesRead),
		&(pAsyncCtx->overlapped)
	);

	if (res) {
		FinishCleanly(false);
	}

	return true;
}

VOID AsyncTerminate() {
	free(pAsyncCtx);
	CloseHandle(iocp);
}