#include "ConfigManager.hh"
#include <fstream>

ConfigManager* ConfigManager::GetInstance() {
    static ConfigManager instance;
    return &instance;
}

bool ConfigManager::LoadConfig(const std::string& path) {
    fLastError = "";
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        fLastError = "Main config not found: " + path;
        return false;
    }

    try {
        ifs >> fConfig;
        
        // subfile for histogram ranges
        if (fConfig.contains("hist_range_file")) {
            std::string subPath = fConfig["hist_range_file"];
            std::ifstream subIfs(subPath);
            if (subIfs.is_open()) {
                nlohmann::json subJson;
                subIfs >> subJson;
                fConfig["ranges"] = subJson;
            } else {
                fLastError = "Range file not found: " + subPath;
            }
        }
    } catch (const std::exception& e) {
        fLastError = "JSON Error: " + std::string(e.what());
    }
    return true;
}
