// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"

class TOYSTOLIFE_API TTL_IReaderBase
{
public:
    virtual ~TTL_IReaderBase() = default;
    
    virtual bool Start() = 0;
    virtual void Stop() = 0;
    
    virtual bool IsConnected() const = 0;
    virtual FString GetReaderName() const = 0;
};
