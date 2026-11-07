#include "GE_Damage.h"

#include "GameplayTagContainer.h"
#include "SalamanderAttributeSet.h"

UGE_Damage::UGE_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo Mod;
	Mod.Attribute = USalamanderAttributeSet::GetHealthAttribute();
	Mod.ModifierOp = EGameplayModOp::Additive;
	FSetByCallerFloat Mag;
	Mag.DataTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"), false); // no error if not registered yet
	Mod.ModifierMagnitude = Mag;
	Modifiers.Add(Mod);
}
