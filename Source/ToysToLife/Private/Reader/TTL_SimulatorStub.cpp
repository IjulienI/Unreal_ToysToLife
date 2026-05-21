// Fill out your copyright notice in the Description page of Project Settings.


#include "Reader/TTL_SimulatorStub.h"
#include "ToysToLife.h"

TTL_SimulatorStub::TTL_SimulatorStub()
{
}

TTL_SimulatorStub::~TTL_SimulatorStub()
{
    Stop();
}

bool TTL_SimulatorStub::Start()
{
    UE_LOG(LogTTLGameplay, Log, TEXT("TTL_Simulator: Ready. %d tags pre loaded. "
        "Use CycleNextTag() or SimulateTagByIndex(N) to inject events."), SimulatedTags.Num());
    return true;
}

void TTL_SimulatorStub::Stop()
{
    if (bTagPresent)
    {
        SimulateTagRemoved();
    }
}

void TTL_SimulatorStub::SimulateTagPlaced(const FString& UID)
{
    if (UID.IsEmpty())
    {
        UE_LOG(LogTTLGameplay, Warning, TEXT("TTL_Simulator: SimulateTagPlaced called with empty UID."));
        return;
    }
    
    if (bTagPresent)
    {
        SimulateTagRemoved();
    }
    
    bTagPresent = true;
    CurrentUID = UID;
    
    FNFCRawEvent Event;
    Event.EventType = ENFCEventType::TagPlaced;
    Event.UID = UID;
    EventQueue.Enqueue(Event);
    
    UE_LOG(LogTTLGameplay, Log, TEXT("TTL_Simulator: Tag PLACED | UID: %s"), *UID);
}

void TTL_SimulatorStub::SimulateTagRemoved()
{
    if (!bTagPresent)
    {
        return;
    }
    
    const FString RemovedUID = CurrentUID;
    
    bTagPresent = false;
    CurrentUID = TEXT("");
    CurrentTagIndex = -1;
    
    FNFCRawEvent Event;
    Event.EventType = ENFCEventType::TagRemoved;
    Event.UID = RemovedUID;
    EventQueue.Enqueue(Event);
    
    UE_LOG(LogTTLGameplay, Log, TEXT("TTL_Simulator: Tag REMOVED | UID: %s"), *RemovedUID);
}

void TTL_SimulatorStub::SimulateTagByIndex(int32 Index)
{
    if (!SimulatedTags.IsValidIndex(Index))
    {
        UE_LOG(LogTTLGameplay, Warning,
            TEXT("TTL_Simulator: Invalid index %d (valid range: 0–%d)."),
            Index, SimulatedTags.Num() - 1);
        return;
    }
    
    CurrentTagIndex = Index;
    const FSimulatedTag& Tag = SimulatedTags[Index];
    
    UE_LOG(LogTTLGameplay, Log,
        TEXT("TTL_Simulator: Placing tag [%d] — %s (%s)"),
        Index, *Tag.Label, *Tag.UID);
    
    SimulateTagPlaced(Tag.UID);
}

void TTL_SimulatorStub::CycleNextTag()
{
    if (SimulatedTags.IsEmpty())
    {
        UE_LOG(LogTTLGameplay, Warning, TEXT("TTL_Simulator: CycleNextTag called but SimulatedTags is empty."));
        return;
    }
    
    CurrentTagIndex = (CurrentTagIndex + 1) % SimulatedTags.Num();
    SimulateTagByIndex(CurrentTagIndex);
}
