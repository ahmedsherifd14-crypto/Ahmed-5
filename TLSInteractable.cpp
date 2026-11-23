#include "Gameplay/TLSInteractable.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Perception/AISense_Hearing.h"
#include "Player/TLSCharacter.h"
#include "Core/TLSGameInstance.h"
#include "Kismet/GameplayStatics.h"

ATLSInteractableActor::ATLSInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionProfileName(TEXT("Interactable"));

	// Placeholder cube so placed interactables are visible and can be focused. Replace per Blueprint.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaceholderCube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (PlaceholderCube.Succeeded())
	{
		Mesh->SetStaticMesh(PlaceholderCube.Object);
		Mesh->SetWorldScale3D(FVector(0.3f));
	}
}

void ATLSInteractableActor::BeginPlay()
{
	Super::BeginPlay();
	if (IsConsumed())
	{
		Destroy();
	}
}

bool ATLSInteractableActor::IsConsumed() const
{
	if (PersistentId.IsNone())
	{
		return false;
	}
	if (const UTLSGameInstance* GI = Cast<UTLSGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		return GI->HasFlag(FName(*FString::Printf(TEXT("Consumed_%s"), *PersistentId.ToString())));
	}
	return false;
}

void ATLSInteractableActor::MarkConsumed()
{
	if (PersistentId.IsNone())
	{
		return;
	}
	if (UTLSGameInstance* GI = Cast<UTLSGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SetFlag(FName(*FString::Printf(TEXT("Consumed_%s"), *PersistentId.ToString())));
	}
}

FText ATLSInteractableActor::GetInteractPrompt(const ATLSCharacter* Interactor) const
{
	return PromptText.IsEmpty() ? FText::FromString(TEXT("Interact [E]")) : PromptText;
}

void ATLSInteractableActor::ReportNoise(const ATLSCharacter* Source) const
{
	if (InteractNoiseLoudness > 0.f)
	{
		UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), InteractNoiseLoudness,
			const_cast<ATLSCharacter*>(Source), 0.f, TEXT("Interaction"));
	}
}
