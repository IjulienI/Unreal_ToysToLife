// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TTL_Types.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TTL_GameplaySubsystem.generated.h"

class TTL_SimulatorStub;
class TTL_IReaderBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNFCTagPlaced, const FNFCFigurineEvent&, Event); 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNFCTagRemoved, const FNFCFigurineEvent&, Event);


UCLASS()
class TOYSTOLIFE_API UTTL_GameplaySubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
    GENERATED_BODY()
    
public:
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "NFC|Envents")
    FOnNFCTagPlaced OnTagPlaced;
    
    UPROPERTY(BlueprintAssignable, Category = "NFC|Events")
    FOnNFCTagRemoved OnTagRemoved;

    // Configuration
    UPROPERTY(BlueprintReadWrite, Category = "NFC|Config")
    UDataTable* FigurineDataTable = nullptr;
    
    UPROPERTY(BlueprintReadWrite, Category = "NFC|Config")
    bool bUseSimulator = false;
    
    // Reader Controls
    UFUNCTION(BlueprintCallable, Category = "NFC")
    void StartReader();
    
    UFUNCTION(BlueprintCallable, Category = "NFC")
    void StopReader();
    
    UFUNCTION(BlueprintCallable, Category = "NFC")
    bool IsReaderConnected();
    
    UFUNCTION(BlueprintCallable, Category = "NFC")
    FString GetActiveReaderName() const;
    
    // Datatable Lookup
    UFUNCTION(BlueprintCallable, Category = "NFC")
    bool LookupFigurine(const FString& UID, FFigurineData& OutData) const;
    
    // Simulation API
    UFUNCTION(BlueprintCallable, Category = "NFC|Simulator")
    void SimulateTagPlaced(const FString& UID);
    
    UFUNCTION(BlueprintCallable, Category = "NFC|Simulator")
    void SimulateTagRemoved();
    
    UFUNCTION(BlueprintCallable, Category = "NFC|Simulator")
    void SimulateTagByIndex(int32 Index);
    
    UFUNCTION(BlueprintCallable, Category = "NFC|Simulator")
    void CycleSimulatedTag();

    UFUNCTION(BlueprintCallable, Category = "NFC|Simulator")
    TArray<FString> GetSimulatedTagLabels() const;

    UFUNCTION(BlueprintCallable, Category = "NFC|Simulator")
    int32 AddSimulatedTag(const FString& UID, const FString& Label);

    UFUNCTION(BlueprintCallable, Category = "NFC|Simulator")
    void ClearSimulatedTags();

    UFUNCTION(BlueprintCallable, Category = "NFC|Simulator")
    bool IsSimulatedTagPresent() const;

    UFUNCTION(BlueprintCallable, Category = "NFC|Simulator")
    FString GetCurrentSimulatedUID() const;

    // USubsystem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    // FTickableGameObject
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return true; }
    virtual bool IsTickableInEditor() const override { return false; }
    virtual TStatId GetStatId() const override
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT(UTTL_GameplaySubsystem, STATGROUP_Tickables);
    }
    
private:
    TSharedPtr<TTL_IReaderBase> Reader;
    TSharedPtr<TTL_SimulatorStub> Stub;
    
    void ProcessEvent(const FNFCRawEvent& RawEvent);
    FNFCFigurineEvent BuildFigurineEvent(const FNFCRawEvent& RawEvent) const;
};
