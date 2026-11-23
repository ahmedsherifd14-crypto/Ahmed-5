#include "UI/TLSInventoryWidget.h"
#include "Player/TLSCharacter.h"
#include "Player/TLSInventoryComponent.h"
#include "Components/TextBlock.h"

void UTLSInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (ATLSCharacter* Player = GetOwningPlayerPawn<ATLSCharacter>())
	{
		Player->Inventory->OnInventoryChanged.AddDynamic(this, &UTLSInventoryWidget::Refresh);
	}
	Refresh();
}

void UTLSInventoryWidget::NativeDestruct()
{
	if (ATLSCharacter* Player = GetOwningPlayerPawn<ATLSCharacter>())
	{
		Player->Inventory->OnInventoryChanged.RemoveDynamic(this, &UTLSInventoryWidget::Refresh);
	}
	Super::NativeDestruct();
}

void UTLSInventoryWidget::Refresh()
{
	const ATLSCharacter* Player = GetOwningPlayerPawn<ATLSCharacter>();
	if (!Player || !Player->Inventory)
	{
		return;
	}

	if (ItemsText)
	{
		FString Lines;
		for (const FTLSInventoryEntry& E : Player->Inventory->GetEntries())
		{
			const FText* Name = ItemDisplayNames.Find(E.ItemId);
			const FString NameStr = Name ? Name->ToString() : E.ItemId.ToString();
			Lines += FString::Printf(TEXT("%s  x%d\n"), *NameStr, E.Count);
		}
		if (Lines.IsEmpty())
		{
			Lines = TEXT("Empty");
		}
		ItemsText->SetText(FText::FromString(Lines));
	}

	if (JournalText)
	{
		FString Lines;
		for (const FName& Doc : Player->Inventory->GetDocuments())
		{
			const FText* Title = DocumentTitles.Find(Doc);
			Lines += FString::Printf(TEXT("- %s\n"), *(Title ? Title->ToString() : Doc.ToString()));
		}
		if (Lines.IsEmpty())
		{
			Lines = TEXT("No documents found yet.");
		}
		JournalText->SetText(FText::FromString(Lines));
	}
}
