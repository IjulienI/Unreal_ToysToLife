// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "TTL_EditorSubsystem.generated.h"

class TTL_PCSCReader;
struct FFigurineData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEditorTagScanned, const FString&, UID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScanStatusChanged, bool, bIsScanning, const FString&, StatusMessage);

UCLASS()
class TOYSTOLIFEEDITOR_API UTTL_EditorSubsystem : public UEditorSubsystem
{
    GENERATED_BODY()
    
public:
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "NFC|Editor")
    FOnEditorTagScanned OnTagScanned;
    
    UPROPERTY(BlueprintAssignable, Category = "NFC|Editor")
    FOnScanStatusChanged OnScanStatusChanged;
    
    // Scan control
    UFUNCTION(BlueprintCallable, Category = "NFC|Editor")
    bool StartScan();
    
    UFUNCTION(BlueprintCallable, Category = "NFC|Editor")
    void StopScan();
    
    UFUNCTION(BlueprintPure, Category = "NFC|Editor")
    bool IsScanning() const { return bScanning; }
    
    UFUNCTION(BlueprintPure, Category = "NFC|Editor")
    FString GetLastScannedUID() const { return LastScannedUID; }
    
    UFUNCTION(BlueprintCallable, Category = "NFC|Editor")
    void ClearLastUID() { LastScannedUID.Empty(); }
    
    // Datatable operation
    UFUNCTION(BlueprintCallable, Category = "NFC|Editor")
    bool AddRowToDataTable(UDataTable* TargetTable, const FName& RowName, const FFigurineData& Data);
    
    UFUNCTION(BlueprintCallable, Category = "NFC|Editor")
    bool RemoveRowFromDataTable(UDataTable* TargetTable, const FName& RowName);
    
    UFUNCTION(BlueprintCallable, Category = "NFC|Editor")
    bool IsUIDAlreadyRegistered(UDataTable* TargetTable, const FString& UID, FName& OutRowName) const;
    
    UFUNCTION(BlueprintCallable, Category = "NFC|Editor")
    TArray<FFigurineData> GetAllRows(UDataTable* TargetTable) const;
    
    UFUNCTION(BlueprintCallable, Category = "NFC|Editor")
    TArray<FName> GetAllRowNames(UDataTable* TargetTable) const;
    
    // UEditorSubsystem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
private:
    bool bScanning = false;
    FString LastScannedUID;
    FString LastPolledUID;
    
    FTimerHandle PollingTimerHandle;
    
    static constexpr float PollingIntervalSec = 0.15f;
    
    TSharedPtr<TTL_PCSCReader> Reader;
    
    void PollReaderQueue();
};
