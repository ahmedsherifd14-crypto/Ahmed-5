#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TLSInventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FTLSInventoryEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ItemId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Count = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

/** Stackable items plus a set of collected document IDs (the "journal"). */
UCLASS(ClassGroup = (TLS), meta = (BlueprintSpawnableComponent))
class THELASTSIGNAL_API UTLSInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTLSInventoryComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 MaxSlots = 12;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(FName ItemId, int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(FName ItemId, int32 Count = 1);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetCount(FName ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(FName ItemId) const { return GetCount(ItemId) > 0; }

	UFUNCTION(BlueprintCallable, Category = "Journal")
	bool AddDocument(FName DocumentId);

	UFUNCTION(BlueprintPure, Category = "Journal")
	bool HasDocument(FName DocumentId) const { return Documents.Contains(DocumentId); }

	UFUNCTION(BlueprintPure, Category = "Journal")
	const TArray<FName>& GetDocuments() const { return Documents; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	const TArray<FTLSInventoryEntry>& GetEntries() const { return Entries; }

	/** Used by save/load to restore state without firing a flood of change events. */
	void SetState(const TArray<FTLSInventoryEntry>& NewEntries, const TArray<FName>& NewDocuments);

private:
	UPROPERTY()
	TArray<FTLSInventoryEntry> Entries;

	UPROPERTY()
	TArray<FName> Documents;
};
