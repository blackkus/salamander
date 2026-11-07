#include "GE_Damage.h"

#include "GameplayTagContainer.h"
#include "SalamanderAttributeSet.h"
#include "AbilitySystemComponent.h"

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


void UGE_Damage::ApplyTo(UAbilitySystemComponent* TargetASC, float Amount)
{
	if (TargetASC == nullptr)
	{
		return;
	}
	// Set the magnitude on the GE default object, then make the spec (debug/ability path).
	if (UGE_Damage* Def = Cast<UGE_Damage>(UGE_Damage::StaticClass()->GetDefaultObject()))
	{
		for (FGameplayModifierInfo& Mod : Def->Modifiers)
		{
			if (Mod.Attribute == USalamanderAttributeSet::GetHealthAttribute())
			{
				Mod.ModifierMagnitude = FScalableFloat(-Amount);
			}
		}
	}
	FGameplayEffectContextHandle Ctx = TargetASC->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = TargetASC->MakeOutgoingSpec(UGE_Damage::StaticClass(), 1.f, Ctx);
	if (Spec.IsValid())
	{
		TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}
