#include "Gameplay/TLSEndingTerminal.h"
#include "Player/TLSCharacter.h"
#include "Kismet/GameplayStatics.h"

ATLSEndingTerminal::ATLSEndingTerminal()
{
	PromptText = FText::FromString(TEXT("Access terminal [E]"));
}

TArray<FTLSEndingOption> ATLSEndingTerminal::GetAvailableOptions() const
{
	const UTLSGameInstance* GI = Cast<UTLSGameInstance>(UGameplayStatics::GetGameInstance(this));
	TArray<FTLSEndingOption> Result;
	for (const FTLSEndingOption& Opt : Options)
	{
		bool bMeetsRequirements = true;
		for (const FName& Flag : Opt.RequiredFlags)
		{
			if (!GI || !GI->HasFlag(Flag))
			{
				bMeetsRequirements = false;
				break;
			}
		}
		if (bMeetsRequirements)
		{
			Result.Add(Opt);
		}
	}
	return Result;
}

FText ATLSEndingTerminal::GetInteractPrompt(const ATLSCharacter* Interactor) const
{
	return PromptText;
}

void ATLSEndingTerminal::OnInteract(ATLSCharacter* Interactor)
{
	OnShowChoices(Interactor);
}

bool ATLSEndingTerminal::ChooseEnding(ETLSEnding Ending)
{
	for (const FTLSEndingOption& Opt : GetAvailableOptions())
	{
		if (Opt.Ending != Ending)
		{
			continue;
		}

		if (UTLSGameInstance* GI = Cast<UTLSGameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			GI->ChosenEnding = Ending;
		}
		if (!Opt.LevelName.IsNone())
		{
			UGameplayStatics::OpenLevel(this, Opt.LevelName);
		}
		return true;
	}
	return false;
}
