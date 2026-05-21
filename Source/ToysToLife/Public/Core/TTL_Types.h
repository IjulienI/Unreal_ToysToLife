// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TTL_Types.generated.h"

// ENUMS

// Type of the NFC figures.
UENUM(BlueprintType)
enum class ENFCFigurineType : uint8
{
    Unknown UMETA(DisplayName = "Unknown"),
    Character UMETA(DisplayName = "Character"),
    World UMETA(DisplayName = "World"),
    Totem UMETA(DisplayName = "Totem"),
};

// Type of NFC event
UENUM(BlueprintType)
enum class ENFCEventType : uint8
{
    TagPlaced UMETA(DisplayName = "Tag Placed"),
    TagRemoved UMETA(DisplayName = "Tag Removed"),
};


// Internal communication struct
struct FNFCRawEvent
{
    ENFCEventType  EventType = ENFCEventType::TagPlaced;
    
    // UID formatted as uppercase hex pair separated by colons e.g "04:A3:B2:C1:D0:E4:F5"
    FString UID;
};


// DataTable row : maps one UID to a figure definition
USTRUCT(BlueprintType)
struct TOYSTOLIFE_API FFigurineData : public FTableRowBase
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NFC")
    FString UID;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NFC")
    FString Name;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NFC")
    ENFCFigurineType Type = ENFCFigurineType::Unknown;
    
    // For Character type
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NFC|Character", meta = (EditCondition = "Type == ENFCFigurineType::Character"))
    TSoftClassPtr<AActor> CharacterClass;
    
    // For World Type
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NFC|World", meta = (EditCondition = "Type == ENFCFigurineType::World"))
    FName LevelName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NFC")
    TSoftObjectPtr<UTexture2D> Thumbnail;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NFC")
    FString Metadata;
};

// Blueprint facing event
USTRUCT(BlueprintType)
struct TOYSTOLIFE_API FNFCFigurineEvent
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly, Category = "NFC")
    FString UID;
    
    UPROPERTY(BlueprintReadOnly, Category = "NFC")
    FFigurineData FigurineData;
    
    UPROPERTY(BlueprintReadOnly, Category = "NFC")
    bool bWasFound = false;
};