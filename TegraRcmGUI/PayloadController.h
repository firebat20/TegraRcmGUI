#pragma once
#include <string>
#include <vector>
#include <functional>
#include <windows.h>

class PayloadController
{
public:
    PayloadController();
    ~PayloadController();

    // Callback types to notify UI
    using LogCallback = std::function<void(const std::wstring&)>;
    using StatusCallback = std::function<void(int)>;

    // Set callbacks
    void SetLogCallback(LogCallback cb) { m_logCallback = cb; }
    void SetStatusCallback(StatusCallback cb) { m_statusCallback = cb; }

    // Backend actions
    int GetRcmStatus();
    bool LookForAPXDevice();
    int InjectPayload(const std::wstring& payloadPath);
    void InstallDriver();

private:
    void Log(const std::wstring& message);
    std::wstring GetAppDirectory();
    
    LogCallback m_logCallback;
    StatusCallback m_statusCallback;

    bool m_waitingReconnect;
};
