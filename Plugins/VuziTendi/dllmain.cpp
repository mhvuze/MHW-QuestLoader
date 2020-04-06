// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include <loader.h>

#include "ghidra_export.h"
#include "util.h"

#include <set>
#include <map>

using namespace loader;
static void* offsetPtr(void* ptr, int offset) { return offsetPtr<void>(ptr, offset); }

__declspec(dllexport) extern void showMessage(std::string message) {
	MH::Chat::ShowGameMessage(*(undefined**)MH::Chat::MainPtr, &message[0], -1, -1, 0);
}

CreateHook(MH::Monster::SoftenTimers::AddWoundTimer, AddPartTimer, void*, void* timerMgr, unsigned int index, float timerStart)
{
	auto ret = original(timerMgr, index, timerStart);
	*offsetPtr<float>(ret, 0xc) = 3000;
	LOG(INFO) << "Tenderize duration: " << 3000;
	return ret;
}

void onLoad()
{
	if (std::string(GameVersion) == "???") {
		LOG(ERR) << "VuziTendi: Targeting wrong game version.";
		return;
	}

	MH_Initialize();

	QueueHook(AddPartTimer);

	MH_ApplyQueued();
}


BOOL APIENTRY DllMain( HMODULE hModule,
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

