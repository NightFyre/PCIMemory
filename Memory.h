#pragma once
#include <leechcore.h>
#include <vmmdll.h>
#include <vector>

class PCIMemory
{
public:

	/*
	 *
	 *
	*/
	static bool				InitProcess(const char* procName);
	
	/*
	 *
	 *
	*/
	static DWORD			GetProcID();
	static bool				GetProcID(const char* name, int& dwPID);
	
	/*
	 *
	 *
	*/
	static bool				GetProcInfo(int dwPID, VMMDLL_PROCESS_INFORMATION& result);
	static bool				GetProcInfo(VMMDLL_PROCESS_INFORMATION& result);
	
	/*
	 *
	 *
	*/
	static __int64			GetModuleBase();
	static __int64			GetModuleBase(const char* name);
	static __int64			GetModuleBase(int dwPID, const char* name);
	
	/*
	 *
	 *
	*/
	static __int64			GetProcPEB();
	static __int64			GetProcPEB(int dwPID);
	
	/*
	 *
	 *
	*/
	static bool				ReadVirtualMemory(__int64 pAddress, LPVOID lResult, DWORD cbSize);
	static bool				ReadVirtualMemory(int dwPID, __int64 pAddress, LPVOID lResult, DWORD cbSize);

	/*
	 *
	 *
	*/
	static bool				WriteVirtualMemory(__int64 pAddress, LPVOID patch, DWORD cbSize);
	static bool				WriteVirtualMemory(int dwPID, __int64 pAddress, LPVOID patch, DWORD cbSize);
	
	/*
	 *
	 *
	*/
	static __int64			ResolvePtrChain(__int64 base, DWORD offsets[], int count);
	static __int64			ResolvePtrChain(int dwPID, __int64 base, DWORD offsets[], int count);

	/*
	 *
	 *
	*/
	static bool				MapSectionMemory(char* xBytes, LPVOID& pOut, DWORD cbSize);

	/*
	 *
	 *
	*/
	static bool				FreeMapSection(LPVOID pData, DWORD cbSize);

	/*
	 *
	 *
	*/
	static bool				DumpModule(int dwPID, const char* modName, std::vector<char>& out);
	static bool				DumpBytes(int dwPID, __int64 lpAddress, DWORD cbSize, std::vector<char>& out);

	/*
	 *
	 *
	*/
	static void				PrintSectionMemory(int dwPID, __int64 addr, DWORD cbSize);
	static void				PrintSectionMemory(std::vector<char> bytes, __int64 addr);


	/*
	 *
	 *
	*/
	template<typename T>
	static T Read(int dwPID, __int64 address)
	{
		T result{ 0 };
		ReadVirtualMemory(dwPID, address, &result, sizeof(result));
		return result;
	}
	
	/*
	 *
	 *
	*/
	template<typename T>
	static bool Write(int dwPID, __int64 address, T patch)
	{
		return WriteVirtualMemory(dwPID, address, patch, sizeof(patch));
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
	 *
	 *
	*/
	~PCIMemory();
};