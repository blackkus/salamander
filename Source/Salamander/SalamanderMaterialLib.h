#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SalamanderMaterialLib.generated.h"

UCLASS()
class SALAMANDER_API USalamanderMaterialLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	// Crée un nœud paramètre vectoriel nommé dans un matériau (appelé depuis les scripts Python éditeur,
	// car ParameterName/DefaultValue ne sont pas exposés en Python en 5.8).
	UFUNCTION(BlueprintCallable, Category="Salamander|Material")
	static class UMaterialExpressionVectorParameter* CreateVectorParameterNode(class UMaterial* Material, FName ParamName, FLinearColor DefaultValue);
#endif
};
