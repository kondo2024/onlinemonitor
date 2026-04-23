#include "ConfigManager.hh"
#include <fstream>
#include <iostream>

ConfigManager* ConfigManager::GetInstance()
{
  static ConfigManager instance;
  return &instance;
}

bool ConfigManager::LoadConfig(const std::string& path) {
  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    std::cout<<"Error: Cannot open file: " + path<<std::endl;
    return false;
  }

  try {
    nlohmann::json mainJson;
    ifs >> mainJson;

    if (mainJson.contains("hist_range_file")) {
      std::string subPath = mainJson["hist_range_file"];
      std::ifstream subIfs(subPath);
      if (subIfs.is_open()) {
        nlohmann::json subJson;
        subIfs >> subJson;
        mainJson["ranges"] = std::move(subJson);
      }
    }
    
    fConfig = std::move(mainJson);
    // for THttpServer
    fConfigContents.SetName("Contents");
    fConfigContents.SetTitle(fConfig.dump().c_str());

  } catch (const std::exception& e) {
    std::cout << "Error: [ConfigManager] JSON Error: " << e.what() << std::endl;
    return false;
  }

  fFilename = path;
  if (!fIsLoaded) std::cout<<"[ConfigManager] Loaded: "<<fFilename<<std::endl;
  fIsLoaded = true;
  return true;

}

