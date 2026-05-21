// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/TTL_IReaderBase.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "HAL/ThreadSafeBool.h"

#if NFC_PCSC_SUPPORTED
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/AllowWindowsPlatformAtomics.h"
#include <winscard.h>
#include "Windows/HideWindowsPlatformAtomics.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif 

class TOYSTOLIFE_API TTL_PCSCReader : public TTL_IReaderBase, public FRunnable
{
public:
    TTL_PCSCReader();
    virtual ~TTL_PCSCReader() override;
    
    // Interface
    virtual bool Start() override;
    virtual void Shutdown() override;
    virtual bool IsConnected() const override;
    virtual FString GetReaderName() const override;
    
    // FRunnable
    virtual bool Init() override;
    virtual uint32 Run() override;
    virtual void Exit() override;
    virtual void Stop() override;
    
private:
    FRunnableThread* Thread = nullptr;
    FThreadSafeBool bRunning { false };
    FThreadSafeBool bConnected { false };
    
    FString LastUID;
    bool bTagPresent = false;
    
    static constexpr float PollingIntervalSec = 0.1f;
    
#if NFC_PCSC_SUPPORTED
    SCARDCONTEXT Context = 0;
    
    bool EstablishContext();
    void ReleaseContext();
    bool FindFirstReader(FString& OutReaderName) const;
    FString GetCardUID(const FString& ReaderName);
    FString BytesToHexString(const uint8* Data, DWORD Length) const;
#endif
};
