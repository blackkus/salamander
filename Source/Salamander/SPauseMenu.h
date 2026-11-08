#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SMenuPanel;
class ASalamanderGameMode;

// Esc pause menu: modal, reuses SMenuPanel (IP/nick/color/key bindings) + Resume + Quit.
class SALAMANDER_API SPauseMenu : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPauseMenu) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	static void Show();
	static void Hide();
	static bool IsVisible();

private:
	FReply OnResumeClicked();
	FReply OnQuitClicked();

	static TSharedPtr<SPauseMenu> Instance;
	TSharedPtr<SMenuPanel> Panel;
};
