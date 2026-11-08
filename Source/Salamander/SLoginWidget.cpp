#include "SLoginWidget.h"

#include "SalamanderGameMode.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void SLoginWidget::Construct(const FArguments& InArgs)
{
	// Make sure saved bindings + AZERTY alt keys are applied to the mapping context.
	USalamanderInputLib::ApplyAllBindings();

	auto Label = [](const FString& T)
	{
		return SNew(STextBlock).Text(FText::FromString(T));
	};

	TSharedRef<SVerticalBox> KeyBox = SNew(SVerticalBox);
	for (const FBindSlot& Slot : USalamanderInputLib::GetSlots())
	{
		const FName SlotName = Slot.Slot;
		TSharedPtr<STextBlock> KeyText;
		KeyBox->AddSlot()
			.AutoHeight()
			.Padding(2.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f)[ Label(SlotName.ToString()) ]
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.f)
				[
					SNew(SButton)
					.OnClicked(this, &SLoginWidget::OnSlotClicked, SlotName)
					[
						SAssignNew(KeyText, STextBlock)
						.Text(FText::FromString(USalamanderInputLib::GetBoundKey(Slot).GetFName().ToString()))
					]
				]
			];
		SlotTexts.Add(SlotName, KeyText);
	}

	ChildSlot
	[
		SNew(SBorder)
		.Padding(24.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(4.f)[ Label("SALAMANDER") ]
				+ SVerticalBox::Slot().AutoHeight().Padding(4.f)[ Label("Server IP (ex 127.0.0.1 or server PC LAN IP)") ]
				+ SVerticalBox::Slot().AutoHeight().Padding(4.f)[ SAssignNew(IpBox, SEditableTextBox).Text(FText::FromString("127.0.0.1")) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(4.f)[ Label("Nick (16 chars max)") ]
				+ SVerticalBox::Slot().AutoHeight().Padding(4.f)[ SAssignNew(NickBox, SEditableTextBox) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(4.f)[ Label("Color hex (ex FF8800)") ]
				+ SVerticalBox::Slot().AutoHeight().Padding(4.f)[ SAssignNew(ColorBox, SEditableTextBox).Text(FText::FromString("FF8800")) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(12.f)
				[
					SNew(SButton)
					.Text(FText::FromString("Connect"))
					.HAlign(HAlign_Center)
					.OnClicked(this, &SLoginWidget::OnConnectClicked)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(12.f)[ Label("Key bindings (click a key button, then press the new key):") ]
				+ SVerticalBox::Slot().AutoHeight()[ KeyBox ]
				+ SVerticalBox::Slot().AutoHeight().Padding(8.f)[ SAssignNew(HintText, STextBlock).Text(FText::FromString("")) ]
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
			NickBox.IsValid() ? NickBox->GetText().ToString() : TEXT(""),
			ColorBox.IsValid() ? ColorBox->GetText().ToString() : TEXT("FF8800"));
	}
	return FReply::Handled();
}

FReply SLoginWidget::OnSlotClicked(const FName SlotName)
{
	PendingSlot = SlotName;
	if (HintText.IsValid())
	{
		HintText->SetText(FText::FromString(FString::Printf(TEXT("Press a key for %s..."), *SlotName.ToString())));
	}
	return FReply::Handled();
}

FReply SLoginWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (PendingSlot != NAME_None)
	{
		const FKey NewKey = InKeyEvent.GetKey();
		USalamanderInputLib::RebindSlot(PendingSlot, NewKey);
		PendingSlot = NAME_None;
		if (HintText.IsValid())
		{
			HintText->SetText(FText::FromString(TEXT("")));
		}
		RefreshSlotButtons();
		return FReply::Handled();
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

void SLoginWidget::RefreshSlotButtons()
{
	for (const FBindSlot& Slot : USalamanderInputLib::GetSlots())
	{
		if (TSharedPtr<STextBlock>* Txt = SlotTexts.Find(Slot.Slot))
		{
			(*Txt)->SetText(FText::FromString(USalamanderInputLib::GetBoundKey(Slot).GetFName().ToString()));
		}
	}
}
