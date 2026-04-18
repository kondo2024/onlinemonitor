#ifndef CONFIG_MANAGER_HH
#define CONFIG_MANAGER_HH

#include <string>
#include <iostream>
#include <nlohmann/json.hpp>

class ConfigManager {
public:
    static ConfigManager* GetInstance();

    bool LoadConfig(const std::string& path);

    bool HasError() const { return !fLastError.empty(); }
    std::string GetLastError() const { return fLastError; }

    const nlohmann::json& GetJson() const { return fConfig; }

    // check if key exists
    bool Contains(const std::string& key, const std::string& subkey = "") const {
        if (subkey.empty()) return fConfig.contains(key);
        return fConfig.contains(key) && fConfig[key].contains(subkey);
    }

private:
    ConfigManager() = default;
    nlohmann::json fConfig;
    std::string fLastError;
};

#endif
