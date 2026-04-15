#include "HistogramManager.hh"
#include <TROOT.h>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp> // JSONライブラリ

using json = nlohmann::json;

// グローバルインスタンスの実体
HistogramManager* gHistoManager = nullptr;

HistogramManager::HistogramManager() {
  gHistoManager = this;
}

HistogramManager::~HistogramManager() {
  std::cout<<"~HistogramManager"<<std::endl;
  for (auto& pair : fHistograms) {
    if (pair.second) delete pair.second;
  }
}

bool HistogramManager::Init(const std::string& configPath) {
  LoadJSON(configPath);
  return true;
}

void HistogramManager::LoadJSON(const std::string& path) {
  std::ifstream ifs(path);
  if (!ifs.is_open()) return;

  try {
    json j;
    ifs >> j;
    for (auto& [name, cfg] : j.items()) {
      fConfigMap[name] = {cfg["bins"], {cfg["min"], cfg["max"]}};
    }
    std::cout << "[HistogramManager] Loaded configuration for " << fConfigMap.size() << " histograms." << std::endl;
  } catch (std::exception& e) {
    std::cerr << "[HistogramManager] JSON Parse Error: " << e.what() << std::endl;
  }
}

TH1* HistogramManager::GetTH1(const std::string& name, const std::string& title, 
			      int bins, double min, double max, const std::string& folder) {
    
  // すでに存在すればそれを返す
  if (fHistograms.count(name)) return fHistograms[name];

  // JSON設定があれば上書き
  if (fConfigMap.count(name)) {
    bins = fConfigMap[name].first;
    min = fConfigMap[name].second.first;
    max = fConfigMap[name].second.second;
  }

  // THttpServerで見えるようにROOTのディレクトリ構造に登録
//  gROOT->mkdir(folder.c_str());
//  gROOT->cd(folder.c_str());

  TH1F* h = new TH1F(name.c_str(), title.c_str(), bins, min, max);
  fHistograms[name] = h;
    
  return h;
}

void HistogramManager::ResetAll() {
  std::cout << "[HistogramManager] Resetting all histograms..." << std::endl;
  for (auto& pair : fHistograms) {
    pair.second->Reset();
  }
}

void HistogramManager::PrintListOfHistograms() {

  std::cout<<"HistogramManager::PrintListOfHistograms()"<<std::endl;
  for (auto& pair : fHistograms) {
    std::cout<<pair.second->GetName()<<std::endl;
  }
}
