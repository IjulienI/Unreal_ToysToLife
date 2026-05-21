// Fill out your copyright notice in the Description page of Project Settings.


#include "Reader/TTL_PCSCReader.h"

#include "ToysToLife.h"


TTL_PCSCReader::TTL_PCSCReader() {}

TTL_PCSCReader::~TTL_PCSCReader()
{
    Stop();
}

bool TTL_PCSCReader::Start()
{
#if !NFC_PCSC_SUPPORTED
    UE_LOG(LogTTLGameplay, Warning, TEXT("TTL_PCSCReader: PC/SC is only supported on Windows. Use the simulator."))
    return false;
#else
    if (bRunning)
    {
        UE_LOG(LogTTLGameplay, Warning, TEXT("TTL_PCSCReader: PC/SC is already running."))
        return true;
    }
    
    SCARDCONTEXT TempContext = 0;
    LONG PreCheck = SCardEstablishContext(SCARD_SCOPE_USER, nullptr, nullptr, &TempContext);
    if (PreCheck != SCARD_S_SUCCESS)
    {
        UE_LOG(LogTTLGameplay, Warning, TEXT("TTL_PCSCReader: No PC/SC context available (error 0x%08X). Is a reader plugged in?"),
            (uint32)PreCheck);
        return false;
    }
    
    DWORD dwReaders = SCARD_AUTOALLOCATE;
    LPTSTR pszReaders = nullptr;
    LONG ListResult = SCardListReaders(TempContext, nullptr, (LPTSTR)&pszReaders, &dwReaders);
    bool bHasReader = ListResult == SCARD_S_SUCCESS && pszReaders && *pszReaders != TEXT('\0');
    
    if (pszReaders)
    {
        SCardFreeMemory(TempContext, pszReaders);
    }
    SCardReleaseContext(TempContext);
    
    if (!bHasReader)
    {
        UE_LOG(LogTTLGameplay, Warning, TEXT("TTL_PCSCReader: PC/SC context established but no readers found. Plug in an ACR122U."))
        return false;
    }
    
    bRunning = true;
    bConnected = false;
    Thread = FRunnableThread::Create(this, TEXT("NFCReaderThread"), 0, TPri_BelowNormal);
    
    if (!Thread)
    {
        bRunning = false;
        UE_LOG(LogTTLGameplay, Warning, TEXT("TTL_PCSCReader: Failed to create reader thread."))
        return false;
    }
    UE_LOG(LogTTLGameplay, Log, TEXT("TTL_PCSCReader: Reader thread spawned. Polling at %.0f Hz."),
        1.0f / PollingIntervalSec);
    return true;
#endif
}

void TTL_PCSCReader::Stop()
{
    bRunning = false;
    
    if (Thread)
    {
        // Todo : Fix the overflow !!!!
        Thread->WaitForCompletion();
        delete Thread;
        Thread = nullptr;
    }
    bConnected = false;
}

bool TTL_PCSCReader::IsConnected() const
{
    return bConnected;
}

FString TTL_PCSCReader::GetReaderName() const
{
    return FString("TTL_PCSCReader");
}

bool TTL_PCSCReader::Init()
{
    return true;
}

uint32 TTL_PCSCReader::Run()
{
#if !NFC_PCSC_SUPPORTED
    return 1;
#else
    if (!EstablishContext())
    {
        UE_LOG(LogTTLGameplay, Error, TEXT("TTL_PCSCReader: [Thread] Failed to establish PC/SC context. Thread exiting."))
        bConnected = false;
        return 1;
    }
    
    bConnected = true;
    UE_LOG(LogTTLGameplay, Log, TEXT("TTL_PCSCReader: [Thread] PC/SC context ready. Polling..."))
    
    while (bRunning)
    {
        FString ReaderName;
        if (FindFirstReader(ReaderName))
        {
            FString CurrentUID = GetCardUID(ReaderName);
            
            if (!CurrentUID.IsEmpty() && !bTagPresent)
            {
                bTagPresent = true;
                LastUID = CurrentUID;
                
                FNFCRawEvent Event;
                Event.EventType = ENFCEventType::TagPlaced;
                Event.UID = CurrentUID;
                EventQueue.Enqueue(Event);
            }
            else if (CurrentUID.IsEmpty() && bTagPresent)
            {
                bTagPresent = false;
                
                FNFCRawEvent Event;
                Event.EventType = ENFCEventType::TagRemoved;
                Event.UID = LastUID;
                EventQueue.Enqueue(Event);
                
                LastUID.Empty();
            }
            else
            {
                if (bTagPresent)
                {
                    bTagPresent = false;
                    FNFCRawEvent Event;
                    Event.EventType = ENFCEventType::TagRemoved;
                    Event.UID = LastUID;
                    EventQueue.Enqueue(Event);
                    LastUID.Empty();
                }

                UE_LOG(LogTTLGameplay, Warning, TEXT("TTL_PCSCReader: [Thread] No readers found. Retrying..."));

            }
            FPlatformProcess::Sleep(PollingIntervalSec);
        }
    }
    
    ReleaseContext();
    UE_LOG(LogTTLGameplay, Log, TEXT("TTL_PCSCReader: [Thread] Polling stopped cleanly."));
    return 0;
#endif
}

void TTL_PCSCReader::Exit()
{
    bConnected = false;
}

bool TTL_PCSCReader::EstablishContext()
{
    LONG Result = SCardEstablishContext(SCARD_SCOPE_USER, nullptr, nullptr, &Context);
    if (Result != SCARD_S_SUCCESS)
    {
        UE_LOG(LogTTLGameplay, Error,
            TEXT("TTL_PCSCReader: SCardEstablishContext failed — 0x%08X"), 
            (uint32)Result);
        return false;
    }
    return true;
}

void TTL_PCSCReader::ReleaseContext()
{
    if (Context != 0)
    {
        SCardReleaseContext(Context);
        Context = 0;
    }
}

bool TTL_PCSCReader::FindFirstReader(FString& OutReaderName) const
{
    DWORD dwReaders   = SCARD_AUTOALLOCATE;
    LPTSTR pszReaders = nullptr;
    
    LONG Result = SCardListReaders(Context, nullptr, (LPTSTR)&pszReaders, &dwReaders);
    if (Result != SCARD_S_SUCCESS || !pszReaders || *pszReaders == TEXT('\0'))
    {
        if (pszReaders) SCardFreeMemory(Context, pszReaders);
        return false;
    }
    
    OutReaderName = FString(pszReaders);
    SCardFreeMemory(Context, pszReaders);
    return true;
}

FString TTL_PCSCReader::GetCardUID(const FString& ReaderName)
{
    SCARDHANDLE hCard = 0;
    DWORD dwActiveProtocol = 0;
    
    LONG ConnResult = SCardConnect(
        Context,
        *ReaderName,
        SCARD_SHARE_SHARED,
        SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
        &hCard,
        &dwActiveProtocol
    );
    
    if (ConnResult != SCARD_S_SUCCESS)
    {
        return FString();
    }
    
    const uint8 GetUIDApdu[] = { 0xFF, 0xCA, 0x00, 0x00, 0x00 };
    uint8 Response[256] = { 0 };
    DWORD ResponseLen  = sizeof(Response);
    
    const SCARD_IO_REQUEST* pioSendPCI = dwActiveProtocol == SCARD_PROTOCOL_T0 ? SCARD_PCI_T0 : SCARD_PCI_T1;
    
    LONG TransmitResult = SCardTransmit(
        hCard,
        pioSendPCI,
        GetUIDApdu,
        sizeof(GetUIDApdu),
        nullptr,
        Response,
        &ResponseLen
    );
    
    SCardDisconnect(hCard, SCARD_LEAVE_CARD);
    
    if (TransmitResult != SCARD_S_SUCCESS || ResponseLen < 3)
    {
        return FString();
    }
    
    const uint8 SW1 = Response[ResponseLen - 2];
    const uint8 SW2 = Response[ResponseLen - 1];
    
    if (SW1 != 0x90 || SW2 != 0x00)
    {
        UE_LOG(LogTTLGameplay, Verbose,
            TEXT("TTL_PCSCReader: GET UID returned status %02X %02X"), SW1, SW2);
        return FString();
    }
    
    const DWORD UIDLength = ResponseLen - 2;
    return BytesToHexString(Response, UIDLength);
}

FString TTL_PCSCReader::BytesToHexString(const uint8* Data, DWORD Length) const
{
    FString Result;
    Result.Reserve(Length * 3);
    
    for (DWORD i = 0; i < Length; ++i)
    {
        if (i > 0) Result += TEXT(":");
        Result += FString::Printf(TEXT("%02X"), Data[i]);
    }
    return Result;
}
