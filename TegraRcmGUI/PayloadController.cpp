#include "PayloadController.h"
#include "TegraRcmSmash.h"
#include <setupapi.h>
#include <shellapi.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <Shlwapi.h>

#pragma comment (lib, "setupapi.lib")
#pragma comment (lib, "shlwapi.lib")

PayloadController::PayloadController() : m_waitingReconnect(false) {}

PayloadController::~PayloadController() {}

void PayloadController::Log(const std::wstring& message) {
    if (m_logCallback) {
        m_logCallback(message);
    }
}

std::wstring PayloadController::GetAppDirectory() {
    wchar_t szPath[MAX_PATH];
    GetModuleFileNameW(NULL, szPath, MAX_PATH);
    PathRemoveFileSpecW(szPath);
    return std::wstring(szPath);
}

int PayloadController::GetRcmStatus() {
    return TegraRcmSmash::RcmStatus();
}

bool PayloadController::LookForAPXDevice() {
    unsigned index;
    HDEVINFO hDevInfo = SetupDiGetClassDevs(NULL, L"USB", NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
    if (hDevInfo == INVALID_HANDLE_VALUE) return false;

    SP_DEVINFO_DATA DeviceInfoData;
    wchar_t HardwareID[1024];

    for (index = 0; ; index++) {
        DeviceInfoData.cbSize = sizeof(DeviceInfoData);
        if (!SetupDiEnumDeviceInfo(hDevInfo, index, &DeviceInfoData)) {
            break;
        }
        SetupDiGetDeviceRegistryPropertyW(hDevInfo, &DeviceInfoData, SPDRP_HARDWAREID, NULL, (BYTE*)HardwareID, sizeof(HardwareID), NULL);
        if (wcsstr(HardwareID, L"VID_0955&PID_7321")) {
            SetupDiDestroyDeviceInfoList(hDevInfo);
            return true;
        }
    }
    SetupDiDestroyDeviceInfoList(hDevInfo);
    return false;
}

void PayloadController::InstallDriver() {
    SHELLEXECUTEINFOW shExInfo = { 0 };
    shExInfo.cbSize = sizeof(shExInfo);
    shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shExInfo.lpVerb = L"runas";

    std::wstring exe_file = GetAppDirectory() + L"\\apx_driver\\InstallDriver.exe";
    
    shExInfo.lpFile = exe_file.c_str();
    shExInfo.nShow = SW_SHOW;

    if (ShellExecuteExW(&shExInfo)) {
        CloseHandle(shExInfo.hProcess);
        Log(L"Driver installation started.");
    } else {
        Log(L"Failed to start driver installation.");
    }
}

int PayloadController::InjectPayload(const std::wstring& payloadPath) {
    if (m_waitingReconnect) {
        // UI should prompt the user, here we just assume it's unsafe or we log it
        Log(L"Waiting for device reconnect before injecting.");
        return -99;
    }

    std::wstring smasherPath = GetAppDirectory() + L"\\TegraRcmSmash.exe";
    std::wstring args = L"\"" + payloadPath + L"\"";
    std::wstring cmdLine = smasherPath + L" " + args;

    wchar_t cmd[4096];
    wcscpy_s(cmd, 4096, cmdLine.c_str());

    PROCESS_INFORMATION pi;
    STARTUPINFOW si;
    ZeroMemory(&pi, sizeof(pi));
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;

    Log(L"Invoking TegraRcmSmash.exe with args: " + args);

    BOOL ret = CreateProcessW(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    int rc = -50;
    
    if (ret) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exit_code;
        if (GetExitCodeProcess(pi.hProcess, &exit_code)) {
            rc = (exit_code != STILL_ACTIVE) ? exit_code : -52;
        } else {
            rc = -51;
        }
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        
        if (rc >= 0) {
            m_waitingReconnect = true;
            Log(L"Payload successfully injected.");
        } else {
            Log(L"Injection failed with code: " + std::to_wstring(rc));
        }
    } else {
        DWORD err = GetLastError();
        Log(L"CreateProcess failed with error: " + std::to_wstring(err));
    }

    if (m_statusCallback) {
        m_statusCallback(rc);
    }
    return rc;
}
