#pragma once
#include "memory.h"
#include <iostream>
#include <string>
#include <vector>

//-----------------------------------------------------------------------------
//	
//-----------------------------------------------------------------------------
static bool bInitialized{ false };					//	
static VMM_HANDLE pHandle{ nullptr };				//	
static DWORD procID{ 0 };							//	
static __int64 dwModuleBase{ 0 };					//	
static VMMDLL_PROCESS_INFORMATION procInfo{ 0 };	//	

//-----------------------------------------------------------------------------
//	
//-----------------------------------------------------------------------------
PCIMemory::PCIMemory()
{
	if (pHandle)
		VMMDLL_Close(pHandle);

	LPSTR args[] = { (LPSTR)"", (LPSTR)"-device" , (LPSTR)"FPGA" };
	pHandle = VMMDLL_Initialize(3, args);

	printf("[+] PCIMemory::PCIMemory\n");
}

PCIMemory::PCIMemory(const char* procName)
{
	if (pHandle)
		VMMDLL_Close(pHandle);
	
	DWORD mType = 3;
	LPSTR args[] = { (LPSTR)"", (LPSTR)"-device" , (LPSTR)"FPGA" };
	pHandle = VMMDLL_Initialize(3, args);
	InitProcess(procName);

	printf("[+] PCIMemory::PCIMemory(const char*)\n");
}

PCIMemory::PCIMemory(LPSTR* args, const char* procName, DWORD mType)
{
	if (pHandle)
		VMMDLL_Close(pHandle);


	pHandle = VMMDLL_Initialize(mType, args);
	InitProcess(procName);

	printf("[+] PCIMemory::PCIMemory(LPSTR*, cost char*, DWORD)\n");
}

PCIMemory::~PCIMemory() 
{ 
	if (pHandle)
		VMMDLL_Close((VMM_HANDLE)pHandle); 

	printf("[-] PCIMemory::~PCIMemory\n");
}

//-----------------------------------------------------------------------------
//	
//-----------------------------------------------------------------------------

bool PCIMemory::InitProcess(const char* procName)
{
	VMMDLL_PidGetFromName(pHandle, (LPSTR)procName, &procID);
	dwModuleBase = GetModuleBase(procID, procName);
	GetProcInfo(procID, procInfo);
}

bool PCIMemory::GetProcID(const char* name, int& result)
{
	if (!pHandle)
		return false;

	return VMMDLL_PidGetFromName((VMM_HANDLE)pHandle, (LPSTR)name, (PDWORD)&result);
}

bool PCIMemory::GetProcInfo(int dwPID, VMMDLL_PROCESS_INFORMATION& result)
{
	if (!dwPID)
		return false;

	SIZE_T cbProcessInformation = sizeof(VMMDLL_PROCESS_INFORMATION);
	ZeroMemory(&result, sizeof(VMMDLL_PROCESS_INFORMATION));
	result.magic = VMMDLL_PROCESS_INFORMATION_MAGIC;
	result.wVersion = VMMDLL_PROCESS_INFORMATION_VERSION;
	return VMMDLL_ProcessGetInformation(pHandle, dwPID, &result, &cbProcessInformation);
}

__int64 PCIMemory::GetModuleBase(int dwPID, const char* name)
{
	if (!pHandle)
		return 0;

	return VMMDLL_ProcessGetModuleBaseU((VMM_HANDLE)pHandle, dwPID, (LPSTR)name);
}

__int64 PCIMemory::GetProcPEB(int dwPID)
{
	VMMDLL_PROCESS_INFORMATION procInfo{};
	if (!GetProcInfo(dwPID, procInfo))
		return 0;

	return procInfo.win.vaPEB;
}

bool PCIMemory::ReadVirtualMemory(int dwPID, __int64 pAddress, LPVOID lResult, DWORD cbSize)
{
	if (!pHandle)
		return false;

	return VMMDLL_MemRead(pHandle, dwPID, pAddress, (PBYTE)lResult, cbSize);
}

bool PCIMemory::WriteVirtualMemory(int dwPID, __int64 pAddress, LPVOID lPatch, DWORD cbSize)
{
	if (!pHandle)
		return false;

	return VMMDLL_MemWrite(pHandle, dwPID, pAddress, (PBYTE)lPatch, cbSize);
}

__int64 PCIMemory::ResolvePtrChain(int dwPID, __int64 baseAddr, DWORD offsets[], int count)
{
	if (!pHandle)
		return 0;

	__int64 result = baseAddr;
	for (int i = 0; i < count; i++)
	{
		result = Read<__int64>(dwPID, result);
		result += offsets[i];
	}
	return result;
}

bool PCIMemory::DumpModule(int dwPID, const char* modName, std::vector<char>& out)
{
	PVMMDLL_MAP_MODULEENTRY modEntry32{};
	if (!VMMDLL_Map_GetModuleFromNameU(pHandle, dwPID, (LPSTR)modName, &modEntry32, 0))
		return false;

	__int64 modBase = modEntry32->vaBase;
	auto fileSize = modEntry32->cbFileSizeRaw;		//	file size
	auto imageSize = modEntry32->cbImageSize;		//	unpacked file size
	DWORD mSize = fileSize;							//	@TODO: image size tends to be different and will result in a read failure
	out.resize(mSize);

	ReadVirtualMemory(dwPID, modBase, out.data(), mSize);
	///	@TODO: sometimes memory will fail to read even though result will be valid. related to size
	//	if (!ReadVirtualMemory(dwPID, modBase, out.data(), mSize))
	//	{
	//		VMMDLL_MemFree(modEntry32);
	//		out.clear();
	//		printf("failed to read virtual memory -> 0x%llX\nfileSize: %d\nimageSize: %d\n", modBase, fileSize, imageSize);
	//		return false;
	//	}
	VMMDLL_MemFree(modEntry32);

	return true;
}

bool PCIMemory::DumpBytes(int dwPID, __int64 lpAddress, DWORD cbSize, std::vector<char>& out)
{
	out.resize(cbSize);

	if (!ReadVirtualMemory(dwPID, lpAddress, out.data(), cbSize))
		return false;

	return true;
}

bool PCIMemory::MapSectionMemory(char* wxBytes, LPVOID& outData, DWORD cbSize)
{
	HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, GetCurrentProcessId());
	if (hProcess == NULL)
		return false;

	outData = VirtualAllocEx(hProcess, NULL, cbSize, MEM_COMMIT, PAGE_READWRITE);
	if (outData == NULL)
	{
		CloseHandle(hProcess);
		return false;
	}

	SIZE_T bytesWritten;
	if (!WriteProcessMemory(hProcess, outData, wxBytes, cbSize, &bytesWritten))
	{
		VirtualFreeEx(hProcess, outData, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}
	CloseHandle(hProcess);
	return true;
}

bool PCIMemory::FreeMapSection(LPVOID pData, DWORD cbSize)
{
	HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, GetCurrentProcessId());
	if (hProcess == NULL)
		return false;

	VirtualFreeEx(hProcess, pData, 0, MEM_RELEASE);
	CloseHandle(hProcess);
	return true;
}

void PCIMemory::PrintSectionMemory(std::vector<char> bytes, __int64 addr)
{
	auto base = addr;
	for (int i = 0; i < bytes.size(); i++)
	{
		if (i % 8 == 0)
		{
			printf("\n[0x%llX][0x%X]:\t", (void*)((__int64)base), (i));
			base += 0x8;
		}
		printf("%02X ", static_cast<unsigned char>(bytes[i]));
	}
}

void PCIMemory::PrintSectionMemory(int dwPID, __int64 addr, DWORD cbSize)
{
	std::vector<char> outBytes;
	if (!PCIMemory::DumpBytes(dwPID, addr, cbSize, outBytes))
		return;

	PrintSectionMemory(outBytes, addr);
}


//-----------------------------------------------------------------------------
//	
//-----------------------------------------------------------------------------

DWORD PCIMemory::GetProcID() { return procID; }
__int64 PCIMemory::GetModuleBase() { return dwModuleBase; }

bool PCIMemory::GetProcInfo(VMMDLL_PROCESS_INFORMATION& result)
{
	if (!procID)
		return false;

	return GetProcInfo(procID, result);
}

__int64 PCIMemory::GetModuleBase(const char* name)
{
	if (!procID)
		return false;

	GetModuleBase(procID, name);
}

__int64 PCIMemory::GetProcPEB()
{
	if (!procID)
		return false;

	return GetProcPEB(procID);
}

bool PCIMemory::ReadVirtualMemory(__int64 pAddress, LPVOID lResult, DWORD cbSize)
{
	if (!procID)
		return false;

	return ReadVirtualMemory(procID, pAddress, lResult, cbSize);
}

bool PCIMemory::WriteVirtualMemory(__int64 pAddress, LPVOID lPatch, DWORD cbSize)
{
	if (!procID)
		return false;

	return WriteVirtualMemory(procID, pAddress, lPatch, cbSize);
}

__int64 PCIMemory::ResolvePtrChain(__int64 baseAddr, DWORD offsets[], int count)
{
	if (!procID)
		return false;

	return ResolvePtrChain(procID, baseAddr, offsets, count);
}