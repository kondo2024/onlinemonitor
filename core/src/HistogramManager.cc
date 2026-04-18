#include "HistogramManager.hh"
#include <TROOT.h>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <TH1.h>
#include <TH2.h>

using json = nlohmann::json;

HistogramManager* gHistManager = nullptr;

HistogramManager::HistogramManager() {
  gHistManager = this;
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
    
  if (fHistograms.count(name)) return fHistograms[name];

  if (fConfigMap.count(name)) {
    bins = fConfigMap[name].first;
    min = fConfigMap[name].second.first;
    max = fConfigMap[name].second.second;
  }

  TH1* h = nullptr;
  
  if (!fHistograms.count(name)) {
//  gROOT->mkdir(folder.c_str());
//  gROOT->cd(folder.c_str());
    h = new TH1F(name.c_str(), title.c_str(), bins, min, max);
    fHistograms[name] = h;

  } else {// if hist exists
    h = fHistograms[name];
    TH2* h2 = dynamic_cast<TH2*>(h);

    if (h2) {
      // temptemptemp
      if (h2->GetNbinsX() != bins || h2->GetXaxis()->GetXmin() != min) {
        std::cout << "[HistogramManager] Updating TH2 bins for: " << name << std::endl;
        // TH2::SetBins(nx, xmin, xmax, ny, ymin, ymax)
        h2->SetBins(bins, min, max, bins, min, max);
        h2->Reset("ICES");
      }
    } else {
      if (h->GetNbinsX() != bins || h->GetXaxis()->GetXmin() != min || h->GetXaxis()->GetXmax() != max) {
        std::cout << "[HistogramManager] Updating TH1 bins for: " << name << std::endl;
        // TH1::SetBins(nx, xmin, xmax)
        h->SetBins(bins, min, max);
        h->Reset("ICES");
      }
    }
  }

  return h;
}

void HistogramManager::ResetAll() {
  std::cout << "[HistogramManager] Resetting all histograms..." << std::endl;
  for (auto& pair : fHistograms) {
    pair.second->Reset("ICES");
  }
}

void HistogramManager::PrintListOfHistograms() {

  std::cout<<"HistogramManager::PrintListOfHistograms()"<<std::endl;
  for (auto& pair : fHistograms) {
    std::cout<<pair.second->GetName()<<std::endl;
  }
}
