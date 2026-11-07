#include "SalamanderMaterialLib.h"

#if WITH_EDITOR
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "DA_DeathRules.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"
#include "UObject/UObjectGlobals.h"

UMaterialExpressionVectorParameter* USalamanderMaterialLib::CreateVectorParameterNode(UMaterial* Material, FName ParamName, FLinearColor DefaultValue)
{
	if (Material == nullptr)
	{
		return nullptr;
	}

	UMaterialExpressionVectorParameter* Node = NewObject<UMaterialExpressionVectorParameter>(Material);
	Node->ParameterName = ParamName;
	Node->DefaultValue = DefaultValue;
	Node->MaterialExpressionEditorX = -600;
	Node->MaterialExpressionEditorY = 0;
	Material->GetExpressionCollection().AddExpression(Node);
	Material->PostEditChange();
	Material->MarkPackageDirty();
	return Node;
}
#endif


#if WITH_EDITOR
UDA_DeathRules* USalamanderMaterialLib::EnsureDeathRulesAsset()
{
	UDA_DeathRules* DA = LoadObject<UDA_DeathRules>(nullptr, TEXT("/Game/Data/DA_DeathRules"));
	if (DA != nullptr)
	{
		return DA;
	}
	UPackage* Pkg = CreatePackage(TEXT("/Game/Data"));
	DA = NewObject<UDA_DeathRules>(Pkg, UDA_DeathRules::StaticClass(), TEXT("DA_DeathRules"), RF_Public | RF_Standalone);
	DA->RespawnDelay = 5.f; // PROVISOIRE
	DA->MarkPackageDirty();
	const FString FilePath = FPackageName::LongPackageNameToFilename(TEXT("/Game/Data/DA_DeathRules"), FPackageName::GetAssetPackageExtension());
	UPackage::Save(Pkg, DA, *FilePath, FSavePackageArgs());
	return DA;
}
#endif

#if WITH_EDITOR
template <typename T>
static T* EnsureDataAsset(const TCHAR* Path, const TCHAR* Name)
{
	T* Asset = LoadObject<T>(nullptr, Path);
	if (Asset != nullptr)
	{
		return Asset;
	}
	UPackage* Pkg = CreatePackage(TEXT("/Game/Data"));
	Asset = NewObject<T>(Pkg, T::StaticClass(), Name, RF_Public | RF_Standalone);
	Asset->MarkPackageDirty();
	const FString FilePath = FPackageName::LongPackageNameToFilename(Path, FPackageName::GetAssetPackageExtension());
	UPackage::Save(Pkg, Asset, *FilePath, FSavePackageArgs());
	return Asset;
}

UDA_MeleeAttackStats* USalamanderMaterialLib::EnsureMeleeStatsAsset()
{
	return EnsureDataAsset<UDA_MeleeAttackStats>(TEXT("/Game/Data/DA_MeleeAttackStats"), TEXT("DA_MeleeAttackStats"));
}

UDA_EnemyStats* USalamanderMaterialLib::EnsureEnemyStatsAsset()
{
	return EnsureDataAsset<UDA_EnemyStats>(TEXT("/Game/Data/DA_EnemyStats"), TEXT("DA_EnemyStats"));
}
#endif
