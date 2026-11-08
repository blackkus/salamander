#include "SalamanderInputLib.h"

#include "EnhancedInput/Public/InputMappingContext.h"
#include "EnhancedInput/Public/InputAction.h"
#include "Engine/Engine.h"
#include "GameFramework/InputSettings.h"
#include "Misc/Paths.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"

#define BINDINGS_SECTION TEXT("KeyBindings")
// Fixed-path ini so the menu process and the game process always agree (independent of Windows/WindowsEditor flavor).
static const FString& KeyBindingsIni()
{
	static const FString Path = FPaths::ProjectSavedDir() / TEXT("Config/KeyBindings.ini");
	return Path;
}

static const FString IMC_PATH = TEXT("/Game/Input/IMC_Default");

const TArray<FBindSlot>& USalamanderInputLib::GetSlots()
{
	static TArray<FBindSlot> Slots;
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
	return Slots;
}

FKey USalamanderInputLib::GetBoundKey(const FBindSlot& Slot)
{
	FString Saved;
	if (GConfig != nullptr && GConfig->GetString(BINDINGS_SECTION, *Slot.Slot.ToString(), Saved, KeyBindingsIni()) && !Saved.IsEmpty())
	{
		const FKey K(*Saved);
		if (K.IsValid())
		{
			return K;
		}
	}
	return Slot.DefaultKey;
}

static bool FindMapping(const UInputMappingContext* IMC, const UInputAction* Action, const FKey& Key, FEnhancedActionKeyMapping& Out)
{
	if (IMC == nullptr || Action == nullptr)
	{
		return false;
	}
	for (const FEnhancedActionKeyMapping& M : IMC->GetMappings())
	{
		if (M.Action == Action && M.Key == Key)
		{
			Out = M;
			return true;
		}
	}
	return false;
}

void USalamanderInputLib::RebindSlot(const FName& SlotName, const FKey& NewKey)
{
	if (!NewKey.IsValid())
	{
		return;
	}
	for (const FBindSlot& Slot : GetSlots())
	{
		if (Slot.Slot != SlotName)
		{
			continue;
		}
		UInputMappingContext* IMC = LoadObject<UInputMappingContext>(nullptr, *IMC_PATH);
		UInputAction* Action = LoadObject<UInputAction>(nullptr, *Slot.ActionPath);
		if (IMC == nullptr || Action == nullptr)
		{
			return;
		}
		FEnhancedActionKeyMapping ExistingNew;
		if (FindMapping(IMC, Action, NewKey, ExistingNew))
		{
			return; // new key already mapped
		}
		FEnhancedActionKeyMapping OldMapping;
		FKey FromKey = Slot.DefaultKey;
		if (!FindMapping(IMC, Action, FromKey, OldMapping))
		{
			// default key already rebound earlier: use the currently bound key
			FromKey = GetBoundKey(Slot);
			if (!FindMapping(IMC, Action, FromKey, OldMapping))
			{
				return;
			}
		}
		IMC->UnmapKey(Action, FromKey);
		FEnhancedActionKeyMapping& NewMapping = IMC->MapKey(Action, NewKey);
		NewMapping.Modifiers = OldMapping.Modifiers;
		NewMapping.Triggers = OldMapping.Triggers;

		if (GConfig != nullptr)
		{
			GConfig->SetString(BINDINGS_SECTION, *SlotName.ToString(), *NewKey.GetFName().ToString(), KeyBindingsIni());
			GConfig->Flush(false, KeyBindingsIni());
		}
		UE_LOG(LogTemp, Log, TEXT("[Input] rebound %s: %s -> %s"), *SlotName.ToString(), *FromKey.GetFName().ToString(), *NewKey.GetFName().ToString());
		return;
	}
}

void USalamanderInputLib::ApplyAllBindings()
{
	UInputMappingContext* IMC = LoadObject<UInputMappingContext>(nullptr, *IMC_PATH);
	if (IMC == nullptr)
	{
		return;
	}
	// Saved rebinds.
	for (const FBindSlot& Slot : GetSlots())
	{
		const FKey Bound = GetBoundKey(Slot);
		UE_LOG(LogTemp, Log, TEXT("[Input] slot %s bound=%s default=%s"), *Slot.Slot.ToString(), *Bound.GetFName().ToString(), *Slot.DefaultKey.GetFName().ToString());
		if (Bound != Slot.DefaultKey)
		{
			RebindSlot(Slot.Slot, Bound);
		}
	}
	// AZERTY alt keys (only when the slot is still on its default key).
	for (const FBindSlot& Slot : GetSlots())
	{
		if (!Slot.AltKey.IsValid())
		{
			continue;
		}
		if (GetBoundKey(Slot) != Slot.DefaultKey)
		{
			continue;
		}
		UInputAction* Action = LoadObject<UInputAction>(nullptr, *Slot.ActionPath);
		FEnhancedActionKeyMapping OldMapping;
		FEnhancedActionKeyMapping ExistingAlt;
		if (FindMapping(IMC, Action, Slot.DefaultKey, OldMapping) && !FindMapping(IMC, Action, Slot.AltKey, ExistingAlt))
		{
			FEnhancedActionKeyMapping& Alt = IMC->MapKey(Action, Slot.AltKey);
			Alt.Modifiers = OldMapping.Modifiers;
			Alt.Triggers = OldMapping.Triggers;
		}
	}
}

void USalamanderInputLib::EnsureDefaultMappings()
{
	UInputMappingContext* IMC = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Input/IMC_Default"));
	if (IMC == nullptr)
	{
		return;
	}
	struct FDef { const TCHAR* ActionPath; FKey Key; };
	const FDef Defs[] = {
		{ TEXT("/Game/Input/Actions/IA_Sprint"), EKeys::LeftShift },
		{ TEXT("/Game/Input/Actions/IA_Crouch"), EKeys::LeftControl },
		{ TEXT("/Game/Input/Actions/IA_Attack"), EKeys::LeftMouseButton },
	};
	for (const FDef& D : Defs)
	{
		UInputAction* Action = LoadObject<UInputAction>(nullptr, D.ActionPath);
		if (Action == nullptr)
		{
			continue;
		}
		bool bFound = false;
		for (const FEnhancedActionKeyMapping& M : IMC->GetMappings())
		{
			if (M.Action == Action && M.Key == D.Key) { bFound = true; break; }
		}
		if (bFound)
		{
			continue;
		}
		IMC->UnmapKey(Action, D.Key);
		FEnhancedActionKeyMapping& M = IMC->MapKey(Action, D.Key);
		M.Triggers.Add(NewObject<UInputTriggerPressed>(IMC));
		UE_LOG(LogTemp, Log, TEXT("[Input] baked into asset: %s -> %s"), *Action->GetName(), *D.Key.GetFName().ToString());
	}
	// Persist the asset so the mappings exist at load time (before any context is registered).
	UPackage* Pkg = IMC->GetOutermost();
	if (Pkg != nullptr)
	{
		Pkg->MarkPackageDirty();
		const FString FilePath = FPackageName::LongPackageNameToFilename(TEXT("/Game/Input/IMC_Default"), FPackageName::GetAssetPackageExtension());
		UPackage::Save(Pkg, IMC, *FilePath, FSavePackageArgs());
	}
}
