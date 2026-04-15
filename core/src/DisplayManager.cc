#include "DisplayManager.hh"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <cmath>

using json = nlohmann::json;

DisplayManager* gDispManager = nullptr;

DisplayManager::DisplayManager() : fTotalPages(0), fHistsPerPage(9) {
}

DisplayManager::~DisplayManager() {
}

bool DisplayManager::Init(const std::string& configPath) {
  std::ifstream ifs(configPath);
  if (!ifs.is_open()) return false;

  try {
    json j;
    ifs >> j;
        
    // config.json 内の "display": { "hists_per_page": 9 } などを読み込む
    if (j.contains("display")) {
      fHistsPerPage = j["display"].value("hists_per_page", 9);
    }

    // 表示対象の全ヒストグラム名を取得
    std::vector<std::string> allHists;
    for (auto& det : j["detectors_config"]) {
      for (auto& hName : det["histograms"]) {
	allHists.push_back(hName);
      }
    }

    // ページ分割ロジック
    fTotalPages = std::ceil((double)allHists.size() / fHistsPerPage);
    fPages.clear();
        
    for (int i = 0; i < fTotalPages; ++i) {
      PageConfig pg;
      for (int j = 0; j < fHistsPerPage; ++j) {
	int idx = i * fHistsPerPage + j;
	if (idx < allHists.size()) {
	  pg.histogramNames.push_back(allHists[idx]);
	}
      }
      fPages.push_back(pg);
    }

    std::cout << "[DisplayManager] Configured " << fTotalPages << " pages." << std::endl;
    return true;

  } catch (std::exception& e) {
    std::cerr << "[DisplayManager] Error: " << e.what() << std::endl;
    return false;
  }
}

std::string DisplayManager::GetLayoutJSON() {
  json j;
  j["total_pages"] = fTotalPages;
  j["hists_per_page"] = fHistsPerPage;
    
  for (int i = 0; i < fPages.size(); ++i) {
    j["pages"][i] = fPages[i].histogramNames;
  }
    
  return j.dump();
}
