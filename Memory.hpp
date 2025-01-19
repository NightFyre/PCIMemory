#pragma once
#include <iostream>
#include <vector>
#include <string>

//	External Libraries
#include <leechcore.h>
#include <vmmdll.h>

#ifdef _DEBUG
#else
#endif

#ifdef _WIN64
typedef unsigned __int64 i64_t;
#else
typedef unsigned int i64_t;
#endif

//	fwd declare helpers
std::string ToLower(const std::string& input);
std::string ToUpper(const std::string& input);
std::string ToString(const std::wstring& input);
std::wstring ToWString(const std::string& input);

#define KILOBYTE (1 * 1024)						// 1 KB = 1024 Bytes
#define MEGABYTE (1024 * 1024)					// 1 MB = 1024 KB
#define GIGABYTE (2048 * 1024)					// 1 GB = 1024 MB
#define TERABYTE (4096 * 1024)					// 1 TB = 1024 GB

#define PROCESS_NAME ""								//
inline VMM_HANDLE pHandle{ nullptr };				//	obtained on PCIMemory class construction

typedef struct PCIPROCESSINFO64
{
	DWORD			dwProcID;						//	
	i64_t			dwModuleBase;					//	
	i64_t			dwPEB;							//	
	i64_t			dwCR3;							//	
	std::string		dwProcName;						//	
	std::string		dwProcPath;						//	

	//	
	VMM_HANDLE		pHandle;						//	
	VMMDLL_PROCESS_INFORMATION vmProcessInfo;		//	
} PCIPROCESSINFO32, procInfo_t;
inline procInfo_t vmProcess;						//	generated on PCIMemory class construction

// Argument values for function returns
#define PCI_SUCCESS 1
#define PCI_FAILURE 0
#define PCI_ERROR -1

//	enum { PCI_ERROR = -1, PCI_FAILURE, PCI_SUCCESS };

class PCIMemory
{
	/*//--------------------------\\
			CONSTRUCTORS
	*/
public:
	/* Calls VMMDLL_Initialize with standard params and stores the returned handle for library functionality
	 * NOTE: static variables other than VMMDLL_HANDLE are not initialized, library funcationality is not diminished in any way
	 * this method is mainly for debugging multiple processes without needing to declare a process name
	 * should prevent being confused on which process is being accessed with subsequent library calls
	*/
	PCIMemory();

	/* Calls VMMDLL_Initialize with standard params and stores the returned handle for library functionality
	 * - Process ID is stored
	 * - Process ModuleBase is Obtained and stored
	 * - VMMDLL_PROCESS_INFORMATION structure is obtained and stored
	*/
	PCIMemory(const std::string& procName);

	/* Calls VMMDLL_Initialize with custom params and stores the returned handle for library functionality
	 * - Process ID is stored
	 * - Process ModuleBase is Obtained and stored
	 * - VMMDLL_PROCESS_INFORMATION structure is obtained and stored
	 * NOTE: this is an advanced method of initializing a class instance
	*/
	PCIMemory(LPSTR* args, const std::string& procName, DWORD argc = 3);

	/* Deconstructor is automatically handled when the intializing instance is out of scope. */
	~PCIMemory() noexcept;

public:

	/*
	*	[RAW FUNCTIONS]
	* Contains most of the heavy logic and must be utilized with the default constructor
	*/

	/* Initializes VMDLL library and returns a default handle for process operations
	* User must manually obtain process information for various operations
	*/
	static void				PCI_Init(LPSTR* args, DWORD argc, VMM_HANDLE& vmHandle);

	/* Initializes VMMDLL library and auto generates a process structure making it very simple to begin manipulating a specific process
	*/
	static void				PCI_InitProcess(LPSTR* args, DWORD argc, const std::string& procName);

public:

	/**/
	static bool				PCI_GetProcessID(const std::string& name, DWORD& dwPID);

	/**/
	static i64_t			PCI_GetModuleBase(int dwPID, const std::string& name);

	/**/
	static i64_t			PCI_GetProcPEB(int dwPID);

	/**/
	static i64_t			PCI_GetProcAddress(int dwPID, const std::string& name, const std::string& fn);

	/**/
	static bool				PCI_GetProcAddressEx(int dwPID, const std::string& modName, const std::string& exportName, i64_t* out);

	/**/
	static bool				PCI_GetProcDirectory(int dwPID, const std::string& modName, std::string& out);

	/**/
	static bool				PCI_GetProcInfo(int dwPID, VMMDLL_PROCESS_INFORMATION& result);

public:

	/**/
	static bool				PCI_ReadVirtualMemory(int dwPID, i64_t pAddress, LPVOID lResult, DWORD cbSize);

	/**/
	static bool				PCI_ReadVirtualMemoryEx(int dwPID, i64_t pAddress, LPVOID lResult, DWORD cbSize);

	/**/
	static bool				PCI_WriteVirtualMemory(int dwPID, i64_t pAddress, LPVOID patch, DWORD cbSize);

	/**/
	static i64_t			PCI_ResolvePtrChain(int dwPID, i64_t base, DWORD offsets[], int count);

public:

	/**/
	static VMMDLL_SCATTER_HANDLE PCI_CreateScatterHandle(int dwPID, DWORD dwFlags);

	/**/
	static bool				PCI_ClearScatterHandle(VMMDLL_SCATTER_HANDLE hScatter, int dwPID, DWORD flags);

	/**/
	static void				PCI_CloseScatterHandle(VMMDLL_SCATTER_HANDLE hScatter);

	/**/
	static bool				PCI_AddReadScatterRequest(VMMDLL_SCATTER_HANDLE hScatter, i64_t pAddress, LPVOID lResult, DWORD cbSize);

	/**/
	static bool				PCI_AddWriteScatterRequest(VMMDLL_SCATTER_HANDLE hScatter, i64_t pAddress, LPVOID lResult, DWORD cbSize);

	/**/
	static bool				PCI_ExecuteReadScatterRequest(VMMDLL_SCATTER_HANDLE hScatter, int dwPID);

	/**/
	static bool				PCI_ExecuteWriteScatterRequest(VMMDLL_SCATTER_HANDLE hScatter, int dwPID);

public:

	/**/
	static bool				PCI_DumpBytes(int dwPID, i64_t lpAddress, DWORD cbSize, std::vector<char>& out);

	/**/
	static bool				PCI_DumpModule(int dwPID, const std::string& modName, std::vector<char>& out);

	/**/
	static bool				PCI_DumpSectionToFile(int dwPID, const std::string& fileName, i64_t addr, DWORD cbSize);

	/**/
	static bool				PCI_DumpModuleToFile(int dwPID, const std::string& modName);

	/**/
	static bool				PCI_DumpModuleToFileA(int dwPID, const std::string& path, const std::string& modName);

	/**/
	static void				PCI_PrintSectionMemory(int dwPID, i64_t addr, DWORD cbSize);


	/**/
	template<typename T>
	static T PCI_Read(int dwPID, i64_t address, DWORD cbSize)
	{
		T result;
		PCI_ReadVirtualMemory(dwPID, address, &result, cbSize);
		return result;
	}


	/**/
	template<typename T>
	static T PCI_Read(int dwPID, i64_t address)
	{
		T result{ 0 };
		PCI_ReadVirtualMemory(dwPID, address, &result, sizeof(result));
		return result;
	}


	/**/
	template<typename T>
	static T PCI_ReadEx(int dwPID, i64_t address)
	{
		T result;
		PCI_ReadVirtualMemoryEx(dwPID, address, &result, sizeof(result));
		return result;
	}


	/**/
	template<typename T>
	static bool PCI_Write(int dwPID, i64_t address, T patch, DWORD cbSize)
	{
		return PCI_WriteVirtualMemory(dwPID, address, &patch, cbSize);
	}


	/**/
	template<typename T>
	static bool PCI_Write(int dwPID, i64_t address, T patch)
	{
		return PCI_WriteVirtualMemory(dwPID, address, &patch, sizeof(patch));
	}


	/**/
	static void				PrintSectionMemory(std::vector<char> bytes, i64_t addr);

	/**/
	static bool				MapSectionMemory(char* xBytes, LPVOID& pOut, DWORD cbSize);

	/**/
	static bool				FreeMapSection(LPVOID pData, DWORD cbSize);

public:
	/**/
	static DWORD			GetProcID();

	/**/
	static i64_t			GetModuleBase();

	/**/
	static i64_t			GetModuleBase(const std::string& name);

	/**/
	static i64_t			GetProcPEB();

	/**/
	static bool				GetProcInfo(VMMDLL_PROCESS_INFORMATION& result);

	/**/
	static VMMDLL_SCATTER_HANDLE GetScatterHandle(DWORD flags = VMMDLL_FLAG_NOCACHE);

	/**/
	static bool				ClearScatterHandle(VMMDLL_SCATTER_HANDLE hScatter, DWORD flags = VMMDLL_FLAG_NOCACHE);

	/**/
	static void				CloseScatterHandle(VMMDLL_SCATTER_HANDLE hScatter);

	/**/
	static bool				ReadVirtualMemory(i64_t pAddress, LPVOID lResult, DWORD cbSize);

	/**/
	static bool				ReadVirtualMemoryEx(i64_t pAddress, LPVOID lResult, DWORD cbSize);

	/**/
	static bool				WriteVirtualMemory(i64_t pAddress, LPVOID patch, DWORD cbSize);

	/**/
	static i64_t			ResolvePtrChain(i64_t base, DWORD offsets[], int count);

	/**/
	static bool				RequestReadScatter(VMMDLL_SCATTER_HANDLE hScatter, i64_t pAddress, LPVOID lResult, DWORD cbSize);

	/**/
	static bool				RequestWriteScatter(VMMDLL_SCATTER_HANDLE hScatter, i64_t pAddress, LPVOID lResult, DWORD cbSize);

	/**/
	static bool				ExecuteReadScatter(VMMDLL_SCATTER_HANDLE hScatter);

	/**/
	static bool				ExecuteWriteScatter(VMMDLL_SCATTER_HANDLE hScatter);


	/**/
	template<typename T>
	static T Read(i64_t address, DWORD cbSize)
	{
		T result;
		ReadVirtualMemory(address, &result, cbSize);
		return result;
	}


	/**/
	template<typename T>
	static T Read(i64_t address)
	{
		T result;
		ReadVirtualMemory(address, &result, sizeof(result));
		return result;
	}


	/**/
	template<typename T>
	static T ReadEx(i64_t address, DWORD cbSize)
	{
		T result;
		ReadVirtualMemoryEx(address, &result, cbSize);
		return result;
	}


	/**/
	template<typename T>
	static T ReadEx(i64_t address)
	{
		T result;
		ReadVirtualMemoryEx(address, &result, sizeof(result));
		return result;
	}


	/**/
	template<typename T>
	static bool Write(i64_t address, T patch, DWORD cbSize)
	{
		return WriteVirtualMemory(address, &patch, cbSize);
	}


	/**/
	template<typename T>
	static bool Write(i64_t address, T patch)
	{
		return WriteVirtualMemory(address, &patch, sizeof(patch));
	}
};


//-------------------------------------------------------------------------------------------------
//
//										CONSTRUCTORS
//
//-------------------------------------------------------------------------------------------------

PCIMemory::PCIMemory()
{
	LPSTR args[] = { (LPSTR)"", (LPSTR)"-device" , (LPSTR)"FPGA" };
	PCI_Init(args, 3, pHandle);
	printf("[+] PCIMemory::PCIMemory\n");
}

PCIMemory::PCIMemory(const std::string& procName)
{
	LPSTR args[] = { (LPSTR)"", (LPSTR)"-device" , (LPSTR)"FPGA" };
	PCI_InitProcess(args, 3, procName.c_str());

	auto test = Read<int>(vmProcess.dwModuleBase);
	printf("[+] PCIMemory::PCIMemory(%s)\n- PID:\t\t%d\n- MODULE:\t0x%llX\n- PEB:\t\t0x%llX\n- EMAGIC:\t0x%X\n\n",
		procName, vmProcess.dwProcID, vmProcess.dwModuleBase, vmProcess.dwPEB, test);
}

PCIMemory::PCIMemory(LPSTR* args, const std::string& procName, DWORD argc)
{
	PCI_InitProcess(args, argc, procName.c_str());

	auto test = Read<int>(vmProcess.dwModuleBase);
	printf("[+] PCIMemory::PCIMemory(%s)\n- PID:\t\t%d\n- MODULE:\t0x%llX\n- PEB:\t\t0x%llX\n- EMAGIC:\t0x%X\n\n",
		procName, argc, vmProcess.dwProcID, vmProcess.dwModuleBase, vmProcess.dwPEB, test);
}

PCIMemory::~PCIMemory()
{
	if (pHandle)
		VMMDLL_Close(pHandle);

	vmProcess = procInfo_t();

	printf("[-] PCIMemory::~PCIMemory\n");
}


//-------------------------------------------------------------------------------------------------
//
//										STATIC METHODS
//
//-------------------------------------------------------------------------------------------------

void PCIMemory::PCI_Init(LPSTR* args, DWORD argc, VMM_HANDLE& vmHandle)
{
	//	close any pre-existing handles
	if (vmHandle)
		VMMDLL_Close(vmHandle);

	//	initialize VMDLL
	vmHandle = VMMDLL_Initialize(argc, args);
}

void PCIMemory::PCI_InitProcess(LPSTR* args, DWORD argc, const std::string& procName)
{
	if (pHandle)
		VMMDLL_Close(pHandle);

	vmProcess.dwProcName = std::string(procName);

	//	Initialize VMDLL
	pHandle = VMMDLL_Initialize(argc, args);

	//	Get Process ID
	VMMDLL_PidGetFromName(pHandle, (LPSTR)vmProcess.dwProcName.c_str(), &vmProcess.dwProcID);

	//	Get Module Base
	vmProcess.dwModuleBase = PCI_GetModuleBase(vmProcess.dwProcID, vmProcess.dwProcName.c_str());

	// Get Process PEB
	vmProcess.dwPEB = PCI_GetProcPEB(vmProcess.dwProcID);

	//	Generate VMDLL_PROCESS_INFORMATION Struct
	PCI_GetProcInfo(vmProcess.dwProcID, vmProcess.vmProcessInfo);
}


//-------------------------------------------------------------------------------------------------
//
//										STATIC METHODS
//
//-------------------------------------------------------------------------------------------------

bool PCIMemory::PCI_GetProcessID(const std::string& name, DWORD& result)
{
	if (!pHandle)
		return PCI_FAILURE;

	return VMMDLL_PidGetFromName((VMM_HANDLE)pHandle, (LPSTR)name.c_str(), (PDWORD)&result);
}

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

i64_t PCIMemory::PCI_GetModuleBase(int dwPID, const std::string& name)
{
	if (!pHandle)
		return PCI_ERROR;

	return VMMDLL_ProcessGetModuleBaseU((VMM_HANDLE)pHandle, dwPID, (LPSTR)name.c_str());
}

i64_t PCIMemory::PCI_GetProcPEB(int dwPID)
{
	VMMDLL_PROCESS_INFORMATION procInfo{};
	if (!PCI_GetProcInfo(dwPID, procInfo))
		return PCI_ERROR;

	return procInfo.win.vaPEB;
}

i64_t PCIMemory::PCI_GetProcAddress(int dwPID, const std::string& name, const std::string& fn)
{
	if (!pHandle)
		return PCI_ERROR;

	return VMMDLL_ProcessGetProcAddressU(pHandle, dwPID, (LPSTR)name.c_str(), (LPSTR)fn.c_str());
}

bool PCIMemory::PCI_GetProcAddressEx(int dwPID, const std::string& modName, const std::string& exportName, i64_t* fn)
{
	bool bFound{ false };
	i64_t result{ 0 };

	PVMMDLL_MAP_EAT mEAT;
	if (!VMMDLL_Map_GetEAT(pHandle, dwPID, LPWSTR(ToWString(modName).c_str()), &mEAT))
	{
		VMMDLL_MemFree(mEAT);
		return PCI_FAILURE;
	}

	auto key = ToLower(exportName);

	for (int i = 0; i < mEAT->cMap; i++)
	{
		auto pEntry = mEAT->pMap[i];

		auto name = ToLower(ToString(pEntry.wszFunction));
		if (name != key)
			continue;

		bFound = true;
		result = pEntry.vaFunction;


		/*	MANUAL METHOD
			char nameBuff[64];
			auto nameRVA = pNames + (i * sizeof(DWORD));	//	address of name
			i64_t pName = map->vaModuleBase + PCIMemory::Read<DWORD>(nameRVA, sizeof(DWORD));
			if (!PCIMemory::ReadVirtualMemory(pName, &nameBuff, 64))
				continue;
		*/
	}
	VMMDLL_MemFree(mEAT);

	*fn = result;

	return bFound;
}

bool PCIMemory::PCI_GetProcDirectory(int dwPID, const std::string& modName, std::string& out)
{
	PVMMDLL_MAP_MODULEENTRY modEntry32{};
	if (!VMMDLL_Map_GetModuleFromNameU(pHandle, dwPID, LPSTR(modName.c_str()), &modEntry32, 0))
		return PCI_FAILURE;

	out = modEntry32->uszFullName;

	VMMDLL_MemFree(modEntry32);

	return PCI_SUCCESS;
}

bool PCIMemory::PCI_ReadVirtualMemory(int dwPID, i64_t pAddress, LPVOID lResult, DWORD cbSize)
{
	if (!pHandle)
		return PCI_FAILURE;

	return VMMDLL_MemRead(pHandle, dwPID, pAddress, (PBYTE)lResult, cbSize);
}

bool PCIMemory::PCI_ReadVirtualMemoryEx(int dwPID, i64_t pAddress, LPVOID lResult, DWORD cbSize)
{
	if (!pHandle)
		return PCI_FAILURE;

	uint32_t flags = VMMDLL_FLAG_NOCACHE | VMMDLL_FLAG_NOPAGING | VMMDLL_FLAG_ZEROPAD_ON_FAIL | VMMDLL_FLAG_NOPAGING_IO;
	return VMMDLL_MemReadEx(pHandle, dwPID, pAddress, (PBYTE)lResult, cbSize, nullptr, flags);
}

bool PCIMemory::PCI_WriteVirtualMemory(int dwPID, i64_t pAddress, LPVOID lPatch, DWORD cbSize)
{
	if (!pHandle)
		return PCI_FAILURE;

	return VMMDLL_MemWrite(pHandle, dwPID, pAddress, (PBYTE)lPatch, cbSize);
}

VMMDLL_SCATTER_HANDLE PCIMemory::PCI_CreateScatterHandle(int dwPID, DWORD dwFlags)
{
	if (!pHandle)
		return PCI_FAILURE;

	return VMMDLL_Scatter_Initialize(pHandle, dwPID, dwFlags);
}

bool PCIMemory::PCI_ClearScatterHandle(VMMDLL_SCATTER_HANDLE hScatter, int dwPID, DWORD flags)
{
	if (!pHandle)
		return PCI_FAILURE;

	return VMMDLL_Scatter_Clear(hScatter, dwPID, flags);
}

void PCIMemory::PCI_CloseScatterHandle(VMMDLL_SCATTER_HANDLE hScatter)
{
	VMMDLL_Scatter_CloseHandle(hScatter);
}

bool PCIMemory::PCI_AddReadScatterRequest(VMMDLL_SCATTER_HANDLE hScatter, i64_t pAddress, LPVOID lResult, DWORD cbSize)
{
	if (!pHandle)
		return PCI_FAILURE;

	return VMMDLL_Scatter_PrepareEx(hScatter, pAddress, cbSize, (PBYTE)lResult, 0);
}

bool PCIMemory::PCI_AddWriteScatterRequest(VMMDLL_SCATTER_HANDLE hScatter, i64_t pAddress, LPVOID lResult, DWORD cbSize)
{
	if (!pHandle)
		return PCI_FAILURE;

	return VMMDLL_Scatter_PrepareWrite(hScatter, pAddress, (PBYTE)lResult, cbSize);
}

bool PCIMemory::PCI_ExecuteReadScatterRequest(VMMDLL_SCATTER_HANDLE hScatter, int dwPID)
{
	if (!pHandle)
		return PCI_FAILURE;

	return VMMDLL_Scatter_ExecuteRead(hScatter);
}

bool PCIMemory::PCI_ExecuteWriteScatterRequest(VMMDLL_SCATTER_HANDLE hScatter, int dwPID)
{
	if (!pHandle)
		return PCI_FAILURE;

	return VMMDLL_Scatter_Execute(hScatter);
}

i64_t PCIMemory::PCI_ResolvePtrChain(int dwPID, i64_t baseAddr, DWORD offsets[], int count)
{
	if (!pHandle)
		return PCI_ERROR;

	i64_t result = baseAddr;
	for (int i = 0; i < count; i++)
	{
		result = Read<i64_t>(dwPID, result);
		result += offsets[i];
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
//
//										STATIC METHODS ( ADVANCED )
//
//-------------------------------------------------------------------------------------------------

bool PCIMemory::PCI_DumpModule(int dwPID, const std::string& modName, std::vector<char>& out)
{
	PVMMDLL_MAP_MODULEENTRY modEntry32;
	if (!VMMDLL_Map_GetModuleFromNameU(pHandle, dwPID, LPSTR(modName.c_str()), &modEntry32, 0))
		return PCI_FAILURE;

	i64_t modBase = modEntry32->vaBase;
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

bool PCIMemory::PCI_DumpBytes(int dwPID, i64_t lpAddress, DWORD cbSize, std::vector<char>& out)
{
	out.resize(cbSize);

	return PCI_ReadVirtualMemory(dwPID, lpAddress, out.data(), cbSize);
}

void PCIMemory::PCI_PrintSectionMemory(int dwPID, i64_t addr, DWORD cbSize)
{
	std::vector<char> outBytes;
	if (!PCI_DumpBytes(dwPID, addr, cbSize, outBytes))
		return;

	PrintSectionMemory(outBytes, addr);
}

bool PCIMemory::PCI_DumpSectionToFile(int dwPID, const std::string& fileName, i64_t addr, DWORD cbSize)
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

bool PCIMemory::PCI_DumpModuleToFile(int dwPID, const std::string& modName)
{
	char buffer[MAX_PATH];
	DWORD czSize = GetCurrentDirectoryA(MAX_PATH, buffer);
	std::string dir = buffer;
	dir += "\\dumps\\";
	CreateDirectoryA(dir.c_str(), 0);
	dir += modName;
	return PCI_DumpModuleToFileA(dwPID, dir.c_str(), modName);
}

bool PCIMemory::PCI_DumpModuleToFileA(int dwPID, const std::string& path, const std::string& modName)
{
	std::vector<char> bytes;
	if (!PCI_DumpModule(dwPID, modName, bytes))
		return PCI_FAILURE;

	auto handle = CreateFileA(path.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!handle)
		return PCI_FAILURE;

	DWORD lpBytesWritten;
	return WriteFile(handle, bytes.data(), bytes.size(), &lpBytesWritten, NULL) && lpBytesWritten > 0;
}

void PCIMemory::PrintSectionMemory(std::vector<char> bytes, i64_t addr)
{
	auto base = addr;
	for (int i = 0; i < bytes.size(); i++)
	{
		if (i % 8 == 0)
		{
			printf("\n[0x%llX][0x%X]:\t", (void*)((i64_t)base), (i));
			base += 0x8;
		}
		printf("%02X ", static_cast<unsigned char>(bytes[i]));
	}
}

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


//-------------------------------------------------------------------------------------------------
//
//										INSTANCE METHODS
//
//-------------------------------------------------------------------------------------------------

/**/
DWORD PCIMemory::GetProcID()
{
	if (!vmProcess.dwProcID)
		return PCI_ERROR;

	return vmProcess.dwProcID;
}

/**/
i64_t PCIMemory::GetModuleBase()
{
	if (!vmProcess.dwProcID)
		return PCI_ERROR;

	return vmProcess.dwModuleBase;
}

/**/
i64_t PCIMemory::GetModuleBase(const std::string& name)
{
	if (!vmProcess.dwProcID)
		return PCI_ERROR;

	PCI_GetModuleBase(vmProcess.dwProcID, name);
}

/**/
i64_t PCIMemory::GetProcPEB()
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
VMMDLL_SCATTER_HANDLE PCIMemory::GetScatterHandle(DWORD flags)
{
	if (!vmProcess.dwProcID)
		return PCI_FAILURE;

	return PCI_CreateScatterHandle(vmProcess.dwProcID, flags);
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
bool PCIMemory::ReadVirtualMemory(i64_t pAddress, LPVOID lResult, DWORD cbSize)
{
	if (!vmProcess.dwProcID)
		return PCI_FAILURE;

	return PCI_ReadVirtualMemory(vmProcess.dwProcID, pAddress, lResult, cbSize);
}

/**/
bool PCIMemory::ReadVirtualMemoryEx(i64_t pAddress, LPVOID lResult, DWORD cbSize)
{
	if (!vmProcess.dwProcID)
		return PCI_FAILURE;

	return PCI_ReadVirtualMemoryEx(vmProcess.dwProcID, pAddress, lResult, cbSize);
}

/**/
bool PCIMemory::WriteVirtualMemory(i64_t pAddress, LPVOID lPatch, DWORD cbSize)
{
	if (!vmProcess.dwProcID)
		return PCI_FAILURE;

	return PCI_WriteVirtualMemory(vmProcess.dwProcID, pAddress, lPatch, cbSize);
}

/**/
i64_t PCIMemory::ResolvePtrChain(i64_t baseAddr, DWORD offsets[], int count)
{
	if (!vmProcess.dwProcID)
		return PCI_ERROR;

	return PCI_ResolvePtrChain(vmProcess.dwProcID, baseAddr, offsets, count);
}

/**/
bool PCIMemory::RequestReadScatter(VMMDLL_SCATTER_HANDLE hScatter, i64_t pAddress, LPVOID lResult, DWORD cbSize)
{
	if (!vmProcess.dwProcID)
		return PCI_FAILURE;

	return PCI_AddReadScatterRequest(hScatter, pAddress, lResult, cbSize);
}

/**/
bool PCIMemory::RequestWriteScatter(VMMDLL_SCATTER_HANDLE hScatter, i64_t pAddress, LPVOID lResult, DWORD cbSize)
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

	bool result = PCI_ExecuteReadScatterRequest(hScatter, vmProcess.dwProcID);

	//	@TODO
	PCI_ClearScatterHandle(hScatter, vmProcess.dwProcID, VMMDLL_FLAG_NOCACHE);

	return result;
}

/**/
bool PCIMemory::ExecuteWriteScatter(VMMDLL_SCATTER_HANDLE hScatter)
{
	if (!vmProcess.dwProcID)
		return PCI_FAILURE;

	bool result = PCI_ExecuteWriteScatterRequest(hScatter, vmProcess.dwProcID);

	//	@TODO
	PCI_ClearScatterHandle(hScatter, vmProcess.dwProcID, VMMDLL_FLAG_NOCACHE);

	return result;
}


//-------------------------------------------------------------------------------------------------
//
// 									HELPER METHODS
//
//-------------------------------------------------------------------------------------------------

std::string ToLower(const std::string& input)
{
	std::string result;
	for (auto c : input)
		result += tolower(c);
	return result;
};

std::string ToUpper(const std::string& input)
{
	std::string result;
	for (auto c : input)
		result += toupper(c);
	return result;
};

std::string ToString(const std::wstring& input) { return std::string(input.begin(), input.end()); }

std::wstring ToWString(const std::string& input) { return std::wstring(input.begin(), input.end()); }