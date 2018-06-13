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
	//const char source[] = "C:\\Users\\hspov\\source\\repos\\AsyncFileCopy\\TestAsyncFileCopy\\slb.txt";
	//const char destination[] = "C:\\Users\\hspov\\source\\repos\\AsyncFileCopy\\TestAsyncFileCopy\\omaior.txt";
	CONTEXT myctx;
	GetThreadContext(GetCurrentThread(), &myctx);

	AsyncInit();
	CopyFileAsynch((PCSTR)source, (PCSTR)destination, mycb, &myctx);
	
	
	AsyncTerminate();	
	printf("complete,enter any key\n");
	getchar();
    return 0;
}

