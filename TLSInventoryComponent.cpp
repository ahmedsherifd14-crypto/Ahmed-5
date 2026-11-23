#include "Player/TLSInventoryComponent.h"

UTLSInventoryComponent::UTLSInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UTLSInventoryComponent::AddItem(FName ItemId, int32 Count)
{
	if (ItemId.IsNone() || Count <= 0)
	{
		return false;
	}

	for (FTLSInventoryEntry& E : Entries)
	{
		if (E.ItemId == ItemId)
		{
			E.Count += Count;
			OnInventoryChanged.Broadcast();
			return true;
		}
	}

	if (Entries.Num() >= MaxSlots)
	{
		return false; // Inventory full; the pickup stays in the world.
	}

	FTLSInventoryEntry New;
	New.ItemId = ItemId;
	New.Count = Count;
	Entries.Add(New);
	OnInventoryChanged.Broadcast();
	return true;
}

bool UTLSInventoryComponent::RemoveItem(FName ItemId, int32 Count)
{
	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		if (Entries[i].ItemId == ItemId)
		{
			if (Entries[i].Count < Count)
			{
				return false;
			}
			Entries[i].Count -= Count;
			if (Entries[i].Count == 0)
			{
				Entries.RemoveAt(i);
			}
			OnInventoryChanged.Broadcast();
			return true;
		}
	}
	return false;
}

int32 UTLSInventoryComponent::GetCount(FName ItemId) const
{
	for (const FTLSInventoryEntry& E : Entries)
	{
		if (E.ItemId == ItemId)
		{
			return E.Count;
		}
	}
	return 0;
}

bool UTLSInventoryComponent::AddDocument(FName DocumentId)
{
	if (DocumentId.IsNone() || Documents.Contains(DocumentId))
	{
		return false;
	}
	Documents.Add(DocumentId);
	OnInventoryChanged.Broadcast();
	return true;
}

void UTLSInventoryComponent::SetState(const TArray<FTLSInventoryEntry>& NewEntries, const TArray<FName>& NewDocuments)
{
	Entries = NewEntries;
	Documents = NewDocuments;
	OnInventoryChanged.Broadcast();
}
