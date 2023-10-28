# PCIMemory
Description  
- x64 Release 
- No Binaries Included
- Example Project

## BUILD INSTRUCTIONS
The following static libraries are not included and need to be placed in the "libs" directory. The files can be found in the PCILeech reposititory maintained by ufrisk.
- leechcore.lib
- vmm.lib

Additionally, the following dynamic libraries will need to be sourced and placed in thh same directory as the compiled executable or the program will not run
- FTD3XX.dll
- leechcore.dll
- vmm.dll


## CODE EXAMPLES
//	
```cpp
const char* procName = "notepad.exe";

PCIMemory mem = PCIMemory();
DWORD procID = PCIMemory::GetProcID(procName);
__int64 dwModuleBase = PCIMemory::GetModuleBase(procID, procName);
```

## CREDITS
- [LeechCore](https://github.com/ufrisk/LeechCore)

## LICENSE

## DISCLAIMER