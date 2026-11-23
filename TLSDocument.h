#pragma once

#include "CoreMinimal.h"
#include "Gameplay/TLSInteractable.h"
#include "TLSDocument.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDocumentRead, FName, DocumentId, FText, Body);

/**
 * A readable note or log. Reading it adds it to the journal and shows the text. Story-relevant
 * documents can set a flag so later content reacts to what the player found.
 */
UCLASS()
class THELASTSIGNAL_API ATLSDocument : public ATLSInteractableActor
{
	GENERATED_BODY()

public:
	virtual FText GetInteractPrompt(const ATLSCharacter* Interactor) const override;
	virtual void OnInteract(ATLSCharacter* Interactor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	FName DocumentId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document", meta = (MultiLine = true))
	FText Body;

	/** Flag set when this document is read (e.g. "ReadLogA"). Optional. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	FName FlagOnRead;

	/** Shows the document. Blueprint or the HUD listens to this and displays Title/Body. */
	UPROPERTY(BlueprintAssignable, Category = "Document")
	FOnDocumentRead OnDocumentRead;
};
