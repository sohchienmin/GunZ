// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "stdio.h"
#include <fstream>
#include <Windows.h>
#include <stdint.h>

#include "./lib/Detours Express 3.0/include/detours.h"


#pragma comment(lib, "./lib/Detours Express 3.0/lib.X86/detours.lib")

using namespace std;

#define EncryptAddress 0x401120
#define DecryptAddress 0x4010F0
#define DecryptAddressExe 0x66e890
#define EncryptFunctionAddress 0x4027E0 	// Decrypt Function
#define DecryptFunctionAddress 0x4013D0		// Decrypt Function

typedef void(*pEncrypt)(char* pData, int _size);
typedef void(*pDecrypt)(char* pData, int _size);
typedef void(*pDecryptExe)(char* pData, int _size);
typedef void(*pEncryptFunction)(char pFileName);
typedef void(*pDecryptFunction)(const char* pArg, char pFileName);

pEncrypt pCustomEncrypt = (pEncrypt) (EncryptAddress);
pDecrypt pCustomDecrypt = (pDecrypt) (DecryptAddress);
pDecryptExe pCustomDecryptExe = (pDecryptExe) (DecryptAddressExe);
pEncryptFunction pCustomEncryptFunction = (pEncryptFunction) (EncryptFunctionAddress);
pDecryptFunction pCustomDecryptFunction = (pDecryptFunction) (DecryptFunctionAddress);

char KEYS[] =
{
	69, 420, 666, 777, 133, 7 //37, 409, 577, 607, 619, 5 (old encryption)
};

void CustomEncrypt(char* pData, int _size)
{
	if (!pData) return;

	for (int i = 0; i < _size; i++)
	{
		unsigned char b = pData[i];
		for (int d = 0; d < 6; d++)
		{
			b = b ^ KEYS[d % (sizeof(KEYS) / sizeof(char))];
		}
		pData[i] = b;
	}
}

void CustomDecrypt(char* pData, int _size)
{
	if (!pData) return;

	for (int i = 0; i < _size; i++)
	{
		unsigned char b = pData[i];
		for (int d = 0; d < 6; d++)
		{
			b = b ^ KEYS[d % (sizeof(KEYS) / sizeof(char))];
		}
		pData[i] = b;
	}
}

void CustomEncryptFunction(char pFileName)
{
	return pCustomEncryptFunction(pFileName);
}

void CustomDecryptFunction(const char* pArg, char pFileName)
{
	return pCustomDecryptFunction(pArg, pFileName);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) 
	{
		case DLL_PROCESS_ATTACH:
			{
				DetourTransactionBegin();
				DetourUpdateThread(GetCurrentThread());
				DetourAttach(&(PVOID&) pCustomEncrypt, CustomEncrypt);
				DetourAttach(&(PVOID&) pCustomDecrypt, CustomDecrypt);
				DetourAttach(&(PVOID&) pCustomEncryptFunction, CustomEncryptFunction);
				DetourAttach(&(PVOID&) pCustomDecryptFunction, CustomDecryptFunction);
				DetourTransactionCommit();
			}
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}