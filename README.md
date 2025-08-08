# A ProxyDLL Template for version.dll

This project provides a simple template for creating a ProxyDLL for `version.dll`. It is designed to patch opcodes in a target executable at runtime.

## How to Use

Follow these steps to use the template for your own projects:

1. Open the `dllmain.cpp` file.

2. Locate the section dedicated to opcode patching. It looks like this:

```cpp
//-------------------------------------------------------------------------------
// Patch Opcodes (Change assembly code at runtime)
// Only "targetOffset" and "patchBytes" variables should be modified.

// Example: version_proxy_test.exe
DWORD targetOffset = 0xBA7; // FileOffset in the target executable
BYTE patchBytes[] = { 0xE9, 0xD6, 0xFE, 0xFF, 0xFF }; // The bytes to write

WriteOpCodesToExe(targetOffset, patchBytes, sizeof(patchBytes));
//-------------------------------------------------------------------------------
```

3. Modify the targetOffset variable with the file offset of the instruction you want to patch in your target executable.

4. Modify the patchBytes array with the new opcode bytes you wish to write.

5. Compile the project to generate the version.dll file.

6. Place the resulting version.dll in the same directory as your target executable.

7. When the executable starts, the DLL will be loaded and the WriteOpCodesToExe function will automatically apply your patch.


## Example Use Case
```cpp
// Bypass license check (NOP the verification call)
DWORD targetOffset = 0x4012A3;
BYTE patchBytes[] = { 0x90, 0x90, 0x90, 0x90, 0x90 }; // 5-byte NOP
```
