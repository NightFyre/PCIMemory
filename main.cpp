#pragma once
#include "Memory.h"

//	EXAMPLE
int main()
{
	// INIT
	//	-	NOTE: Initializing PCIMemory with a process name will populate a static structure 'vmProcess' containing basic process information
	PCIMemory mem = PCIMemory(PROCESS_NAME);	//	Memory.cpp ~16
	auto procInfo = vmProcess;	//	copy of 'vmProcess' structure, use to compare with the following calls

	// PID
	//	- Returns vmProcess structure Process ID
	DWORD dwPID = PCIMemory::GetProcID();	

	// MODULE
	//	- Returns vmProcess structure Module Base Address
	__int64 dwMODULE = PCIMemory::GetModuleBase();
	
	// RESOURCE MODULE
	//	- Returns module base address for ntdll module in the selected process ( if found )
	__int64 dwNTDLL = PCIMemory::GetModuleBase("ntdll.dll");

	//	PROCESS PEB
	//	- returns process peb address which is stored in the vmProcess structure
	__int64 dwPEB = PCIMemory::GetProcPEB();

	return EXIT_SUCCESS;	//	PCIMemory should automaticall deconstuct
}