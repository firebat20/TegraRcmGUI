#include "SettingsManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <windows.h>
#include <Shlwapi.h>
#include <locale>

#pragma comment(lib, "Shlwapi.lib")

SettingsManager::SettingsManager(const std::wstring& configDirectory)
    : m_configDirectory(configDirectory)
{
}

// UTF-8 / Wide conversion helpers
static std::wstring Utf8ToWide(const std::string& utf8);
static std::string WideToUtf8(const std::wstring& wstr);

SettingsManager::~SettingsManager()
{
}

std::wstring SettingsManager::GetPresetsFilePath() const
{
    return m_configDirectory + L"\\presets.conf";
}

std::wstring SettingsManager::GetFavoritesFilePath() const
{
    return m_configDirectory + L"\\favorites.conf";
}

std::string SettingsManager::GetPreset(const std::string& param)
{
    std::string presetsPathUtf8 = WideToUtf8(GetPresetsFilePath());
    std::ifstream readFile(presetsPathUtf8, std::ios::in | std::ios::binary);
    std::string readout;
    std::string search = param + "=";
    std::string value = "";

    if (readFile.is_open())
    {
        while (std::getline(readFile, readout)) {
            if (readout.find(search) != std::string::npos) {
                std::string delimiter = "=";
                value = readout.substr(readout.find(delimiter) + 1);
            }
        }
    }
    return value;
}

void SettingsManager::SetPreset(const std::string& param, const std::string& value)
{
    std::wstring rfile = GetPresetsFilePath();
    std::wstring wfile = m_configDirectory + L"\\presets.conf.tmp";
    std::string wfileUtf8 = WideToUtf8(wfile);
    std::string rfileUtf8 = WideToUtf8(rfile);
    std::ofstream outFile(wfileUtf8, std::ios::out | std::ios::binary);
    std::ifstream readFile(rfileUtf8, std::ios::in | std::ios::binary);

    std::string readout;
    std::string search = param + "=";
    std::string replace = search + value + "\n";
    bool found = false;

    if (readFile.is_open()) {
        while (std::getline(readFile, readout)) {
            if (readout.find(search) != std::string::npos) {
                outFile << replace;
                found = true;
            }
            else if (readout.size() > 0) {
                outFile << readout << "\n";
            }
        }
    }

    if (!found) {
        outFile << replace;
    }

    outFile.flush();
    outFile.close();
    readFile.close();

    // Atomically replace the original presets file with the temp file
    const int maxRetries = 3;
    int attempt = 0;
    BOOL moved = FALSE;
    while (attempt < maxRetries && !moved) {
        moved = MoveFileExW(wfile.c_str(), rfile.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
        if (!moved) {
            Sleep(50);
            attempt++;
        }
    }

    if (!moved) {
        // Replacement failed; clean up temp file and leave original intact
        ::DeleteFileW(wfile.c_str());
    }
}

std::wstring SettingsManager::GetRelativePath(const std::wstring& absolutePath) const
{
    wchar_t szOut[MAX_PATH] = L"";
    
    // Gets current executing directory (to mirror portable logic)
    wchar_t szModPath[MAX_PATH];
    GetModuleFileNameW(NULL, szModPath, MAX_PATH);
    PathRemoveFileSpecW(szModPath);

    if (PathRelativePathToW(szOut, szModPath, FILE_ATTRIBUTE_DIRECTORY, absolutePath.c_str(), FILE_ATTRIBUTE_NORMAL)) {
        return std::wstring(szOut);
    }
    return absolutePath;
}

std::wstring SettingsManager::GetAbsolutePath(const std::wstring& relativePath) const
{
    wchar_t szModPath[MAX_PATH];
    GetModuleFileNameW(NULL, szModPath, MAX_PATH);
    PathRemoveFileSpecW(szModPath);

    wchar_t szOut[MAX_PATH] = L"";
    PathCombineW(szOut, szModPath, relativePath.c_str());
    
    // Canonicalize the path to remove ..\ segments
    wchar_t szCanonical[MAX_PATH] = L"";
    PathCanonicalizeW(szCanonical, szOut);
    
    return std::wstring(szCanonical);
}

std::vector<std::wstring> SettingsManager::GetFavorites()
{
    std::vector<std::wstring> favoritesListing;
    std::string favPathUtf8 = WideToUtf8(GetFavoritesFilePath());
    std::ifstream readFile(favPathUtf8, std::ios::in | std::ios::binary);
    std::string readout;

    if (readFile.is_open()) {
        while (std::getline(readFile, readout)) {
            if (readout.empty()) continue;
            // Strip trailing \r from Windows line endings
            if (readout.back() == '\r')
                readout.pop_back();
            if (readout.empty()) continue;
            // convert UTF-8 line to wide
            std::wstring wline = Utf8ToWide(readout);
            // If it's a relative path (doesn't contain ':'), make it absolute
            if (wline.find(L":") == std::wstring::npos) {
                favoritesListing.push_back(GetAbsolutePath(wline));
            } else {
                favoritesListing.push_back(wline);
            }
        }
    }
    return favoritesListing;
}

void SettingsManager::AddFavorite(const std::wstring& absolutePath)
{
    std::wstring relativeFav = GetRelativePath(absolutePath);
    std::string favPathUtf8 = WideToUtf8(GetFavoritesFilePath());
    std::ofstream outFile(favPathUtf8, std::ios::out | std::ios::app | std::ios::binary);
    if (outFile.is_open()) {
        std::string utf8 = WideToUtf8(relativeFav);
        outFile << utf8 << "\n";
        outFile.flush();
    }
}

void SettingsManager::SaveFavorites(const std::vector<std::wstring>& favorites)
{
    std::wstring rfile = GetFavoritesFilePath();
    std::wstring tmpfile = rfile + L".tmp";

    // Write to temp file first
    std::string tmpPathUtf8 = WideToUtf8(tmpfile);
    std::ofstream outFile(tmpPathUtf8, std::ios::out | std::ios::binary);
    if (!outFile.is_open()) {
        // can't open temp file; abort
        return;
    }

    for (const auto& fav : favorites) {
        std::wstring relativeFav = GetRelativePath(fav);
        std::string utf8 = WideToUtf8(relativeFav);
        outFile << utf8 << "\n";
    }
    outFile.flush();
    outFile.close();

    // Atomically replace original with temp
    // Try a few times in case of transient locks
    const int maxRetries = 3;
    int attempt = 0;
    BOOL moved = FALSE;
    while (attempt < maxRetries && !moved) {
        moved = MoveFileExW(tmpfile.c_str(), rfile.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
        if (!moved) {
            Sleep(50);
            attempt++;
        }
    }
    if (!moved) {
        // cleanup temp file if replace failed
        ::DeleteFileW(tmpfile.c_str());
    }
}

// Helper: convert UTF-8 std::string to std::wstring
static std::wstring Utf8ToWide(const std::string& utf8)
{
    if (utf8.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), NULL, 0);
    if (size_needed <= 0) return std::wstring();
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &wstr[0], size_needed);
    return wstr;
}

// Helper: convert std::wstring to UTF-8 std::string
static std::string WideToUtf8(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
    if (size_needed <= 0) return std::string();
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, NULL, NULL);
    return str;
}
