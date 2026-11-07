#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DA_DeathRules.h"
#include "SalamanderMaterialLib.generated.h"

UCLASS()
class SALAMANDER_API USalamanderMaterialLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	// Creates a named vector parameter node in a material (called from editor Python scripts,
	// because ParameterName/DefaultValue are not exposed to Python in 5.8).
	UFUNCTION(BlueprintCallable, Category="Salamander|Material")
	static class UMaterialExpressionVectorParameter* CreateVectorParameterNode(class UMaterial* Material, FName ParamName, FLinearColor DefaultValue);

	// Creates /Game/Data/DA_DeathRules if missing (PROVISOIRE RespawnDelay=5s).
	UFUNCTION(BlueprintCallable, Category="Salamander|Editor")
	static UDA_DeathRules* EnsureDeathRulesAsset();
#endif
};
