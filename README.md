#A ProxyDLL Template for version.dll
This project provides a simple template for creating a ProxyDLL for version.dll. It is designed to patch opcodes in a target executable at runtime.

How to Use
Follow these steps to use the template for your own projects:

Open the dllmain.cpp file.

Locate the section dedicated to opcode patching. It looks like this:

C++

//-------------------------------------------------------------------------------
// Patch Opcodes (Change assembly code at runtime)
// Only "targetOffset" and "patchBytes" variables should be modified.

// Example: version_proxy_test.exe
DWORD targetOffset = 0xBA7; // FileOffset in the target executable
BYTE patchBytes[] = { 0xE9, 0xD6, 0xFE, 0xFF, 0xFF }; // The bytes to write

WriteOpCodesToExe(targetOffset, patchBytes, sizeof(patchBytes));
//-------------------------------------------------------------------------------

Modify the targetOffset variable with the file offset of the instruction you want to patch in your target executable.

Modify the patchBytes array with the new opcode bytes you wish to write.

Compile the project to generate the version.dll file.

Place the resulting version.dll in the same directory as your target executable.

When the executable starts, the DLL will be loaded and the WriteOpCodesToExe function will automatically apply your patch.

