#pragma once
#include "Memory.h"


//-----------------------------------------------------------------------------
//	-	PCIMemory - Constructors
//-----------------------------------------------------------------------------

PCIMemory::PCIMemory()
{
	LPSTR args[] = { (LPSTR)"", (LPSTR)"-device" , (LPSTR)"FPGA" };
	PCI_Init(args, 3, pHandle);
	printf("[+] PCIMemory::PCIMemory\n");
}

PCIMemory::PCIMemory(const char* procName)
{
	DWORD mType = 3;
	LPSTR args[] = { (LPSTR)"", (LPSTR)"-device" , (LPSTR)"FPGA" };
	PCI_InitProcess(args, mType, procName);

	auto test = Read<int>(vmProcess.dwModBase);
	printf("[+] PCIMemory::PCIMemory(%s)\n- PID:\t\t%d\n- MODULE:\t0x%llX\n- PEB:\t\t0x%llX\n- EMAGIC:\t0x%X\n\n",
		procName, vmProcess.dwProcID, vmProcess.dwModBase, vmProcess.dwPEB, test);
}

PCIMemory::PCIMemory(LPSTR* args, const char* procName, DWORD mType)
{
	PCI_InitProcess(args, mType, procName);

	auto test = Read<int>(vmProcess.dwModBase);
	printf("[+] PCIMemory::PCIMemory({ %s, %s, %s }, %s, %d)\n- PID:\t\t%d\n- MODULE:\t0x%llX\n- PEB:\t\t0x%llX\n- EMAGIC:\t0x%X\n\n",
		args[0], args[1], args[2], procName, mType, vmProcess.dwProcID, vmProcess.dwModBase, vmProcess.dwPEB, test);
}

PCIMemory::~PCIMemory() 
{ 
	if (pHandle)
		VMMDLL_Close(pHandle); 

	PCIProcess emptyInfo{};
	vmProcess = emptyInfo;

	printf("[-] PCIMemory::~PCIMemory\n");
}


//-----------------------------------------------------------------------------
//	-	PCIMemory - Raw Functions
//-----------------------------------------------------------------------------

/*
* Initializes VMDLL library and returns a default handle for process operations
* User must manually obtain process information for various operations
*/
void PCIMemory::PCI_Init(LPSTR* args, DWORD cType, VMM_HANDLE& vmHandle)
{
	//	close any pre-existing handles
	if (vmHandle)
		VMMDLL_Close(vmHandle);

	//	initialize VMDLL
	vmHandle = VMMDLL_Initialize(cType, args);
}

/*
* Initializes VMMDLL library and auto generates a process structure making it very simple to begin manipulating a specific process
*/
void PCIMemory::PCI_InitProcess(LPSTR* args, DWORD cType, const char* procName)
{
	if (pHandle)
		VMMDLL_Close(pHandle);

	vmProcess.dwProcName = std::string(procName);

	//	Initialize VMDLL
	pHandle = VMMDLL_Initialize(3, args);

	//	Get Process ID
	VMMDLL_PidGetFromName(pHandle, (LPSTR)vmProcess.dwProcName.c_str(), &vmProcess.dwProcID);

	//	Get Module Base
	vmProcess.dwModBase = PCI_GetModuleBase(vmProcess.dwProcID, vmProcess.dwProcName.c_str());

	// Get Process PEB
	vmProcess.dwPEB = PCI_GetProcPEB(vmProcess.dwProcID);

	//	Generate VMDLL_PROCESS_INFORMATION Struct
	PCI_GetProcInfo(vmProcess.dwProcID, vmProcess.vmProcessInfo);
}

/**/
bool PCIMemory::PCI_GetProcessID(const char* name, DWORD& result)
{
	if (!pHandle)
		return PCI_FAILURE;

	return VMMDLL_PidGetFromName((VMM_HANDLE)pHandle, (LPSTR)name, (PDWORD)&result);
}

/**/
bool PCIMemory::PCI_GetProcInfo(int dwPID, VMMDLL_PROCESS_INFORMATION& result)
{
	if (!dwPID)
		return PCI_FAILURE;

	SIZE_T cbProcessInformation = sizeof(VMMDLL_PROCESS_INFORMATION);
	ZeroMemory(&result, sizeof(VMMDLL_PROCESS_INFORMATION));
	result.magic = VMMDLL_PROCESS_INFORMATION_MAGIC;
	result.wVersion = VMMDLL_PROCESS_INFORMATION_VERSION;
	return VMMDLL_ProcessGetInformation(pHandle, dwPID, &result, &cbProcessInformation);
}

/**/
__int64 PCIMemory::PCI_GetModuleBase(int dwPID, const char* name)
{
	if (!pHandle)
		return PCI_ERROR;

	return VMMDLL_ProcessGetModuleBaseU((VMM_HANDLE)pHandle, dwPID, (LPSTR)name);
}

/**/
__int64 PCIMemory::PCI_GetProcPEB(int dwPID)
{
	VMMDLL_PROCESS_INFORMATION procInfo{};
	if (!PCI_GetProcInfo(dwPID, procInfo))
		return PCI_ERROR;

	return procInfo.win.vaPEB;
}

/**/
__int64 PCIMemory::PCI_GetProcAddress(int dwPID, const char* name, const char* fn)
{
	if (!pHandle)
		return PCI_ERROR;

	return VMMDLL_ProcessGetProcAddressU(pHandle, dwPID, (LPSTR)name, (LPSTR)fn);
}

/**/
bool PCIMemory::PCI_GetProcDirectory(int dwPID, const char* modName, std::string& out)
{
	PVMMDLL_MAP_MODULEENTRY modEntry32{};
	if (!VMMDLL_Map_GetModuleFromNameU(pHandle, dwPID, (LPSTR)modName, &modEntry32, 0))
		return PCI_FAILURE;

	out = modEntry32->uszFullName;

	VMMDLL_MemFree(modEntry32);

	return PCI_SUCCESS;
}

/**/
bool PCIMemory::PCI_ReadVirtualMemory(int dwPID, __int64 pAddress, LPVOID lResult, DWORD cbSize)
{
	if (!pHandle)
		return PCI_FAILURE;

	return VMMDLL_MemRead(pHandle, dwPID, pAddress, (PBYTE)lResult, cbSize);
}

/**/
bool PCIMemory::PCI_ReadVirtualMemoryEx(int dwPID, __int64 pAddress, LPVOID lResult, DWORD cbSize)
{
	if (!pHandle)
		return PCI_FAILURE;

	uint32_t flags = VMMDLL_FLAG_NOCACHE | VMMDLL_FLAG_NOPAGING | VMMDLL_FLAG_ZEROPAD_ON_FAIL | VMMDLL_FLAG_NOPAGING_IO;
	return VMMDLL_MemReadEx(pHandle, dwPID, pAddress, (PBYTE)lResult, cbSize, nullptr, flags);
}

/**/
bool PCIMemory::PCI_WriteVirtualMemory(int dwPID, __int64 pAddress, LPVOID lPatch, DWORD cbSize)
{
	if (!pHandle)
		return PCI_FAILURE;

	return VMMDLL_MemWrite(pHandle, dwPID, pAddress, (PBYTE)lPatch, cbSize);
}

/**/
VMMDLL_SCATTER_HANDLE PCIMemory::PCI_CreateScatterHandle(int dwPID)
{
	if (!pHandle)
		return PCI_FAILURE;

	return VMMDLL_Scatter_Initialize(pHandle, dwPID, VMMDLL_FLAG_NOCACHE);
}

/**/
bool PCIMemory::PCI_ClearScatterHandle(VMMDLL_SCATTER_HANDLE hScatter, int dwPID, DWORD flags)
{
	if (!pHandle)
		return PCI_FAILURE;

	return VMMDLL_Scatter_Clear(hScatter, dwPID, flags);
}

/**/
void PCIMemory::PCI_CloseScatterHandle(VMMDLL_SCATTER_HANDLE hScatter)
{
	VMMDLL_Scatter_CloseHandle(hScatter);
}

/**/
bool PCIMemory::PCI_AddReadScatterRequest(VMMDLL_SCATTER_HANDLE hScatter, __int64 pAddress, LPVOID lResult, DWORD cbSize)
{
	if (!pHandle)
		return PCI_FAILURE;

	return VMMDLL_Scatter_PrepareEx(hScatter, pAddress, cbSize, (PBYTE)lResult, 0);
}

/**/
bool PCIMemory::PCI_AddWriteScatterRequest(VMMDLL_SCATTER_HANDLE hScatter, __int64 pAddress, LPVOID lResult, DWORD cbSize)
{
	if (!pHandle)
		return PCI_FAILURE;

	return VMMDLL_Scatter_PrepareWrite(hScatter, pAddress, (PBYTE)lResult, cbSize);
}

/**/
bool PCIMemory::PCI_ExecuteReadScatterRequest(VMMDLL_SCATTER_HANDLE hScatter, int dwPID)
{
	if (!pHandle)
		return PCI_FAILURE;

	if (!VMMDLL_Scatter_ExecuteRead(hScatter))
		return false;

	if (!VMMDLL_Scatter_Clear(hScatter, dwPID, 0))
		return false;

	return true;
}

/**/
bool PCIMemory::PCI_ExecuteWriteScatterRequest(VMMDLL_SCATTER_HANDLE hScatter, int dwPID)
{
	if (!pHandle)
		return PCI_FAILURE;

	if (!VMMDLL_Scatter_Execute(hScatter))
		return false;

	if (!VMMDLL_Scatter_Clear(hScatter, dwPID, 0))
		return false;

	return true;
}

/**/
__int64 PCIMemory::PCI_ResolvePtrChain(int dwPID, __int64 baseAddr, DWORD offsets[], int count)
{
	if (!pHandle)
		return PCI_ERROR;

	__int64 result = baseAddr;
	for (int i = 0; i < count; i++)
	{
		result = Read<__int64>(dwPID, result);
		result += offsets[i];
	}

	return result;
}

/**/
bool PCIMemory::PCI_DumpModule(int dwPID, const char* modName, std::vector<char>& out)
{
	PVMMDLL_MAP_MODULEENTRY modEntry32;
	if (!VMMDLL_Map_GetModuleFromNameU(pHandle, dwPID, (LPSTR)modName, &modEntry32, 0))
		return PCI_FAILURE;

	__int64 modBase = modEntry32->vaBase;
	auto fileSize = modEntry32->cbFileSizeRaw;		//	file size
	auto imageSize = modEntry32->cbImageSize;		//	unpacked file size
	DWORD mSize = imageSize;						//	@TODO: image size tends to be different and will result in a read failure
	out.resize(mSize);

	PCI_ReadVirtualMemory(dwPID, modBase, out.data(), mSize);
	///	@TODO: sometimes memory will fail to read even though result will be valid. related to size
	//	if (!ReadVirtualMemory(dwPID, modBase, out.data(), mSize))
	//	{
	//		VMMDLL_MemFree(modEntry32);
	//		out.clear();
	//		printf("failed to read virtual memory -> 0x%llX\nfileSize: %d\nimageSize: %d\n", modBase, fileSize, imageSize);
	//		return false;
	//	}
	VMMDLL_MemFree(modEntry32);

	return PCI_SUCCESS;
}

/**/
bool PCIMemory::PCI_DumpBytes(int dwPID, __int64 lpAddress, DWORD cbSize, std::vector<char>& out)
{
	out.resize(cbSize);

	return PCI_ReadVirtualMemory(dwPID, lpAddress, out.data(), cbSize);
}

/**/
void PCIMemory::PCI_PrintSectionMemory(int dwPID, __int64 addr, DWORD cbSize)
{
	std::vector<char> outBytes;
	if (!PCI_DumpBytes(dwPID, addr, cbSize, outBytes))
		return;

	PrintSectionMemory(outBytes, addr);
}

/**/
bool PCIMemory::PCI_DumpSectionToFile(int dwPID, const char* fileName, __int64 addr, DWORD cbSize)
{
	std::vector<char> outBytes;
	if (!PCI_DumpBytes(dwPID, addr, cbSize, outBytes))
		return PCI_FAILURE;

	char buffer[MAX_PATH];
	DWORD czSize = GetCurrentDirectoryA(MAX_PATH, buffer);
	std::string dir = buffer;
	dir += "\\dumps\\";
	CreateDirectoryA(dir.c_str(), 0);
	dir += fileName;


	auto handle = CreateFileA(dir.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!handle)
		return PCI_FAILURE;

	DWORD lpBytesWritten;
	return WriteFile(handle, outBytes.data(), outBytes.size(), &lpBytesWritten, NULL) && lpBytesWritten > 0;
}

/**/
bool PCIMemory::PCI_DumpModuleToFile(int dwPID, const char* modName)
{
	char buffer[MAX_PATH];
	DWORD czSize = GetCurrentDirectoryA(MAX_PATH, buffer);
	std::string dir = buffer;
	dir += "\\dumps\\";
	CreateDirectoryA(dir.c_str(), 0);
	dir += modName;
	return PCI_DumpModuleToFileA(dwPID, dir.c_str(), modName);
}

/**/
bool PCIMemory::PCI_DumpModuleToFileA(int dwPID, const char* path, const char* modName)
{
	std::vector<char> bytes;
	if (!PCI_DumpModule(dwPID, modName, bytes))
		return PCI_FAILURE;

	auto handle = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!handle)
		return PCI_FAILURE;

	DWORD lpBytesWritten;
	return WriteFile(handle, bytes.data(), bytes.size(), &lpBytesWritten, NULL) && lpBytesWritten > 0;
}

//-----------------------------------------------------------------------------
//	- PCIMemory - Tools
//-----------------------------------------------------------------------------

/**/
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

/**/
bool PCIMemory::MapSectionMemory(char* wxBytes, LPVOID& outData, DWORD cbSize)
{
	static DWORD cProcID = GetCurrentProcessId();
	HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, cProcID);
	if (hProcess == NULL)
		return PCI_FAILURE;

	outData = VirtualAllocEx(hProcess, NULL, cbSize, MEM_COMMIT, PAGE_READWRITE);
	if (outData == NULL)
	{
		CloseHandle(hProcess);
		return PCI_FAILURE;
	}

	SIZE_T bytesWritten;
	if (!WriteProcessMemory(hProcess, outData, wxBytes, cbSize, &bytesWritten))
	{
		VirtualFreeEx(hProcess, outData, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return PCI_FAILURE;
	}
	CloseHandle(hProcess);
	return PCI_SUCCESS;
}

/**/
bool PCIMemory::FreeMapSection(LPVOID pData, DWORD cbSize)
{
	static DWORD cProcID = GetCurrentProcessId();
	HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, cProcID);
	if (hProcess == NULL)
		return PCI_FAILURE;

	VirtualFreeEx(hProcess, pData, 0, MEM_RELEASE);
	CloseHandle(hProcess);
	return PCI_SUCCESS;
}


//-----------------------------------------------------------------------------
//	- PCIMemory - Class Instance Functions
//-----------------------------------------------------------------------------

/**/
DWORD PCIMemory::GetProcID() 
{ 
	if (!vmProcess.dwProcID)
		return PCI_ERROR;

	return vmProcess.dwProcID;
}

/**/
__int64 PCIMemory::GetModuleBase() 
{
	if (!vmProcess.dwProcID)
		return PCI_ERROR;

	return vmProcess.dwModBase;
}

/**/
__int64 PCIMemory::GetModuleBase(const char* name)
{
	if (!vmProcess.dwProcID)
		return PCI_ERROR;

	PCI_GetModuleBase(vmProcess.dwProcID, name);
}

/**/
__int64 PCIMemory::GetProcPEB()
{
	if (!vmProcess.dwProcID)
		return PCI_ERROR;

	return vmProcess.dwPEB;
}

/**/
bool PCIMemory::GetProcInfo(VMMDLL_PROCESS_INFORMATION& result)
{
	if (!vmProcess.dwProcID)
		return PCI_FAILURE;

	return PCI_GetProcInfo(vmProcess.dwProcID, result);
}

/**/
VMMDLL_SCATTER_HANDLE PCIMemory::GetScatterHandle()
{
	if (!vmProcess.dwProcID)
		return PCI_FAILURE;

	return PCI_CreateScatterHandle(vmProcess.dwProcID);
}

/**/
bool PCIMemory::ClearScatterHandle(VMMDLL_SCATTER_HANDLE hScatter, DWORD flags)
{
	if (!vmProcess.dwProcID)
		return PCI_FAILURE;

	return PCI_ClearScatterHandle(hScatter, vmProcess.dwProcID, flags);
}

/**/
void PCIMemory::CloseScatterHandle(VMMDLL_SCATTER_HANDLE hScatter)
{
	if (!vmProcess.dwProcID)
		return;

	return PCI_CloseScatterHandle(hScatter);
}

/**/
bool PCIMemory::ReadVirtualMemory(__int64 pAddress, LPVOID lResult, DWORD cbSize)
{
	if (!vmProcess.dwProcID)
		return PCI_FAILURE;

	return PCI_ReadVirtualMemory(vmProcess.dwProcID, pAddress, lResult, cbSize);
}

/**/
bool PCIMemory::ReadVirtualMemoryEx(__int64 pAddress, LPVOID lResult, DWORD cbSize)
{
	if (!vmProcess.dwProcID)
		return PCI_FAILURE;

	return PCI_ReadVirtualMemoryEx(vmProcess.dwProcID, pAddress, lResult, cbSize);
}

/**/
bool PCIMemory::WriteVirtualMemory(__int64 pAddress, LPVOID lPatch, DWORD cbSize)
{
	if (!vmProcess.dwProcID)
		return PCI_FAILURE;

	return PCI_WriteVirtualMemory(vmProcess.dwProcID, pAddress, lPatch, cbSize);
}

/**/
__int64 PCIMemory::ResolvePtrChain(__int64 baseAddr, DWORD offsets[], int count)
{
	if (!vmProcess.dwProcID)
		return PCI_ERROR;

	return PCI_ResolvePtrChain(vmProcess.dwProcID, baseAddr, offsets, count);
}

/**/
bool PCIMemory::RequestReadScatter(VMMDLL_SCATTER_HANDLE hScatter, __int64 pAddress, LPVOID lResult, DWORD cbSize)
{
	if (!vmProcess.dwProcID)
		return PCI_FAILURE;

	return PCI_AddReadScatterRequest(hScatter, pAddress, lResult, cbSize);
}

/**/
bool PCIMemory::RequestWriteScatter(VMMDLL_SCATTER_HANDLE hScatter, __int64 pAddress, LPVOID lResult, DWORD cbSize)
{
	if (!vmProcess.dwProcID)
		return PCI_FAILURE;

	return PCI_AddWriteScatterRequest(hScatter, pAddress, lResult, cbSize);
}

/**/
bool PCIMemory::ExecuteReadScatter(VMMDLL_SCATTER_HANDLE hScatter)
{
	if (!vmProcess.dwProcID)
		return PCI_FAILURE;

	return PCI_ExecuteReadScatterRequest(hScatter, vmProcess.dwProcID);
}

/**/
bool PCIMemory::ExecuteWriteScatter(VMMDLL_SCATTER_HANDLE hScatter)
{
	if (!vmProcess.dwProcID)
		return PCI_FAILURE;

	return PCI_ExecuteWriteScatterRequest(hScatter, vmProcess.dwProcID);
}