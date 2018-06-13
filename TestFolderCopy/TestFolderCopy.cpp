// TestAsyncFileCopy.cpp : define o ponto de entrada para o aplicativo do console.
//

#include "stdafx.h"
#include "../AsyncFileCopy/AsyncFileCopy.h"

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

	LPCTSTR source = TEXT("inFile.txt"); // SO - T3 - V2\TestAsyncFileCopy
	LPCTSTR destination = TEXT("outFile.txt");















	CONTEXT myctx;
	GetThreadContext(GetCurrentThread(), &myctx);

	AsyncInit();
	CopyFileAsynch((PCSTR)source, (PCSTR)destination, mycb, &myctx);
	printf("copying. enter any key to terminate\n");
	getchar();
	AsyncTerminate();
	return 0;
}

