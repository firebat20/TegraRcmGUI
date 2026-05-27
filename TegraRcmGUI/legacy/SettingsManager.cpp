#include "SettingsManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <windows.h>
#include <Shlwapi.h>
#include <locale>
#include <codecvt>

#pragma comment(lib, "Shlwapi.lib")

SettingsManager::SettingsManager(const std::wstring& configDirectory)
    : m_configDirectory(configDirectory)
{
}

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
    std::ifstream readFile(GetPresetsFilePath());
    std::string readout;
    std::string search = param + "=";
    std::string value = "";

    if (readFile.is_open())
    {
        while (std::getline(readFile, readout)) {
            if (readout.find(search) != std::string::npos) {
                std::string delimiter = "=";
                value = readout.substr(readout.find(delimiter) + 1, readout.length() + 1);
            }
        }
    }
    return value;
}

void SettingsManager::SetPreset(const std::string& param, const std::string& value)
{
    std::wstring rfile = GetPresetsFilePath();
    std::wstring wfile = m_configDirectory + L"\\presets.conf.tmp";

    std::ofstream outFile(wfile);
    std::ifstream readFile(rfile);
    
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
    
    outFile.close();
    readFile.close();

    _wremove(rfile.c_str());
    _wrename(wfile.c_str(), rfile.c_str());
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
    std::wifstream readFile(GetFavoritesFilePath());
    readFile.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
    std::wstring readout;

    if (readFile.is_open()) {
        while (std::getline(readFile, readout)) {
            if (readout.empty()) continue;

            // If it's a relative path (doesn't contain ':'), make it absolute
            if (readout.find(L":") == std::wstring::npos) {
                favoritesListing.push_back(GetAbsolutePath(readout));
            } else {
                favoritesListing.push_back(readout);
            }
        }
    }
    return favoritesListing;
}

void SettingsManager::AddFavorite(const std::wstring& absolutePath)
{
    std::wstring relativeFav = GetRelativePath(absolutePath);
    
    std::wofstream outFile;
    outFile.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
    outFile.open(GetFavoritesFilePath(), std::ios::out | std::ios::app);
    
    if (outFile.is_open()) {
        outFile << relativeFav << L"\n";
    }
}

void SettingsManager::SaveFavorites(const std::vector<std::wstring>& favorites)
{
    std::wstring rfile = GetFavoritesFilePath();
    _wremove(rfile.c_str());
    
    for (const auto& fav : favorites) {
        AddFavorite(fav);
    }
}
