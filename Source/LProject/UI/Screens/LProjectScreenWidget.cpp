// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Screens/LProjectScreenWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/GameInstance.h"
#include "Flow/LProjectGameFlowSubsystem.h"
#include "UI/Style/LProjectWidgetStyle.h"

TSharedRef<SWidget> ULProjectScreenWidget::RebuildWidget()
{
	if (!RootOverlay && WidgetTree)
	{
		RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = RootOverlay;
		BuildScreen();
	}
	return Super::RebuildWidget();
}

ULProjectGameFlowSubsystem* ULProjectScreenWidget::Flow() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<ULProjectGameFlowSubsystem>();
	}
	return nullptr;
}

UImage* ULProjectScreenWidget::AddFullScreenImage(const TCHAR* TexName, FLinearColor Tint)
{
	UImage* Img = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	Img->SetBrush(LProjectUI::Brush(TexName, Tint));
	if (UOverlaySlot* OvSlot = RootOverlay->AddChildToOverlay(Img))
	{
		OvSlot->SetHorizontalAlignment(HAlign_Fill);
		OvSlot->SetVerticalAlignment(VAlign_Fill);
	}
	return Img;
}

UImage* ULProjectScreenWidget::AddImage(const TCHAR* TexName,
    FVector2D Size,
    FLinearColor Tint,
    EHorizontalAlignment H,
    EVerticalAlignment V,
    FMargin InPadding)
{
	UImage* Img = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	Img->SetBrush(LProjectUI::BrushSized(TexName, Size, Tint));
	Img->SetDesiredSizeOverride(Size);
	if (UOverlaySlot* OvSlot = RootOverlay->AddChildToOverlay(Img))
	{
		OvSlot->SetHorizontalAlignment(H);
		OvSlot->SetVerticalAlignment(V);
		OvSlot->SetPadding(InPadding);
	}
	return Img;
}

UTextBlock* ULProjectScreenWidget::MakeText(const FString& Str, int32 Size, FLinearColor Color, bool bBold) const
{
	UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	T->SetText(FText::FromString(Str));
	T->SetFont(LProjectUI::Font(Size, bBold));
	T->SetColorAndOpacity(FSlateColor(Color));
	return T;
}

UButton* ULProjectScreenWidget::MakeButton(const FString& Label, FVector2D Size)
{
	UButton* B = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());

	FButtonStyle Style;
	Style.Normal = LProjectUI::BrushSized(TEXT("Tex_Button_Normal"), Size);
	Style.Hovered = LProjectUI::BrushSized(TEXT("Tex_Button_Hover"), Size);
	Style.Pressed = LProjectUI::BrushSized(TEXT("Tex_Button_Pressed"), Size);
	Style.Disabled = Style.Normal;
	B->SetStyle(Style);

	UTextBlock* Lbl = MakeText(Label, 22, LProjectUI::ColText);
	if (UButtonSlot* BSlot = Cast<UButtonSlot>(B->AddChild(Lbl)))
	{
		BSlot->SetHorizontalAlignment(HAlign_Center);
		BSlot->SetVerticalAlignment(VAlign_Center);
	}
	return B;
}

UVerticalBox* ULProjectScreenWidget::AddCenterColumn(EVerticalAlignment V, FMargin InPadding)
{
	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	if (UOverlaySlot* OvSlot = RootOverlay->AddChildToOverlay(Box))
	{
		OvSlot->SetHorizontalAlignment(HAlign_Center);
		OvSlot->SetVerticalAlignment(V);
		OvSlot->SetPadding(InPadding);
	}
	return Box;
}
