#pragma once
#include <string>
#include <vector>
#include <map>

class SettingsManager
{
public:
    // Pass the AppData directory or local directory from the UI (Qt frontend)
    SettingsManager(const std::wstring& configDirectory);
    ~SettingsManager();

    // Presets (presets.conf)
    std::string GetPreset(const std::string& param);
    void SetPreset(const std::string& param, const std::string& value);

    // Favorites (favorites.conf)
    std::vector<std::wstring> GetFavorites();
    void AddFavorite(const std::wstring& absolutePath);
    void SaveFavorites(const std::vector<std::wstring>& favorites);

private:
    std::wstring m_configDirectory;
    
    std::wstring GetPresetsFilePath() const;
    std::wstring GetFavoritesFilePath() const;
    
    // Utilities mimicking the original GetRelativeFilename, but using Win32 API
    std::wstring GetRelativePath(const std::wstring& absolutePath) const;
    std::wstring GetAbsolutePath(const std::wstring& relativePath) const;
};
