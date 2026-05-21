// Fill out your copyright notice in the Description page of Project Settings.


#include "Reader/TTL_PCSCReader.h"


TTL_PCSCReader::TTL_PCSCReader() {}

TTL_PCSCReader::~TTL_PCSCReader()
{
    Stop();
}

bool TTL_PCSCReader::Start()
{
    return false;
}

void TTL_PCSCReader::Stop()
{
    bRunning = false;
    
    if (Thread)
    {
        Thread->WaitForCompletion();
        delete Thread;
        Thread = nullptr;
    }
    bConnected = false;
    
    FRunnable::Stop();
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
    return FRunnable::Init();
}

uint32 TTL_PCSCReader::Run()
{
    return -1;
}

void TTL_PCSCReader::Exit()
{
    FRunnable::Exit();
}

bool TTL_PCSCReader::EstablishContext()
{
    return false;
}

void TTL_PCSCReader::ReleaseContext()
{
}

bool TTL_PCSCReader::FindFirstReader(FString& OutReaderName) const
{
    return false;
}

FString TTL_PCSCReader::GetCardUID(const FString& ReaderName)
{
    return FString();
}

FString TTL_PCSCReader::BytesToHexString(const uint8* Data, DWORD Length) const
{
    return FString();
}
