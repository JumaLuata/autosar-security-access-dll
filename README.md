# AutoSar Security Access DLL with SCons

## Project Overview

This project provides a secure access DLL implementation for AUTOSAR systems, built using the SCons build system. The DLL supports two security levels (0x01 and 0x11) with AES128 encryption, and can be compiled for both 32-bit and 64-bit Windows platforms.

## Features

- **Dual Security Levels**: Supports Level 0x01 (Basic) and Level 0x11 (Advanced)
- **AES128 Encryption**: Simplified cryptographic implementation (replace with production-grade crypto in real deployments)
- **Cross-Platform Build**: Compiles for both 32-bit (x86) and 64-bit (x64) architectures
- **SCons Build System**: Flexible and reliable build automation
- **MinGW-w64 Compatibility**: Tested with GCC toolchain on Windows

## Prerequisites

- **MinGW-w64** (both 32-bit and 64-bit toolchains)
  - Recommended installation via [MSYS2](https://www.msys2.org/):
    ```bash
    pacman -S mingw-w64-i686-toolchain mingw-w64-x86_64-toolchain
    ```
- **Python 3.x** (for SCons)
- **SCons** (install via pip):
  ```bash
  pip install scons
  ```

## Build Instructions

### 1. Clone the repository
```bash
git clone https://github.com/JumaLuata/autosar-security-access-dll.git
cd autosar-security-access-dll
```

### 2. Build options

| Command                  | Description                          |
|--------------------------|--------------------------------------|
| `scons --w32 -j 12`      | Build 32-bit version (x86)           |
| `scons --w64 -j 12`      | Build 64-bit version (x64)           |
| `scons -j 12`            | Build both versions                  |
| `scons -c`               | Clean build artifacts                |

### 3. Output files

Successful builds will generate:
```
build/
├── obj32/                    # 32-bit build artifacts
│   ├── AES128.o              # 32-bit object file
│   ├── SecAcc_Implementation.o
│   ├── autosar_secacc_32.dll # 32-bit output DLL
│   └── autosar_secacc_32.def # 32-bit export definitions
└── obj64/                    # 64-bit build artifacts
    ├── AES128.o              # 64-bit object file
    ├── SecAcc_Implementation.o
    ├── autosar_secacc_64.dll # 64-bit output DLL
    └── autosar_secacc_64.def # 64-bit export definitions
```

## API Documentation

### Key Functions

```c
// Initialize the security module
Std_ReturnType SecAcc_Init(const SecAcc_ConfigType* ConfigPtr);

// Generate a security key
Std_ReturnType SecAcc_GenerateKey(uint8_t securityLevel, 
                                const uint8_t* seed, 
                                uint8_t seedLength,
                                uint8_t* key, 
                                uint8_t* keyLength);

// Verify a security key
Std_ReturnType SecAcc_VerifyKey(uint8_t securityLevel,
                               const uint8_t* key,
                               uint8_t keyLength);
```

### Security Levels

| Level   | Description                |
|---------|----------------------------|
| 0x01    | Basic security operations  |
| 0x11    | Advanced security operations|

## Implementation Notes

1. **Cryptography**: This implementation uses a simplified AES128 algorithm. For production use:
   - Replace with a validated cryptographic library (OpenSSL, mbedTLS, etc.)
   - Consider hardware security modules (HSM) for key management

2. **Thread Safety**: The current implementation is not thread-safe. Add mutex locks if needed for concurrent access.

3. **Error Handling**: Basic error codes are provided. Extend for more detailed error reporting.

## Directory Structure

```
autosar-security-access-dll/
├── include/                  # Header files
│   ├── SecAcc_Interface.h    # Public API
│   └── Crypto_Types.h        # Common types
├── src/                      # Implementation
│   ├── SecAcc_Implementation.c
│   └── AES128.c
├── build/                    # Build outputs
├── SConstruct                # Build configuration
└── README.md                 # This file
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please submit pull requests or open issues for any bugs or feature suggestions.

## Support

For assistance, please contact [your email/contact info] or open an issue in the GitHub repository.