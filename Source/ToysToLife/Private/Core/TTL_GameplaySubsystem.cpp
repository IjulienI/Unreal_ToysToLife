// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/TTL_GameplaySubsystem.h"

#include "ToysToLife.h"
#include "Reader/TTL_PCSCReader.h"
#include "Reader/TTL_SimulatorStub.h"


void UTTL_GameplaySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // GIsEditor = true in editor and false in runtime(PackageBuild)
    bUseSimulator = GIsEditor;
    
    Stub = MakeShared<TTL_SimulatorStub>();
    
    StartReader();
}

void UTTL_GameplaySubsystem::Deinitialize()
{
    StopReader();    
    Stub.Reset();
    
    Super::Deinitialize();
}

void UTTL_GameplaySubsystem::StartReader()
{
    StopReader();
    
    if (bUseSimulator)
    {
        Stub->Start();
        Reader = Stub;
    }
    else
    {
#if NFC_PCSC_SUPPORTED
        TSharedPtr<TTL_PCSCReader> PCSCReader = MakeShared<TTL_PCSCReader>();
        
        if (ensure(PCSCReader->Start()))
        {
            Reader = PCSCReader;
        }
        else
        {
            bUseSimulator = true;
            Stub->Start();
            Reader = Stub;
        }
#else
        UE_LOG(LogTTLGameplay, Warning, TEXT("TTL_GameplaySubsystem: PC/SC not supported on this platform. Using simulator."))
        bUseSimulator = true;
        Stub->Start();
        Reader = Stub;
#endif
    }
}

void UTTL_GameplaySubsystem::StopReader()
{
    if (!Reader.IsValid()) return;
    
    if (Reader.Get() != Stub.Get())
    {
        Reader->Stop();
    }
    
    Reader.Reset();
}

bool UTTL_GameplaySubsystem::IsReaderConnected()
{
    return Reader.IsValid() && Reader->IsConnected();
}

FString UTTL_GameplaySubsystem::GetActiveReaderName() const
{
    return Reader.IsValid() ? Reader->GetReaderName() : TEXT("No Reader");
}

void UTTL_GameplaySubsystem::Tick(float DeltaTime)
{
    // Drain active Reader queue
    if (Reader.IsValid())
    {
        FNFCRawEvent RawEvent;
        while (Reader->EventQueue.Dequeue(RawEvent))
        {
            ProcessEvent(RawEvent);
        }
    }
    
    // Drain stub queue when hardware mode is active
    if (Stub.IsValid() && Reader.Get() != Stub.Get())
    {
        FNFCRawEvent RawEvent;
        while (Stub->EventQueue.Dequeue(RawEvent))
        {
            ProcessEvent(RawEvent);
        }
    }
}

void UTTL_GameplaySubsystem::ProcessEvent(const FNFCRawEvent& RawEvent)
{
    const FNFCFigurineEvent FigureEvent = BuildFigurineEvent(RawEvent);
    
    switch (RawEvent.EventType)
    {
    case ENFCEventType::TagPlaced:
        OnTagPlaced.Broadcast(FigureEvent);
        break;
    case ENFCEventType::TagRemoved:
        OnTagRemoved.Broadcast(FigureEvent);
        break;
    default:
        break;
    }
}

FNFCFigurineEvent UTTL_GameplaySubsystem::BuildFigurineEvent(const FNFCRawEvent& RawEvent) const
{
    FNFCFigurineEvent Event;
    Event.UID = RawEvent.UID;
    Event.bWasFound = LookupFigurine(RawEvent.UID, Event.FigurineData);
    return Event;
}

bool UTTL_GameplaySubsystem::LookupFigurine(const FString& UID, FFigurineData& OutData) const
{
    if (!FigurineDataTable)
    {
        UE_LOG(LogTTLGameplay, Verbose, TEXT("TTL_GameplaySubsystem: LookupFigurine FigurineDataTable is not set."));
        return false;
    }
    
    // Iterate all rows and compare UIDs (case-insensitive)
    const TMap<FName, uint8*>& RowMap = FigurineDataTable->GetRowMap();
    for (const auto& Pair : RowMap)
    {
        const FFigurineData* Row = reinterpret_cast<const FFigurineData*>(Pair.Value);
        if (Row && Row->UID.Equals(UID, ESearchCase::IgnoreCase))
        {
            OutData = *Row;
            return true;
        }
    }
    return false;
}

void UTTL_GameplaySubsystem::SimulateTagPlaced(const FString& UID)
{
    if (!Stub.IsValid()) return;
    Stub->SimulateTagPlaced(UID);
}

void UTTL_GameplaySubsystem::SimulateTagRemoved()
{
    if (!Stub.IsValid()) return;
    Stub->SimulateTagRemoved();
}

void UTTL_GameplaySubsystem::SimulateTagByIndex(int32 Index)
{
    if (!Stub.IsValid()) return;
    Stub->SimulateTagByIndex(Index);
}

void UTTL_GameplaySubsystem::CycleSimulatedTag()
{
    if (!Stub.IsValid()) return;
    Stub->CycleNextTag();
}

TArray<FString> UTTL_GameplaySubsystem::GetSimulatedTagLabels() const
{
    TArray<FString> Labels;
    if (!Stub.IsValid()) return Labels;
    
    for (int32 i = 0; i < Stub->SimulatedTags.Num(); ++i)
    {
        const FSimulatedTag& Tag = Stub->SimulatedTags[i];
        Labels.Add(FString::Printf(TEXT("[%d] %s  (%s)"), i, *Tag.Label, *Tag.UID));
    }
    return Labels;
}

int32 UTTL_GameplaySubsystem::AddSimulatedTag(const FString& UID, const FString& Label)
{
    if (!Stub.IsValid()) return INDEX_NONE;
    
    Stub->SimulatedTags.Add(FSimulatedTag(UID, Label));
    const int32 NewIndex = Stub->SimulatedTags.Num() - 1;
    
    UE_LOG(LogTTLGameplay, Log,TEXT("TTL_GameplaySubsystem: AddSimulatedTag [%d] — %s (%s)"),
        NewIndex,
        *Label,
        *UID);
 
    return NewIndex;
}

void UTTL_GameplaySubsystem::ClearSimulatedTags()
{
    if (!Stub.IsValid()) return;
    
    // Remove any placed tag before clearing to keep state consistent
    if (Stub->IsTagPresent())
    {
        Stub->SimulateTagRemoved();
    }
    
    Stub->SimulatedTags.Empty();
    UE_LOG(LogTTLGameplay, Log, TEXT("TTL_GameplaySubsystem: All simulated tags cleared."));
}

bool UTTL_GameplaySubsystem::IsSimulatedTagPresent() const
{
    return Stub.IsValid() && Stub->IsTagPresent();
}

FString UTTL_GameplaySubsystem::GetCurrentSimulatedUID() const
{
    return Stub.IsValid() ? Stub->GetCurrentUID() : FString();
}
