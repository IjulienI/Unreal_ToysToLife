// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/TTL_IReaderBase.h"

struct FSimulatedTag
{
    FString UID;
    FString Label;
    
    FSimulatedTag() = default;
    FSimulatedTag(const FString& InUID, const FString& InLabel) : UID(InUID), Label(InLabel) {}
};

class TOYSTOLIFE_API TTL_SimulatorStub : public TTL_IReaderBase
{
public:
    TTL_SimulatorStub();
    virtual ~TTL_SimulatorStub() override;
    
    // Interface
    virtual bool Start() override;
    virtual void Stop() override;
    virtual bool IsConnected() const override { return true; }
    virtual FString GetReaderName() const override { return TEXT("NFC Simulator"); }
    
    // Simulation API
    void SimulateTagPlaced(const FString& UID);
    void SimulateTagRemoved();
    void SimulateTagByIndex(int32 Index);
    void CycleNextTag();
    
    // Configurable tag List
    FString GetCurrentUID() const { return CurrentUID; }
    bool IsTagPresent() const { return bTagPresent; }
    int32 GetCurrentTagIndex() const { return CurrentTagIndex; }
    
    TArray<FSimulatedTag> SimulatedTags;
    
private:
    bool bTagPresent = false;
    FString CurrentUID;
    int32 CurrentTagIndex = -1;
};
