#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class ASalamanderGameMode;
class SEditableTextBox;

// Login screen (Menu map): IP + pseudo + color + button. 100% Slate, no Blueprint.
class SALAMANDER_API SLoginWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLoginWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void SetGameMode(TWeakObjectPtr<ASalamanderGameMode> InGameMode) { GameMode = InGameMode; }

private:
	FReply OnConnectClicked();

	TWeakObjectPtr<ASalamanderGameMode> GameMode;
	TSharedPtr<SEditableTextBox> IpBox;
	TSharedPtr<SEditableTextBox> PseudoBox;
	TSharedPtr<SEditableTextBox> ColorBox;
};
