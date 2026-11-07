#include "GE_InitStats.h"

#include "SalamanderAttributeSet.h"

UGE_InitStats::UGE_InitStats()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo HealthMod;
	HealthMod.Attribute = USalamanderAttributeSet::GetHealthAttribute();
	HealthMod.ModifierOp = EGameplayModOp::Override;
	HealthMod.ModifierMagnitude = FScalableFloat(100.f);
	Modifiers.Add(HealthMod);

	FGameplayModifierInfo MaxHealthMod;
	MaxHealthMod.Attribute = USalamanderAttributeSet::GetMaxHealthAttribute();
	MaxHealthMod.ModifierOp = EGameplayModOp::Override;
	MaxHealthMod.ModifierMagnitude = FScalableFloat(100.f);
	Modifiers.Add(MaxHealthMod);
}
