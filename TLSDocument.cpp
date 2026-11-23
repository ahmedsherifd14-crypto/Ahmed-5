#include "Gameplay/TLSDocument.h"
#include "Player/TLSCharacter.h"
#include "Player/TLSInventoryComponent.h"
#include "Core/TLSGameInstance.h"
#include "Kismet/GameplayStatics.h"

FText ATLSDocument::GetInteractPrompt(const ATLSCharacter* Interactor) const
{
	return FText::FromString(TEXT("Read [E]"));
}

void ATLSDocument::OnInteract(ATLSCharacter* Interactor)
{
	if (!Interactor)
	{
		return;
	}

	if (Interactor->Inventory)
	{
		Interactor->Inventory->AddDocument(DocumentId);
	}
	if (!FlagOnRead.IsNone())
	{
		if (UTLSGameInstance* GI = Cast<UTLSGameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			GI->SetFlag(FlagOnRead);
		}
	}

	// Reading is recorded as a consumed flag, so the note stays gone after a checkpoint reload.
	// The checkpoint restores flags and journal together, so the two never disagree.
	OnDocumentRead.Broadcast(DocumentId, Body);
	MarkConsumed();
	Destroy();
}
