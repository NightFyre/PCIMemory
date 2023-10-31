#pragma once
#include <iostream>
#include <vector>
#include <string>

//	External Libraries
#include <leechcore.h>
#include <vmmdll.h>

#define PROCESS_NAME ""								//
inline VMM_HANDLE pHandle{ nullptr };				//	obtained on PCIMemory class construction

struct PCIProcess
{
	DWORD			dwProcID;
	__int64			dwModBase;
	__int64			dwPEB;
	std::string		dwProcName;
	VMMDLL_PROCESS_INFORMATION vmProcessInfo;
};
inline PCIProcess vmProcess;						//	generated on PCIMemory class construction

// Argument values for function returns
#define PCI_SUCCESS 1
#define PCI_FAILURE 0
#define PCI_ERROR -1

class PCIMemory
{
public:

	/*
	*	[RAW FUNCTIONS]
	* Contains most of the heavy logic and must be utilized with the default constructor
	* 
	* 
	*/
	static void				PCI_Init(LPSTR* args, DWORD cType, VMM_HANDLE& vmHandle);
	static void				PCI_InitProcess(LPSTR* args, DWORD cType, const char* procName);
	static bool				PCI_GetProcessID(const char* name, DWORD& dwPID);
	static __int64			PCI_GetModuleBase(int dwPID, const char* name);
	static __int64			PCI_GetProcPEB(int dwPID);
	static __int64			PCI_GetProcAddress(int dwPID, const char* name, const char* fn);
	static bool				PCI_GetProcDirectory(int dwPID, const char* modName, std::string& out);
	static bool				PCI_GetProcInfo(int dwPID, VMMDLL_PROCESS_INFORMATION& result);
	static bool				PCI_ReadVirtualMemory(int dwPID, __int64 pAddress, LPVOID lResult, DWORD cbSize);
	static bool				PCI_ReadVirtualMemoryEx(int dwPID, __int64 pAddress, LPVOID lResult, DWORD cbSize);
	static bool				PCI_WriteVirtualMemory(int dwPID, __int64 pAddress, LPVOID patch, DWORD cbSize);
	static __int64			PCI_ResolvePtrChain(int dwPID, __int64 base, DWORD offsets[], int count);
	static bool				PCI_DumpModule(int dwPID, const char* modName, std::vector<char>& out);
	static bool				PCI_DumpBytes(int dwPID, __int64 lpAddress, DWORD cbSize, std::vector<char>& out);
	static bool				PCI_DumpSectionToFile(int dwPID, const char* fileName, __int64 addr, DWORD cbSize);
	static bool				PCI_DumpModuleToFile(int dwPID, const char* modName);
	static bool				PCI_DumpModuleToFileA(int dwPID, const char* path, const char* modName);
	static void				PCI_PrintSectionMemory(int dwPID, __int64 addr, DWORD cbSize);

	template<typename T>
	static T PCI_Read(int dwPID, __int64 address, DWORD cbSize)
	{
		T result;
		PCI_ReadVirtualMemory(dwPID, address, &result, cbSize);
		return result;
	}

	template<typename T>
	static T PCI_Read(int dwPID, __int64 address)
	{
		T result{ 0 };
		PCI_ReadVirtualMemory(dwPID, address, &result, sizeof(result));
		return result;
	}

	template<typename T>
	static T PCI_ReadEx(int dwPID, __int64 address)
	{
		T result;
		PCI_ReadVirtualMemoryEx(dwPID, address, &result, sizeof(result));
		return result;
	}

	template<typename T>
	static bool PCI_Write(int dwPID, __int64 address, T patch, DWORD cbSize)
	{
		return PCI_WriteVirtualMemory(dwPID, address, &patch, cbSize);
	}

	template<typename T>
	static bool PCI_Write(int dwPID, __int64 address, T patch)
	{
		return PCI_WriteVirtualMemory(dwPID, address, &patch, sizeof(patch));
	}

	/*
	*
	*/
	static void				PrintSectionMemory(std::vector<char> bytes, __int64 addr);
	static bool				MapSectionMemory(char* xBytes, LPVOID& pOut, DWORD cbSize);
	static bool				FreeMapSection(LPVOID pData, DWORD cbSize);

	/**/
	static DWORD			GetProcID();
	static __int64			GetModuleBase();
	static __int64			GetModuleBase(const char* name);
	static __int64			GetProcPEB();
	static bool				GetProcInfo(VMMDLL_PROCESS_INFORMATION& result);
	static bool				ReadVirtualMemory(__int64 pAddress, LPVOID lResult, DWORD cbSize);
	static bool				ReadVirtualMemoryEx(__int64 pAddress, LPVOID lResult, DWORD cbSize);
	static bool				WriteVirtualMemory(__int64 pAddress, LPVOID patch, DWORD cbSize);
	static __int64			ResolvePtrChain(__int64 base, DWORD offsets[], int count);

	template<typename T>
	static T Read(__int64 address, DWORD cbSize)
	{
		T result;
		ReadVirtualMemory(address, &result, cbSize);
		return result;
	}

	template<typename T>
	static T Read(__int64 address)
	{
		T result;
		ReadVirtualMemory(address, &result, sizeof(result));
		return result;
	}

	template<typename T>
	static T ReadEx(__int64 address, DWORD cbSize)
	{
		T result;
		ReadVirtualMemoryEx(address, &result, cbSize);
		return result;
	}

	template<typename T>
	static T ReadEx(__int64 address)
	{
		T result;
		ReadVirtualMemoryEx(address, &result, sizeof(result));
		return result;
	}

	template<typename T>
	static bool Write(__int64 address, T patch, DWORD cbSize)
	{
		return WriteVirtualMemory(address, &patch, cbSize);
	}

	template<typename T>
	static bool Write(__int64 address, T patch)
	{
		return WriteVirtualMemory(address, &patch, sizeof(patch));
	}

	/*
	* 
	 * Calls VMMDLL_Initialize with standard params and stores the returned handle for library functionality
	 * NOTE: static variables other than VMMDLL_HANDLE are not initialized, library funcationality is not diminished in any way
	 * this method is mainly for debugging multiple processes without needing to declare a process name
	 * should prevent being confused on which process is being accessed with subsequent library calls
	*/
	PCIMemory();

	/*
	 * Calls VMMDLL_Initialize with standard params and stores the returned handle for library functionality
	 * - Process ID is stored
	 * - Process ModuleBase is Obtained and stored
	 * - VMMDLL_PROCESS_INFORMATION structure is obtained and stored
	 * 
	*/
	PCIMemory(const char* procName);

	/*
		* NOTE: this is an advanced method of initializing a class instance 
	 * Calls VMMDLL_Initialize with custom params and stores the returned handle for library functionality
	 * - Process ID is stored
	 * - Process ModuleBase is Obtained and stored
	 * - VMMDLL_PROCESS_INFORMATION structure is obtained and stored
	 *
	*/
	PCIMemory(LPSTR* args, const char* procName, DWORD mType = 3);

	/*
	 *	Deconstructor is automatically handled when the intializing instance is out of scope.
	 * 
	 *
	*/
	~PCIMemory();
};