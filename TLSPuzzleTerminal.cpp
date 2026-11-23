#include "Gameplay/TLSPuzzleTerminal.h"
#include "Player/TLSCharacter.h"
#include "Core/TLSGameInstance.h"
#include "Kismet/GameplayStatics.h"

ATLSPuzzleTerminal::ATLSPuzzleTerminal()
{
	PromptText = FText::FromString(TEXT("Use keypad [E]"));
}

namespace
{
	UTLSGameInstance* GetTLSGameInstance(const UObject* WorldContext)
	{
		return Cast<UTLSGameInstance>(UGameplayStatics::GetGameInstance(WorldContext));
	}
}

bool ATLSPuzzleTerminal::IsSolved() const
{
	const UTLSGameInstance* GI = GetTLSGameInstance(this);
	return GI && !SolvedFlag.IsNone() && GI->HasFlag(SolvedFlag);
}

FText ATLSPuzzleTerminal::GetInteractPrompt(const ATLSCharacter* Interactor) const
{
	if (IsSolved())
	{
		return FText::FromString(TEXT("Keypad (unlocked)"));
	}
	return PromptText;
}

void ATLSPuzzleTerminal::OnInteract(ATLSCharacter* Interactor)
{
	if (IsSolved())
	{
		return;
	}

	const UTLSGameInstance* GI = GetTLSGameInstance(this);
	if (!RequiredFlag.IsNone() && (!GI || !GI->HasFlag(RequiredFlag)))
	{
		// Shown as a message by the Blueprint: the keypad has no power yet.
		OnRejected();
		return;
	}

	OnOpenKeypad(Interactor);
}

bool ATLSPuzzleTerminal::SubmitCode(const FString& Code)
{
	if (IsSolved())
	{
		return true;
	}

	if (Code != CorrectCode)
	{
		OnRejected();
		return false;
	}

	if (UTLSGameInstance* GI = GetTLSGameInstance(this))
	{
		if (!SolvedFlag.IsNone())
		{
			GI->SetFlag(SolvedFlag);
		}
	}
	OnPuzzleSolved.Broadcast();
	return true;
}
