#include "ConfigManager.hh"
#include <fstream>
#include <iostream>

ConfigManager* ConfigManager::GetInstance() {
    static ConfigManager instance;
    return &instance;
}

bool ConfigManager::LoadConfig(const std::string& path) {
//  fFilename = path;
//  fLastError = "";
//  std::ifstream ifs(path);
//  if (!ifs.is_open()) {
//    fLastError = "Main config not found: " + path;
//    std::cout<<"Main config not found: " + path<<std::endl;
//    return false;
//  }
//
//  try {
//    ifs >> fConfig;
//        
//    // subfile for histogram ranges
//    if (fConfig.contains("hist_range_file")) {
//      std::string subPath = fConfig["hist_range_file"];
//      std::ifstream subIfs(subPath);
//      if (subIfs.is_open()) {
//	nlohmann::json subJson;
//	subIfs >> subJson;
//	fConfig["ranges"] = subJson;
//      } else {
//	fLastError = "Range file not found: " + subPath;
//	std::cout<<"Range file not found: " + subPath<<std::endl;
//      }
//    }
//  } catch (const std::exception& e) {
//    fLastError = "JSON Error: " + std::string(e.what());
//    std::cout<<"JSON Error: " + std::string(e.what()) <<std::endl;
//  }
//  return true;


// 文字列代入を一旦止めてみる（原因の切り分け）
  // fFilename = path; 
  
  std::ifstream ifs(path);
  if (!ifs.is_open()) return false;

  try {
    nlohmann::json mainJson;
    ifs >> mainJson;

    if (mainJson.contains("hist_range_file")) {
      std::string subPath = mainJson["hist_range_file"];
      std::ifstream subIfs(subPath);
      if (subIfs.is_open()) {
        nlohmann::json subJson;
        subIfs >> subJson;
        mainJson["ranges"] = std::move(subJson); // moveを使って所有権を移動
      }
    }
    
    // 最後に一回だけメンバ変数に代入
    fConfig = std::move(mainJson);

  } catch (const std::exception& e) {
    std::cout << "JSON Error: " << e.what() << std::endl;
    return false;
  }
  return true;

}

