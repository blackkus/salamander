#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class ASalamanderGameMode;
class SEditableTextBox;

// Ecran de connexion (map Menu) : IP + pseudo + couleur + bouton. 100% Slate, aucun Blueprint.
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
