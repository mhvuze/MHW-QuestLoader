#include "ghidra_export.h"
#include "dll.h"
#include "loader.h"

using namespace loader;

static int next_id = 0;
CreateHook(MH::EmSetter::CreateMonster, SpawnMonster, void, void* this_ptr, void* unkn, void* ptr, char flag)
{
	int monster_id = *(int*)((char*)this_ptr + 0x168);
	unsigned int subspecies_override = *(int*)((char*)this_ptr + 0x11c);
	if (subspecies_override == 0xCDCDCDCD) 
		subspecies_override = 0;
	else
		next_id = subspecies_override;

	return original(this_ptr, unkn, ptr, flag);
}

CreateHook(MH::Monster::ctor, ConstructMonster, void*, void* this_ptr, unsigned int monster_id, unsigned int variant)
{
	if (next_id) {
		LOG(INFO) << "Setting Subspecies : " << monster_id << ":" << next_id;
		variant = next_id;
		next_id = 0;
	}
	LOG(INFO) << "Creating Monster : " << monster_id << "-" << variant << " @0x" << this_ptr;
	return original(this_ptr, monster_id, variant);
}

void InjectSubspeciesLoader()
{
	if (!ConfigFile.value("enableSubspeciesLoader", true)) return;
	QueueHook(SpawnMonster);
	QueueHook(ConstructMonster);
}
