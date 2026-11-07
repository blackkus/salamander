#include "SalamanderMaterialLib.h"

#if WITH_EDITOR
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"
#include "Materials/MaterialExpressionVectorParameter.h"

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
