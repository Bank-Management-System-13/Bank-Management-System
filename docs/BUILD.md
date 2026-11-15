# Build Instructions

## Requirements
- A C++ compiler with C++11 support or newer (g++ 9+ recommended)
- Git

## Clone the repository
```bash
git clone https://github.com/Bank-Management-System-13/Bank-Management-System.git
cd Bank-Management-System
```

## Compile
```bash
g++ -std=c++17 -o bank main.cpp
```

## Run
- Linux / macOS: `./bank`
- Windows (PowerShell): `.\bank.exe`

## Troubleshooting
- **`g++` not found**: install MinGW-w64 (Windows) or `build-essential` (Linux).
- **Compile errors**: make sure you are using `-std=c++17` or newer.