#include "Salamander.h"

#include "GameplayTagsManager.h"
#include "Modules/ModuleManager.h"

class FSalamanderModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		FDefaultGameModuleImpl::StartupModule();

		// Native gameplay tags (also declared in Config/DefaultGameplayTags.ini).
		UGameplayTagsManager& Mgr = UGameplayTagsManager::Get();
		Mgr.AddNativeGameplayTag(FName("Data.Damage"), TEXT("Damage amount (SetByCaller)"));
		Mgr.AddNativeGameplayTag(FName("State.Dead"), TEXT("Player is dead"));
		Mgr.AddNativeGameplayTag(FName("Ability.MeleeAttack"), TEXT("Melee attack ability"));
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FSalamanderModule, Salamander, "Salamander");
