#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class ASalamanderGameMode;

// Startup menu (Menu map): thin wrapper around the reusable SMenuPanel.
class SALAMANDER_API SLoginWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLoginWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void SetGameMode(TWeakObjectPtr<ASalamanderGameMode> InGameMode) { GameMode = InGameMode; }

private:
	TWeakObjectPtr<ASalamanderGameMode> GameMode;
};
