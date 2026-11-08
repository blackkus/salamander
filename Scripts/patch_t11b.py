# -*- coding: utf-8 -*-
# T11 fixes: default Health/MaxHealth = 100 (avoid death-at-login), looping boss-damage test timer with diagnostics.
import io

base = 'C:/Users/Blackkus/Documents/Unreal Projects/Salamander/Source/Salamander/'

# 1) AttributeSet: default values 100 so a fresh ASC never reads 0 HP before GE_InitStats.
p = base + 'SalamanderAttributeSet.h'
s = io.open(p, encoding='utf-8').read()
old = 'public:\n\t// Current HP. Any change goes through a GameplayEffect (server).\n\tUPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Health, Category="Attributes")\n\tFGameplayAttributeData Health;'
new = 'public:\n\tUSalamanderAttributeSet();\n\n\t// Current HP. Any change goes through a GameplayEffect (server).\n\tUPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Health, Category="Attributes")\n\tFGameplayAttributeData Health;'
assert old in s, 'attr header anchor 1'
s = s.replace(old, new)
io.open(p, 'w', encoding='utf-8', newline='').write(s)

p = base + 'SalamanderAttributeSet.cpp'
s = io.open(p, encoding='utf-8').read()
old = 'void USalamanderAttributeSet::GetLifetimeReplicatedProps'
new = ('USalamanderAttributeSet::USalamanderAttributeSet()\n'
       '{\n'
       '\t// Defaults: a fresh attribute set must not read 0 HP (would trigger the death path before GE_InitStats).\n'
       '\tHealth.SetBaseValue(100.f);\n'
       '\tHealth.SetCurrentValue(100.f);\n'
       '\tMaxHealth.SetBaseValue(100.f);\n'
       '\tMaxHealth.SetCurrentValue(100.f);\n'
       '}\n\n'
       'void USalamanderAttributeSet::GetLifetimeReplicatedProps')
assert old in s, 'attr cpp anchor'
s = s.replace(old, new, 1)
io.open(p, 'w', encoding='utf-8', newline='').write(s)

# 2) Boss damage test timer: loop + diagnostics.
p = base + 'SalamanderGameMode.cpp'
s = io.open(p, encoding='utf-8').read()
old = '''		const int32 Amount = BossDamage;
		FTimerHandle BH;
		GetWorldTimerManager().SetTimer(BH, [this, Amount]()
		{
			for (TActorIterator<ABossCharacter> It(GetWorld()); It; ++It)
			{
				if (UAbilitySystemComponent* BossASC = It->AbilitySystemComponent)
				{
					UGE_Damage::ApplyTo(BossASC, static_cast<float>(Amount));
					UE_LOG(LogTemp, Log, TEXT("[GameMode] TestAutoBossDamage %d on %s"), Amount, *It->GetName());
				}
			}
		}, 12.f, false);'''
new = '''		const int32 Amount = BossDamage;
		UE_LOG(LogTemp, Log, TEXT("[GameMode] TestAutoBossDamage armed amount=%d"), Amount);
		FTimerHandle BH;
		GetWorldTimerManager().SetTimer(BH, [this, Amount]()
		{
			int32 Found = 0;
			for (TActorIterator<ABossCharacter> It(GetWorld()); It; ++It)
			{
				++Found;
				if (UAbilitySystemComponent* BossASC = It->AbilitySystemComponent)
				{
					UGE_Damage::ApplyTo(BossASC, static_cast<float>(Amount));
					UE_LOG(LogTemp, Log, TEXT("[GameMode] TestAutoBossDamage %d on %s"), Amount, *It->GetName());
				}
			}
			if (Found == 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("[GameMode] TestAutoBossDamage: no boss in world"));
			}
		}, 5.f, true);'''
assert old in s, 'gamemode anchor'
s = s.replace(old, new, 1)
io.open(p, 'w', encoding='utf-8', newline='').write(s)

print('patch_t11b OK')
