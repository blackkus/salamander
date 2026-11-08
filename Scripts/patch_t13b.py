# -*- coding: utf-8 -*-
# T13 compile fixes.
import io
BASE = 'C:/Users/Blackkus/Documents/Unreal Projects/Salamander/Source/Salamander/'

def patch(path, pairs):
    p = BASE + path
    s = io.open(p, encoding='utf-8').read()
    for old, new in pairs:
        assert old in s, '%s: anchor not found: %r' % (path, old[:70])
        s = s.replace(old, new, 1)
    io.open(p, 'w', encoding='utf-8', newline='').write(s)
    print('patched', path)

patch('NameTagWidget.cpp', [
    ('#include "NameTagWidget.h"\n',
     '#include "NameTagWidget.h"\n\n#include "Blueprint/WidgetTree.h"\n#include "UObject/UObjectGlobals.h"\n'),
    ('WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);',
     'WidgetTree = NewObject<UWidgetTree>(this, UWidgetTree::StaticClass(), TEXT("WidgetTree"), RF_Transient);'),
    ('.WidgetStyle.BackgroundImage.Tint = FLinearColor(0.f, 0.f, 0.f, 0.55f);',
     '.WidgetStyle.BackgroundImage.TintColor = FLinearColor(0.f, 0.f, 0.f, 0.55f);'),
    ('.WidgetStyle.FillImage.Tint = FLinearColor(0.15f, 0.85f, 0.2f, 1.f);',
     '.WidgetStyle.FillImage.TintColor = FLinearColor(0.15f, 0.85f, 0.2f, 1.f);'),
    ('''	if (NameText.IsValid())
	{
		NameText->SetText(FText::FromString(InName));
	}''',
     '''	if (NameText != nullptr)
	{
		NameText->SetText(FText::FromString(InName));
	}'''),
    ('''	if (HealthBar.IsValid())
	{''',
     '''	if (HealthBar != nullptr)
	{'''),
    ('''		HealthBar->SetFillColorAndOpacity(C);
		HealthBar->WidgetStyle.FillImage.Tint = C;''',
     '''		HealthBar->SetFillColorAndOpacity(C);
		HealthBar->WidgetStyle.FillImage.TintColor = C;'''),
])

# GetGameplayAttributeValue needs 2 args in 5.8 -> use the AttributeSet accessors instead.
patch('EnemyCharacter.cpp', [
    ('''	if (AbilitySystemComponent != nullptr)
	{
		const float H = AbilitySystemComponent->GetGameplayAttributeValue(USalamanderAttributeSet::GetHealthAttribute());
		const float M = AbilitySystemComponent->GetGameplayAttributeValue(USalamanderAttributeSet::GetMaxHealthAttribute());
		return M > 0.f ? H / M : 1.f;
	}
	return 1.f;''',
     '''	if (AttributeSet != nullptr)
	{
		const float H = AttributeSet->GetHealth();
		const float M = AttributeSet->GetMaxHealth();
		return M > 0.f ? H / M : 1.f;
	}
	return 1.f;'''),
])

patch('SalamanderCharacter.cpp', [
    ('''		if (const UAbilitySystemComponent* ASC = PS->GetASC())
		{
			const float H = ASC->GetGameplayAttributeValue(USalamanderAttributeSet::GetHealthAttribute());
			const float M = ASC->GetGameplayAttributeValue(USalamanderAttributeSet::GetMaxHealthAttribute());
			return M > 0.f ? H / M : 1.f;
		}''',
     '''		if (const UAbilitySystemComponent* ASC = PS->GetASC())
		{
			if (const USalamanderAttributeSet* AS = Cast<USalamanderAttributeSet>(ASC->GetAttributeSet(USalamanderAttributeSet::StaticClass())))
			{
				const float H = AS->GetHealth();
				const float M = AS->GetMaxHealth();
				return M > 0.f ? H / M : 1.f;
			}
		}'''),
])

print('patch_t13b OK')
