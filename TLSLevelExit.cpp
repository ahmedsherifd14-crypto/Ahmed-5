#include "Gameplay/TLSLevelExit.h"
#include "Player/TLSCharacter.h"
#include "Core/TLSGameInstance.h"
#include "Save/TLSSaveSubsystem.h"
#include "Kismet/GameplayStatics.h"

ATLSLevelExit::ATLSLevelExit()
{
	PromptText = FText::FromString(TEXT("Go through [E]"));
}

bool ATLSLevelExit::AreRequirementsMet() const
{
	const UTLSGameInstance* GI = Cast<UTLSGameInstance>(UGameplayStatics::GetGameInstance(this));
	for (const FName& Flag : RequiredFlags)
	{
		if (!GI || !GI->HasFlag(Flag))
		{
			return false;
		}
	}
	return true;
}

FText ATLSLevelExit::GetInteractPrompt(const ATLSCharacter* Interactor) const
{
	if (!AreRequirementsMet())
	{
		return LockedText.IsEmpty() ? FText::FromString(TEXT("Locked")) : LockedText;
	}
	return PromptText;
}

void ATLSLevelExit::OnInteract(ATLSCharacter* Interactor)
{
	if (!Interactor || TargetLevel.IsNone())
	{
		return;
	}
	if (!AreRequirementsMet())
	{
		OnExitLocked();
		return;
	}

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	if (UTLSSaveSubsystem* Saves = GI ? GI->GetSubsystem<UTLSSaveSubsystem>() : nullptr)
	{
		Saves->SaveCheckpoint(Interactor, FName(TEXT("level_exit")), ChapterIndex);
	}

	OnLevelExitUsed(Interactor);
	UGameplayStatics::OpenLevel(this, TargetLevel);
}
