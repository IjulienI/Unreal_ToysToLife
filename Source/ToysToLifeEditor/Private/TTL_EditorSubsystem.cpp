// Fill out your copyright notice in the Description page of Project Settings.


#include "TTL_EditorSubsystem.h"

#include "ToysToLife/Public/Reader/TTL_PCSCReader.h"

void UTTL_EditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UTTL_EditorSubsystem::Deinitialize()
{
    StopScan();
    Super::Deinitialize();
}

bool UTTL_EditorSubsystem::StartScan()
{
    if (bScanning)
    {
        OnScanStatusChanged.Broadcast(true, TEXT("Already Scanning"));
        return true;
    }
    
#if !NFC_PCSC_SUPPORTED
    const FString Message = TEXT("PC/SC not supported on this platform");
    OnScanStatusChanged.Broadcast(false, Message);
    return false;    
#else
    Reader = MakeShared<TTL_PCSCReader>();
    
    if (!Reader->Start())
    {
        Reader.Reset();
        const FString Message = TEXT("No NFC reader found. Plug in your ACR122U and try again");
        OnScanStatusChanged.Broadcast(false, Message);
        return false;
    }
    
    if (GEditor)
    {
        GEditor->GetTimerManager()->SetTimer(PollingTimerHandle, this, &UTTL_EditorSubsystem::PollReaderQueue, PollingIntervalSec, true);
    }
    bScanning = true;
    LastScannedUID.Empty();
    LastPolledUID.Empty();
    
    const FString Message = TEXT("Scanning... Place a tag on the reader");
    OnScanStatusChanged.Broadcast(true, Message);
    return true;
#endif
}

void UTTL_EditorSubsystem::StopScan()
{
    if (!bScanning) return;
    
    // Cancel Timer
    if (GEditor && PollingTimerHandle.IsValid())
    {
        GEditor->GetTimerManager()->ClearTimer(PollingTimerHandle);
    }
    
    // Stop reader thread
    if (Reader.IsValid())
    {
        Reader->Shutdown();
        Reader.Reset();
    }
    bScanning = false;
    OnScanStatusChanged.Broadcast(false, TEXT("Scan stopped."));
}

void UTTL_EditorSubsystem::PollReaderQueue()
{
    if (!Reader.IsValid()) return;
    
    FNFCRawEvent RawEvent;
    while (Reader->EventQueue.Dequeue(RawEvent))
    {
        if (RawEvent.EventType == ENFCEventType::TagPlaced)
        {
            if (RawEvent.UID != LastScannedUID)
            {
                LastScannedUID = RawEvent.UID;
                OnTagScanned.Broadcast(RawEvent.UID);
                OnScanStatusChanged.Broadcast(true, FString::Printf(TEXT("Tag detected: %s"), *RawEvent.UID));
            }
        }
        else if (RawEvent.EventType == ENFCEventType::TagRemoved)
        {
            OnScanStatusChanged.Broadcast(true, TEXT("Tag removed. Place a new tag..."));
        }
    }
}

bool UTTL_EditorSubsystem::AddRowToDataTable(UDataTable* TargetTable, const FName& RowName, const FFigurineData& Data)
{
    if (!TargetTable)
    {
        return false;
    }
    if (RowName.IsNone())
    {
        return false;
    }
    
    TargetTable->AddRow(RowName, Data);
    TargetTable->MarkPackageDirty();
    return true;
}

bool UTTL_EditorSubsystem::RemoveRowFromDataTable(UDataTable* TargetTable, const FName& RowName)
{
    if (!TargetTable) return false;
    if (!TargetTable->GetRowMap().Contains(RowName)) return false;
    
    TMap<FName, FFigurineData> RowsBackup;
    
    for (TPair<FName, unsigned char*> RowMap : TargetTable->GetRowMap())
    {
        if (RowMap.Key == RowName)
        {
            continue;
        }

        if (const FFigurineData* RowData = reinterpret_cast<const FFigurineData*>(RowMap.Value))
        {
            RowsBackup.Add(RowMap.Key, *RowData);
        }
    }
    TargetTable->Modify();
    TargetTable->EmptyTable();
    
    for (TPair<FName, FFigurineData> Backup : RowsBackup)
    {
        TargetTable->AddRow(Backup.Key, Backup.Value);
    }
    TargetTable->MarkPackageDirty();
    TargetTable->PostEditChange();
    
    return true;
}

bool UTTL_EditorSubsystem::IsUIDAlreadyRegistered(UDataTable* TargetTable, const FString& UID, FName& OutRowName) const
{
    if (!TargetTable || UID.IsEmpty()) return false;
    
    for (const auto& Pair : TargetTable->GetRowMap())
    {
        const FFigurineData* Row = reinterpret_cast<const FFigurineData*>(Pair.Value);
        if (Row && Row->UID.Equals(UID, ESearchCase::IgnoreCase))
        {
            OutRowName = Pair.Key;
            return true;
        }
    }
    return false;
}

TArray<FFigurineData> UTTL_EditorSubsystem::GetAllRows(UDataTable* TargetTable) const
{
    TArray<FFigurineData> Result;
    if (!TargetTable) return Result;
    
    if (!TargetTable || TargetTable->GetRowStruct() != FFigurineData::StaticStruct())
    {
        return Result;
    }
    
    for (const auto& Pair : TargetTable->GetRowMap())
    {
        const FFigurineData* Row = reinterpret_cast<const FFigurineData*>(Pair.Value);
        if (Row)
        {
            Result.Add(*Row);
        }
    }
    return Result;
}

TArray<FName> UTTL_EditorSubsystem::GetAllRowNames(UDataTable* TargetTable) const
{
    TArray<FName> Result;
    if (!TargetTable) return Result;
    
    TargetTable->GetRowMap().GetKeys(Result);
    return Result;
}
