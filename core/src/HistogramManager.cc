#include "HistogramManager.hh"
#include "ConfigManager.hh"
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

TH1* HistogramManager::GetTH1(const std::string& name, const std::string& title, 
			      int bins, double min, double max, const std::string& folder) {
    
  TH1* h = nullptr;
  if (fHistograms.count(name)) h = fHistograms[name];
  else {
    h = new TH1F(name.c_str(), title.c_str(), bins, min, max);
    fHistograms[name] = h;
  }

  const auto& config = ConfigManager::GetInstance()->GetJson();
  if (config.contains("ranges") && config["ranges"].contains(name)) {
    auto& r = config["ranges"][name];
    h->SetBins(r["xbins"], r["xmin"], r["xmax"]);
    h->Reset("ICES");
    std::cout << "[HistogramManager] Updated TH1 bin/range for: " << name << std::endl;
  }
  return h;
}

TH1* HistogramManager::GetTH2(const std::string& name, const std::string& title, 
			      int xbins, double xmin, double xmax,
			      int ybins, double ymin, double ymax,
			      const std::string& folder) {
    
  TH1* h = nullptr;
  if (fHistograms.count(name)) h = fHistograms[name];
  else {
    h = new TH2F(name.c_str(), title.c_str(), xbins, xmin, xmax, ybins, ymin, ymax);
    fHistograms[name] = h;
  }

  const auto& config = ConfigManager::GetInstance()->GetJson();
  if (config.contains("ranges") && config["ranges"].contains(name)) {
    auto& r = config["ranges"][name];
  
    TH2* h2 = dynamic_cast<TH2*>(h);
    if (h2 && r.contains("ybins")) {
      h2->SetBins(r["xbins"], r["xmin"], r["xmax"], 
		  r["ybins"], r["ymin"], r["ymax"]);
      h2->Reset("ICES");
      std::cout << "[HistogramManager] Updated TH2 bins/ranges for: " << name << std::endl;
    }
  }
  return h;
}

void HistogramManager::ResetAll() {
  std::cout << "[HistogramManager] Resetting all histograms..." << std::endl;
  auto config = ConfigManager::GetInstance();
  config->LoadConfig("config/config.json");

  for (auto& pair : fHistograms) {
    TH2* h2 = dynamic_cast<TH2*>(pair.second);
    if (h2) GetTH2(pair.second->GetName());
    else    GetTH1(pair.second->GetName());
    pair.second->Reset("ICES");
  }
}

void HistogramManager::PrintListOfHistograms() {

  std::cout<<"HistogramManager::PrintListOfHistograms()"<<std::endl;
  for (auto& pair : fHistograms) {
    std::cout<<pair.second->GetName()<<std::endl;
  }
}
