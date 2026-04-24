#ifndef CONFIG_MANAGER_HH
#define CONFIG_MANAGER_HH

#include <string>
#include <iostream>
#include <nlohmann/json.hpp>
#include <TNamed.h>

class ConfigManager {
public:
  static ConfigManager* GetInstance();

  bool LoadConfig(const std::string& path);
  bool ReloadConfig(){return LoadConfig(fFilename);}

  const nlohmann::json& GetJson() const { return fConfig; }

  // check if key exists
  bool Contains(const std::string& key, const std::string& subkey = "") const {
    if (subkey.empty()) return fConfig.contains(key);
    return fConfig.contains(key) && fConfig[key].is_object() && fConfig[key].contains(subkey);
  }

  const std::string& GetConfigPath(){return fFilename;}

  TNamed* GetConfigContentsPtr(){return &fConfigContents;}
  
private:
  ConfigManager();
  std::string fFilename;
  nlohmann::json fConfig;
  TNamed fConfigContents{"Contents", ""};
  bool fIsLoaded = false;
};

#endif
