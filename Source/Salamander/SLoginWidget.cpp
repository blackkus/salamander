#include "SLoginWidget.h"

#include "SMenuPanel.h"
#include "SalamanderGameMode.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void SLoginWidget::Construct(const FArguments& InArgs)
{
	TSharedRef<SMenuPanel> Panel = SNew(SMenuPanel);
	Panel->OnConnect = [this](const FString& Ip, const FString& Nick, const FString& Color)
	{
		if (GameMode.IsValid())
		{
			GameMode->ConnectToServer(Ip, Nick, Color);
		}
	};

	ChildSlot
	[
		SNew(SBorder)
		.Padding(24.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(4.f)
			[
				SNew(STextBlock).Text(FText::FromString("SALAMANDER"))
			]
			+ SVerticalBox::Slot().AutoHeight()[ Panel ]
		]
	];
}
