#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "InputCoreTypes.h"
#include "SalamanderInputLib.h"

class ASalamanderGameMode;
class SEditableTextBox;
class SButton;
class STextBlock;

// Main menu (Menu map): server IP + nick + color + key bindings + connect. 100% Slate, no Blueprint.
class SALAMANDER_API SLoginWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLoginWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void SetGameMode(TWeakObjectPtr<ASalamanderGameMode> InGameMode) { GameMode = InGameMode; }

	// Captures the next pressed key while a rebind is pending.
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
	FReply OnConnectClicked();
	FReply OnSlotClicked(const FName SlotName);
	void RefreshSlotButtons();

	TWeakObjectPtr<ASalamanderGameMode> GameMode;
	TSharedPtr<SEditableTextBox> IpBox;
	TSharedPtr<SEditableTextBox> NickBox;
	TSharedPtr<SEditableTextBox> ColorBox;
	TSharedPtr<STextBlock> HintText;
	TMap<FName, TSharedPtr<STextBlock>> SlotTexts;
	FName PendingSlot = NAME_None;
};
