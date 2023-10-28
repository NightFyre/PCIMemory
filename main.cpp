#pragma once
#include "Memory.h"

#define PROCESS_NAME ""

//	EXAMPLE
int main()
{

	// INIT
	PCIMemory mem = PCIMemory(PROCESS_NAME);	//	Memory.cpp ~28

	// PID
	DWORD dwPID = PCIMemory::GetProcID();



}