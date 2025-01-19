# PCIMemory
PCIMemory is a C++ utility class for advanced memory manipulation tasks on on a process running on another machine. It leverages the LeechCore library and supports reading, writing, scanning memory as well as managing process and module information. The class offers both static and instance based operations for maximum flexibility.

## Features
- Attach and Detach from processes.
- Read , Write & Scan Process Memory.
- Enumerate Modules and Module Sections in a process.

---

## Getting Started

### Prerequisites

- 2x Machines running windows 10 or newer
- A modern C++ compiler
- PCIe FPGA Hardware
- VMMDLL & LeechCore libraries

### Installation

1. Clone the repository
```bash 
git clone https://github.com/NightFyre/PCIMemory.git
```

2. Link LeechCore and VMMDLL libraries 

3. include PCIMemory header in your project
```cpp
#include "PCIMemory.hpp"
```

## Usage
```cpp
const char* procName = "notepad.exe";

PCIMemory mem = PCIMemory();
DWORD procID = PCIMemory::GetProcID(procName);
__int64 dwModuleBase = PCIMemory::GetModuleBase(procID, procName);
```

## BUILD INSTRUCTIONS
The following static libraries are not included and need to be placed in the "libs" directory. The files can be found in the PCILeech reposititory maintained by ufrisk.
- leechcore.lib
- vmm.lib

Additionally, the following dynamic libraries will need to be sourced and placed in thh same directory as the compiled executable or the program will not run
- FTD3XX.dll
- leechcore.dll
- vmm.dll

### Performance Considerations
- Instance methods are optimized for scenarios where a process is frequently accessed.
- Static methods are ideal for one-off operations without the need to maintain a persistent state.
- Only one handle to VMMDLL can be used at a time for VMMDLL operations

## CREDITS
- [LeechCore](https://github.com/ufrisk/LeechCore)