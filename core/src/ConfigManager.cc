#include "ConfigManager.hh"
#include <fstream>
#include <iostream>

ConfigManager::ConfigManager() : fConfigContents("Contents", "") {}

ConfigManager* ConfigManager::GetInstance()
{
  static ConfigManager instance;
  return &instance;
}

bool ConfigManager::LoadConfig(const std::string& path) {
  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    std::cout<<"[ConfigManager] Error: Cannot open file: " + path<<std::endl;
    return false;
  }

  try {
    nlohmann::json mainJson;
    ifs >> mainJson;

    if (mainJson.contains("hist_range_file")) {
      std::string subPath = mainJson["hist_range_file"];

      std::filesystem::path mainPath(path);
      std::filesystem::path subFilePath = mainPath.parent_path() / subPath;

      std::ifstream subIfs(subFilePath.string());
      if (subIfs.is_open()) {
        nlohmann::json subJson;
        subIfs >> subJson;
        mainJson["ranges"] = std::move(subJson);
      }
    }
    
    fConfig = std::move(mainJson);
    // for THttpServer
    std::string dump = fConfig.dump();
    fConfigContents.SetTitle(dump.c_str());

  } catch (const std::exception& e) {
    std::cerr << "Error: [ConfigManager] JSON Parse Error: " << e.what() << std::endl;
    return false;
  }

  fFilename = path;
  if (!fIsLoaded) std::cout<<"[ConfigManager] Loaded: "<<fFilename<<std::endl;
  fIsLoaded = true;
  return true;

}

