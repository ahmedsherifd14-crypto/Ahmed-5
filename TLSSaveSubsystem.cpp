#include "Save/TLSSaveSubsystem.h"
#include "Save/TLSSaveGame.h"
#include "Core/TLSGameInstance.h"
#include "Player/TLSCharacter.h"
#include "Player/TLSFlashlightComponent.h"
#include "Player/TLSInventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

bool UTLSSaveSubsystem::HasSave() const
{
	return UGameplayStatics::DoesSaveGameExist(AutoSlot, 0);
}

bool UTLSSaveSubsystem::SaveCheckpoint(ATLSCharacter* Player, FName CheckpointId, int32 ChapterIndex)
{
	if (!Player || Player->IsDead())
	{
		return false;
	}

	UTLSGameInstance* GI = Cast<UTLSGameInstance>(GetGameInstance());
	UTLSSaveGame* Save = Cast<UTLSSaveGame>(UGameplayStatics::CreateSaveGameObject(UTLSSaveGame::StaticClass()));
	if (!Save || !GI)
	{
		return false;
	}

	Save->LevelName = CurrentLevel;
	Save->CheckpointId = CheckpointId;
	Save->ChapterIndex = ChapterIndex;
	Save->PlayerLocation = Player->GetActorLocation();
	Save->PlayerRotation = Player->GetControlRotation();
	Save->Health = Player->Health;
	Save->Stamina = Player->Stamina;
	Save->Battery = Player->Flashlight ? Player->Flashlight->Battery : 100.f;
	Save->SavedAt = FDateTime::Now().ToString();

	if (UTLSInventoryComponent* Inv = Player->Inventory)
	{
		Save->Items = Inv->GetEntries();
		Save->Documents = Inv->GetDocuments();
	}

	Save->Flags = GI->StoryFlags.Array();

	return UGameplayStatics::SaveGameToSlot(Save, AutoSlot, 0);
}

bool UTLSSaveSubsystem::LoadCheckpoint()
{
	if (!HasSave())
	{
		return false;
	}

	PendingSave = Cast<UTLSSaveGame>(UGameplayStatics::LoadGameFromSlot(AutoSlot, 0));
	if (!PendingSave)
	{
		return false;
	}

	if (UTLSGameInstance* GI = Cast<UTLSGameInstance>(GetGameInstance()))
	{
		GI->StoryFlags.Reset();
		for (const FName& Flag : PendingSave->Flags)
		{
			GI->StoryFlags.Add(Flag);
		}
	}

	UGameplayStatics::OpenLevel(GetWorld(), FName(*PendingSave->LevelName));
	return true;
}

void UTLSSaveSubsystem::StartNewGame(const FString& FirstLevel)
{
	if (UTLSGameInstance* GI = Cast<UTLSGameInstance>(GetGameInstance()))
	{
		GI->StoryFlags.Reset();
		GI->ChosenEnding = ETLSEnding::None;
	}
	UGameplayStatics::DeleteGameInSlot(AutoSlot, 0);
	PendingSave = nullptr;
	UGameplayStatics::OpenLevel(GetWorld(), FName(*FirstLevel));
}

bool UTLSSaveSubsystem::ApplyPendingRestore(ATLSCharacter* Player)
{
	if (!PendingSave || !Player)
	{
		return false;
	}

	Player->SetActorLocationAndRotation(PendingSave->PlayerLocation, PendingSave->PlayerRotation.Quaternion(),
		false, nullptr, ETeleportType::TeleportPhysics);
	if (AController* C = Player->GetController())
	{
		C->SetControlRotation(PendingSave->PlayerRotation);
	}

	Player->RestoreVitals(PendingSave->Health, PendingSave->Stamina);
	if (Player->Flashlight)
	{
		Player->Flashlight->Battery = PendingSave->Battery;
	}
	if (Player->Inventory)
	{
		Player->Inventory->SetState(PendingSave->Items, PendingSave->Documents);
	}

	PendingSave = nullptr;
	return true;
}
