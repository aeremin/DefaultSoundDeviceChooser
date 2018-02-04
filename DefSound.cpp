// ----------------------------------------------------------------------------
// DefSound.cpp
// DefSound Application
// ----------------------------------------------------------------------------

#include "stdafx.h"
#include <iostream>

#include "DefSoundCommandLine.h"


namespace DefSound {
namespace {

const PCWSTR g_wszApplicationName = L"DefSound: Select Default Sound Render Device";

// ----------------------------------------------------------------------------

INT RunCommandLineMode(const CCommandLine &CommandLine)
{
    CEndpointCollection EndpointCollection;

    CCommandLine::CArgumentsPtr pArguments;
    std::wstring ErrorMessage;
    if (!CommandLine.Parse(EndpointCollection, pArguments, ErrorMessage))
    {
        return STATUS_INVALID_PARAMETER;
    }
    _ASSERT(pArguments.get());

    if (EndpointCollection.Get().empty())
        throw CError( L"There are no audio endpoints", ERROR_SUCCESS );

    if (pArguments->m_DeviceIndexes.empty())
    {
        const auto nEndpointIndex = 
            EndpointCollection.SetDefaultNext(pArguments->m_Role);
        std::wcout << EndpointCollection.Get()[nEndpointIndex].m_DeviceDesc << " --- " << EndpointCollection.Get()[nEndpointIndex].m_DeviceId;
    }
    else
    {
        for (const auto nIndex : pArguments->m_DeviceIndexes)
        {
            try
            {
                EndpointCollection.SetDefault(nIndex, pArguments->m_Role);
                std::wcout << EndpointCollection.Get()[nIndex].m_DeviceDesc << " --- " << EndpointCollection.Get()[nIndex].m_DeviceId;
                break;
            }
            catch (const CError &Error)
            {
                UNREFERENCED_PARAMETER(Error);
            }
        }
    }

    return ERROR_SUCCESS;
}

// ----------------------------------------------------------------------------

int ShowError(const CError &Error)
{
    std::wstringstream Stream;
    Stream << Error.m_Description << std::endl;

    static_assert(ERROR_SUCCESS == S_OK, "ERROR_SUCCESS == S_OK");
    static_assert(ERROR_SUCCESS == NO_ERROR, "ERROR_SUCCESS == NO_ERROR");
    if (Error.m_nErrorCode != ERROR_SUCCESS)
    {
        if (static_cast<long>(Error.m_nErrorCode) < 0)
        {
            Stream << L"Error 0x" << std::hex << Error.m_nErrorCode << std::endl;
        }
        else
        {
            Stream << L"Error " << std::dec << Error.m_nErrorCode << std::endl;
        }

        PWSTR wszMessage = nullptr;
        const auto FormatResult =
            ::FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                nullptr,
                Error.m_nErrorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<PWSTR>(&wszMessage),
                0,
                nullptr);
        if (FormatResult)
        {
            std::unique_ptr<WCHAR, decltype(&::LocalFree)> MessageHolder(wszMessage, &::LocalFree);
            Stream << wszMessage;
        }
    }

    ::MessageBox(nullptr, Stream.str().c_str(), g_wszApplicationName, MB_ICONERROR | MB_OK);
    return (Error.m_nErrorCode != ERROR_SUCCESS) ? Error.m_nErrorCode : ERROR_INTERNAL_ERROR;
}

// ----------------------------------------------------------------------------

int WinMainImpl()
{
    try
    {
        struct CCoInitialize
        {
            CCoInitialize()
            {
                HRESULT Result = ::CoInitialize(nullptr);
                if (S_OK != Result)
                    throw CError( MakeDefaultErrorDescription(L"::CoInitialize"), Result );
            }
            ~CCoInitialize()
            {
                ::CoUninitialize();
            }
        };
        CCoInitialize CoInitializeScoped;

        CCommandLine CommandLine;
        return RunCommandLineMode(CommandLine);
    }
    catch (const CError &Error)
    {
        return ShowError(Error);
    }
}

}   // namespace <nameless>
}   // namespace DefSound

// ----------------------------------------------------------------------------

int WINAPI WinMain(
    __in HINSTANCE,
    __in_opt HINSTANCE,
    __in_opt LPSTR,
    __in int)
{
    return DefSound::WinMainImpl();
}


// ----------------------------------------------------------------------------
