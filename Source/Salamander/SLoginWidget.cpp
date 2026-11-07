#include "SLoginWidget.h"

#include "SalamanderGameMode.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Framework/MultiBox/MultiBox.h"
#include "Widgets/Text/STextBlock.h"

void SLoginWidget::Construct(const FArguments& InArgs)
{

	auto Label = [](const FString& T)
	{
		return SNew(STextBlock).Text(FText::FromString(T));
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
			+ SVerticalBox::Slot().AutoHeight().Padding(4.f)[ Label("IP du serveur (ex 127.0.0.1 ou IP locale du PC serveur)") ]
			+ SVerticalBox::Slot().AutoHeight().Padding(4.f)[ SAssignNew(IpBox, SEditableTextBox).Text(FText::FromString("127.0.0.1")) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(4.f)[ Label("Pseudo (16 caracteres max)") ]
			+ SVerticalBox::Slot().AutoHeight().Padding(4.f)[ SAssignNew(PseudoBox, SEditableTextBox) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(4.f)[ Label("Couleur hex (ex FF8800)") ]
			+ SVerticalBox::Slot().AutoHeight().Padding(4.f)[ SAssignNew(ColorBox, SEditableTextBox).Text(FText::FromString("FF8800")) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(12.f)
			[
				SNew(SButton)
				.Text(FText::FromString("Se connecter"))
				.HAlign(HAlign_Center)
				.OnClicked(this, &SLoginWidget::OnConnectClicked)
			]
		]
	];
}

FReply SLoginWidget::OnConnectClicked()
{
	if (GameMode.IsValid())
	{
		GameMode->ConnectToServer(
			IpBox.IsValid() ? IpBox->GetText().ToString() : TEXT("127.0.0.1"),
			PseudoBox.IsValid() ? PseudoBox->GetText().ToString() : TEXT(""),
			ColorBox.IsValid() ? ColorBox->GetText().ToString() : TEXT("FF8800"));
	}
	return FReply::Handled();
}
