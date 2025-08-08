#include "pch.h"
#include <windows.h>
#include <ImageHlp.h>
#include <cstring> // memcpy için
#include <string>



typedef BOOL(WINAPI* GetFileVersionInfoA_Type)(LPCSTR, DWORD, DWORD, LPVOID);
typedef BOOL(WINAPI* GetFileVersionInfoW_Type)(LPCWSTR, DWORD, DWORD, LPVOID);
typedef BOOL(WINAPI* GetFileVersionInfoExW_Type)(DWORD, LPCWSTR, DWORD, DWORD, LPVOID);

typedef DWORD(WINAPI* GetFileVersionInfoSizeA_Type)(LPCSTR, LPDWORD);
typedef DWORD(WINAPI* GetFileVersionInfoSizeW_Type)(LPCWSTR, LPDWORD);
typedef DWORD(WINAPI* GetFileVersionInfoSizeExW_Type)(DWORD, LPCWSTR, LPDWORD);

typedef BOOL(WINAPI* VerQueryValueA_Type)(LPCVOID, LPCSTR, LPVOID*, PUINT);
typedef BOOL(WINAPI* VerQueryValueW_Type)(LPCVOID, LPCWSTR, LPVOID*, PUINT);

static GetFileVersionInfoA_Type GetFileVersionInfoA_0 = NULL;
static GetFileVersionInfoW_Type GetFileVersionInfoW_0 = NULL;
static GetFileVersionInfoExW_Type GetFileVersionInfoExW_0 = NULL;

static GetFileVersionInfoSizeA_Type GetFileVersionInfoSizeA_0 = NULL;
static GetFileVersionInfoSizeW_Type GetFileVersionInfoSizeW_0 = NULL;
static GetFileVersionInfoSizeExW_Type GetFileVersionInfoSizeExW_0 = NULL;

static VerQueryValueA_Type VerQueryValueA_0 = NULL;
static VerQueryValueW_Type VerQueryValueW_0 = NULL;

bool WriteOpCodesToExe(DWORD, const BYTE*, size_t);

void sub_fnLoad()
{
    // x64 de x86 DLL'i
    //C:\Windows\SysWOW64\version.dll -> x86
    // x64 de x64 DLL'i veya x86 da x86 DLL'i
    //C:\Windows\System32\version.dll -> x64

    BOOL Wow64Process;
    CHAR LibFileName[260];
    CHAR libVerName[] = "\\version.dll\0";

    IsWow64Process((HANDLE)0xFFFFFFFF, &Wow64Process);
    if (Wow64Process)
        GetSystemWow64DirectoryA(LibFileName, 0x104u);
    else
        GetSystemDirectoryA(LibFileName, 0x104u);    

    lstrcatA(LibFileName, libVerName);

    HMODULE libVer = LoadLibraryA(LibFileName);
    if (!libVer)
    {
        MessageBoxA(NULL, "Version.dll yüklenemedi!", "Hata", MB_OK);
        return;
    }

    /*if (libVer)
    {
        char pathBuffer[MAX_PATH];
        GetModuleFileNameA(libVer, pathBuffer, MAX_PATH);
        MessageBoxA(NULL, pathBuffer, "Yüklenen DLL", MB_OK);
    }*/

    GetFileVersionInfoA_0 = (GetFileVersionInfoA_Type)GetProcAddress(libVer, "GetFileVersionInfoA");
    GetFileVersionInfoW_0 = (GetFileVersionInfoW_Type)GetProcAddress(libVer, "GetFileVersionInfoW");
    GetFileVersionInfoExW_0 = (GetFileVersionInfoExW_Type)GetProcAddress(libVer, "GetFileVersionInfoExW");
    GetFileVersionInfoSizeA_0 = (GetFileVersionInfoSizeA_Type)GetProcAddress(libVer, "GetFileVersionInfoSizeA");
    GetFileVersionInfoSizeW_0 = (GetFileVersionInfoSizeW_Type)GetProcAddress(libVer, "GetFileVersionInfoSizeW");
    GetFileVersionInfoSizeExW_0 = (GetFileVersionInfoSizeExW_Type)GetProcAddress(libVer, "GetFileVersionInfoSizeExW");
    VerQueryValueA_0 = (VerQueryValueA_Type)GetProcAddress(libVer, "VerQueryValueA");
    VerQueryValueW_0 = (VerQueryValueW_Type)GetProcAddress(libVer, "VerQueryValueW");

    if (!GetFileVersionInfoA_0 || !GetFileVersionInfoW_0 || !GetFileVersionInfoExW_0 ||
        !GetFileVersionInfoSizeA_0 || !GetFileVersionInfoSizeW_0 || !GetFileVersionInfoSizeExW_0 ||
        !VerQueryValueA_0 || !VerQueryValueW_0)
    {
        MessageBoxA(NULL, "DLL FN baðlantý hatasý.", "hata", 0);
        return;
    }


    //-------------------------------------------------------------------------------
    // Opcode Patchleme (Assembly kodunu çalýþma anýnýnda deðiþtir)
    // yalnýzca "targetOffset" ve "patchBytes" deðiþken verileri deðiþtirilmeli.

    //version_proxy_test.exe
    DWORD targetOffset = 0xBA7; //hedef dosyada FileOffset
    BYTE patchBytes[] = { 0xE9 ,0xD6 ,0xFE ,0xFF ,0xFF }; //yazýlacak veri

    WriteOpCodesToExe(targetOffset, patchBytes, sizeof(patchBytes));
    //-------------------------------------------------------------------------------
}




bool WriteOpCodesToExe(DWORD fileOffset, const BYTE* opCodes, size_t opCodeSize) {
    // 1. EXE'nin base adresini al
    HMODULE hExeBase = GetModuleHandle(NULL);
    if (!hExeBase) {
        return false;
    }

    // 2. PE header'larýný parse et
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hExeBase;
    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hExeBase + pDosHeader->e_lfanew);

    // 3. Section header'larýný bul
    PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHeaders);
    bool found = false;

    for (WORD i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++, pSection++) {
        // 4. Fiziksel offset'in hangi section'da olduðunu bul
        if (fileOffset >= pSection->PointerToRawData &&
            fileOffset < (pSection->PointerToRawData + pSection->SizeOfRawData)) {
            found = true;
            break;
        }
    }

    //Hedef dosya ofseti hiçbir bölümde bulunamadý!
    if (!found) {
        return false;
    }

    // 5. Fiziksel offset'i sanal adrese çevir
    LPVOID patchAddress = (BYTE*)hExeBase +
        (fileOffset - pSection->PointerToRawData) + pSection->VirtualAddress;

    //std::cout << "Patching address: 0x" << std::hex << patchAddress << std::endl;

    // 6. Bellek korumasýný deðiþtir ve yazma iþlemi yap
    DWORD oldProtect;
    if (!VirtualProtect(patchAddress, opCodeSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return false;
    }

    // 7. Orijinal byte'larý yedekle
    //BYTE* originalBytes = new BYTE[opCodeSize];
    //memcpy(originalBytes, patchAddress, opCodeSize);

    // 8. Yeni OPCODE'larý yaz
    memcpy(patchAddress, opCodes, opCodeSize);

    // 9. Orijinal korumayý geri yükle
    VirtualProtect(patchAddress, opCodeSize, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), patchAddress, opCodeSize);

    //delete[] originalBytes;
    return true;
}



BOOL WINAPI ex_GetFileVersionInfoA(LPCSTR file, DWORD dwHandle, DWORD dwLen, LPVOID lpData) {
    return GetFileVersionInfoA_0(file, dwHandle, dwLen, lpData);
}

BOOL WINAPI ex_GetFileVersionInfoW(LPCWSTR file, DWORD dwHandle, DWORD dwLen, LPVOID lpData) {
    return GetFileVersionInfoW_0(file, dwHandle, dwLen, lpData);
}

BOOL WINAPI ex_GetFileVersionInfoExW(DWORD dwFlags, LPCWSTR file, DWORD dwHandle, DWORD dwLen, LPVOID lpData) {
    return GetFileVersionInfoExW_0(dwFlags, file, dwHandle, dwLen, lpData);
}

DWORD WINAPI ex_GetFileVersionInfoSizeA(LPCSTR file, LPDWORD handle) {
    return GetFileVersionInfoSizeA_0(file, handle);
}

DWORD WINAPI ex_GetFileVersionInfoSizeW(LPCWSTR file, LPDWORD handle) {
    return GetFileVersionInfoSizeW_0(file, handle);
}

DWORD WINAPI ex_GetFileVersionInfoSizeExW(DWORD dwFlags, LPCWSTR file, LPDWORD handle) {
    return GetFileVersionInfoSizeExW_0(dwFlags, file, handle);
}

BOOL WINAPI ex_VerQueryValueA(LPCVOID pBlock, LPCSTR lpSubBlock, LPVOID* lplpBuffer, PUINT puLen) {
    return VerQueryValueA_0(pBlock, lpSubBlock, lplpBuffer, puLen);
}

BOOL WINAPI ex_VerQueryValueW(LPCVOID pBlock, LPCWSTR lpSubBlock, LPVOID* lplpBuffer, PUINT puLen) {
    return VerQueryValueW_0(pBlock, lpSubBlock, lplpBuffer, puLen);
}




BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    //HANDLE to HMODULE -> static_cast<HMODULE>(hModule)
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule); //windows.h
        sub_fnLoad();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

