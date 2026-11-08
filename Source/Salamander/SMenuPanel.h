#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "InputCoreTypes.h"
#include "SalamanderInputLib.h"

class SEditableTextBox;
class SButton;
class STextBlock;

// Reusable settings panel: server IP + nick + color + key bindings + Connect.
// Used by the startup login menu and by the Esc pause menu.
class SALAMANDER_API SMenuPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMenuPanel) {}
	SLATE_END_ARGS()

	// Called with (Ip, Nick, Color) when the Connect button is clicked.
	TFunction<void(const FString&, const FString&, const FString&)> OnConnect;

	void Construct(const FArguments& InArgs);

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& InPointerEvent) override;

private:
	FReply OnConnectClicked();
	FReply OnSlotClicked(const FName SlotName);
	void RefreshSlotButtons();

	TSharedPtr<SEditableTextBox> IpBox;
	TSharedPtr<SEditableTextBox> NickBox;
	TSharedPtr<SEditableTextBox> ColorBox;
	TSharedPtr<STextBlock> HintText;
	TMap<FName, TSharedPtr<STextBlock>> SlotTexts;
	FName PendingSlot = NAME_None;
};
