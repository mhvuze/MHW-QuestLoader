// dllmain.cpp : Defines the entry point for the DLL application.
// Very slightly adapted version of Stracker's Unbloat dll

#include "Windows.h"
#include <Psapi.h>

#include <algorithm>
#include <vector>

#include "loader.h"

using namespace loader;

__declspec(dllexport) void load() {};

typedef unsigned char byte;

std::vector<void*> scanmem(const std::vector<byte>& bytes)
{
    std::vector<void*> results;
    auto module = GetModuleHandle("MonsterHunterWorld.exe");
    if (module == nullptr) return results;

    MODULEINFO moduleInfo;
    if (!GetModuleInformation(GetCurrentProcess(), module, &moduleInfo, sizeof(moduleInfo)))
        return results;

    byte* startAddr = (byte*)module;
    byte* endAddr = startAddr + moduleInfo.SizeOfImage;
    byte* addr = startAddr;

    while (addr < endAddr)
    {
        MEMORY_BASIC_INFORMATION memInfo;
        if (!VirtualQuery(addr, &memInfo, sizeof(memInfo)) || memInfo.State != MEM_COMMIT || (memInfo.Protect & PAGE_GUARD))
            continue;
        byte* begin = (byte*)memInfo.BaseAddress;
        byte* end = begin + memInfo.RegionSize;


        byte* found = std::search(begin, end, bytes.begin(), bytes.end());
        while (found != end) {
            results.push_back(found);
            found = std::search(found + 1, end, bytes.begin(), bytes.end());
        }

        addr = end;
        memInfo = {};
    }

    return results;
}

bool apply(std::vector<byte> search, std::vector<byte> replace)
{
    auto results = scanmem(search);
    if (results.size() != 1)
        return false;
    void* found = results[0];
    memcpy(found, &replace[0], replace.size());
    return true;
}


std::vector<byte> rawSearchBytes = { 0x48, 0x8B, 0x04, 0xC1, 0xC3, 0x29, 0xD2, 0x49 };
std::vector<byte> rawReplaceBytes = { 0x90, 0x90, 0x90, 0x90, 0xC3, 0x29, 0xD2, 0x49 };

void onLoad()
{
    if (!apply(rawSearchBytes, rawReplaceBytes))
        LOG(ERR) << "UnblockForDummies : Unblocking failed";
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        onLoad();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
