#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InputCoreTypes.h"
#include "SalamanderInputLib.generated.h"

class UInputMappingContext;
class UInputAction;

// One rebindable slot in the main menu.
USTRUCT()
struct FBindSlot
{
	GENERATED_BODY()

	UPROPERTY()
	FName Slot; // config key, ex "Forward"

	UPROPERTY()
	FString ActionPath; // ex "/Game/Input/Actions/IA_Move"

	UPROPERTY()
	FKey DefaultKey;

	// AZERTY alternative key added next to the default (only for move forward/left).
	UPROPERTY()
	FKey AltKey;
};

// Key bindings: load/save in GameUserSettings.ini, apply to IMC_Default in memory.
UCLASS()
class SALAMANDER_API USalamanderInputLib : public UObject
{
	GENERATED_BODY()

public:
	// The rebindable slots shown in the menu.
	static const TArray<FBindSlot>& GetSlots();

	// Current key for a slot (saved binding wins over default).
	static FKey GetBoundKey(const FBindSlot& Slot);

	// Rebinds a slot to a new key (applies to IMC_Default at runtime + persists).
	static void RebindSlot(const FName& SlotName, const FKey& NewKey);

	// Applies all saved bindings + AZERTY alt keys to IMC_Default. Idempotent. Client side.
	static void ApplyAllBindings();
};
