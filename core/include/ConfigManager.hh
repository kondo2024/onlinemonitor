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
  bool ReloadConfig(){return LoadConfig(fFilename.c_str());}

//  bool HasError() const { return !fLastError.empty(); }
//  std::string GetLastError() const { return fLastError; }

  const nlohmann::json& GetJson() const { return fConfig; }

  // check if key exists
  bool Contains(const std::string& key, const std::string& subkey = "") const {
    if (subkey.empty()) return fConfig.contains(key);
    return fConfig.contains(key) && fConfig[key].contains(subkey);
  }

  const std::string GetConfigPath(){return fFilename;}

  TNamed* GetConfigContentsPtr(){return &fConfigContents;}
  
private:
  ConfigManager() = default;
  std::string fFilename;
  nlohmann::json fConfig;
  TNamed fConfigContents;
  //std::string fLastError;
  bool fIsLoaded = false;
};

#endif
