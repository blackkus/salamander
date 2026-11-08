# -*- coding: utf-8 -*-
# T13 compile fixes round 2.
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

# --- SalamanderInputLib.cpp: GetSlots without capturing lambda, GetMappings() ---
patch('SalamanderInputLib.cpp', [
    ('''	static TArray<FBindSlot> Slots;
	if (Slots.Num() == 0)
	{
		auto Add = [&Slots](const TCHAR* Slot, const TCHAR* ActionPath, FKey DefaultKey, FKey AltKey)
		{
			FBindSlot S;
			S.Slot = FName(Slot);
			S.ActionPath = ActionPath;
			S.DefaultKey = DefaultKey;
			S.AltKey = AltKey;
			Slots.Add(S);
		};
		// AZERTY: forward=W->Z, left=A->Q (back=S and right=D are the same keys on AZERTY).
		Add(TEXT("Forward"), TEXT("/Game/Input/Actions/IA_Move"), EKeys::W, EKeys::Z);
		Add(TEXT("Back"), TEXT("/Game/Input/Actions/IA_Move"), EKeys::S, EKeys::Invalid);
		Add(TEXT("Left"), TEXT("/Game/Input/Actions/IA_Move"), EKeys::A, EKeys::Q);
		Add(TEXT("Right"), TEXT("/Game/Input/Actions/IA_Move"), EKeys::D, EKeys::Invalid);
		Add(TEXT("Jump"), TEXT("/Game/Input/Actions/IA_Jump"), EKeys::SpaceBar, EKeys::Invalid);
		Add(TEXT("Sprint"), TEXT("/Game/Input/Actions/IA_Sprint"), EKeys::LeftShift, EKeys::Invalid);
		Add(TEXT("Crouch"), TEXT("/Game/Input/Actions/IA_Crouch"), EKeys::LeftControl, EKeys::Invalid);
		Add(TEXT("Attack"), TEXT("/Game/Input/Actions/IA_Attack"), EKeys::LeftMouseButton, EKeys::Invalid);
	}
	return Slots;''',
     '''	static TArray<FBindSlot> Slots;
	if (Slots.Num() == 0)
	{
		struct FSlotDef { const TCHAR* Slot; const TCHAR* ActionPath; FKey DefaultKey; FKey AltKey; };
		const FSlotDef Defs[] = {
			// AZERTY: forward=W->Z, left=A->Q (back=S and right=D are the same keys on AZERTY).
			{ TEXT("Forward"), TEXT("/Game/Input/Actions/IA_Move"), EKeys::W, EKeys::Z },
			{ TEXT("Back"),    TEXT("/Game/Input/Actions/IA_Move"), EKeys::S, EKeys::Invalid },
			{ TEXT("Left"),    TEXT("/Game/Input/Actions/IA_Move"), EKeys::A, EKeys::Q },
			{ TEXT("Right"),   TEXT("/Game/Input/Actions/IA_Move"), EKeys::D, EKeys::Invalid },
			{ TEXT("Jump"),    TEXT("/Game/Input/Actions/IA_Jump"), EKeys::SpaceBar, EKeys::Invalid },
			{ TEXT("Sprint"),  TEXT("/Game/Input/Actions/IA_Sprint"), EKeys::LeftShift, EKeys::Invalid },
			{ TEXT("Crouch"),  TEXT("/Game/Input/Actions/IA_Crouch"), EKeys::LeftControl, EKeys::Invalid },
			{ TEXT("Attack"),  TEXT("/Game/Input/Actions/IA_Attack"), EKeys::LeftMouseButton, EKeys::Invalid },
		};
		for (const FSlotDef& D : Defs)
		{
			FBindSlot S;
			S.Slot = FName(D.Slot);
			S.ActionPath = D.ActionPath;
			S.DefaultKey = D.DefaultKey;
			S.AltKey = D.AltKey;
			Slots.Add(S);
		}
	}
	return Slots;'''),
    ('	for (const FEnhancedActionKeyMapping& M : IMC->Mappings)',
     '	for (const FEnhancedActionKeyMapping& M : IMC->GetMappings())'),
])

# --- SLoginWidget: keep STextBlock per slot instead of SButton::SetText ---
patch('SLoginWidget.h', [
    ('	TMap<FName, TSharedPtr<SButton>> SlotButtons;',
     '	TMap<FName, TSharedPtr<STextBlock>> SlotTexts;'),
])
patch('SLoginWidget.cpp', [
    ('''		TSharedPtr<SButton> Btn;
		KeyBox->AddSlot()
			.AutoHeight()
			.Padding(2.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f)[ Label(SlotName.ToString()) ]
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.f)
				[
					SAssignNew(Btn, SButton)
					.Text(FText::FromString(USalamanderInputLib::GetBoundKey(Slot).GetFName().ToString()))
					.OnClicked(this, &SLoginWidget::OnSlotClicked, SlotName)
				]
			];
		SlotButtons.Add(SlotName, Btn);''',
     '''		TSharedPtr<STextBlock> KeyText;
		KeyBox->AddSlot()
			.AutoHeight()
			.Padding(2.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f)[ Label(SlotName.ToString()) ]
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.f)
				[
					SNew(SButton)
					.OnClicked(this, &SLoginWidget::OnSlotClicked, SlotName)
					[
						SAssignNew(KeyText, STextBlock)
						.Text(FText::FromString(USalamanderInputLib::GetBoundKey(Slot).GetFName().ToString()))
					]
				]
			];
		SlotTexts.Add(SlotName, KeyText);'''),
    ('''		if (TSharedPtr<SButton>* Btn = SlotButtons.Find(Slot.Slot))
		{
			(*Btn)->SetText(FText::FromString(USalamanderInputLib::GetBoundKey(Slot).GetFName().ToString()));
		}''',
     '''		if (TSharedPtr<STextBlock>* Txt = SlotTexts.Find(Slot.Slot))
		{
			(*Txt)->SetText(FText::FromString(USalamanderInputLib::GetBoundKey(Slot).GetFName().ToString()));
		}'''),
])

print('patch_t13c OK')
