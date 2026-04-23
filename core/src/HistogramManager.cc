#include "HistogramManager.hh"
#include "ConfigManager.hh"
#include <TROOT.h>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <TH1.h>
#include <TH2.h>

using json = nlohmann::json;

HistogramManager::HistogramManager()
  : fResetAllRequested(false)
{}

HistogramManager::~HistogramManager(){}

TH1* HistogramManager::BookTH1(const std::string& name, const std::string& title, 
			       int bins, double min, double max, const std::string& folder) {
    
  TH1* h = nullptr;
  if (fHistogramsMap.count(name)) {
    h = fHistogramsMap[name];
    std::cout<<h->GetName()<<" already exists"<<std::endl;
    return h;
  }

  
  h = new TH1F(name.c_str(), title.c_str(), bins, min, max);
  fHistogramsMap[name] = h;
  fHistograms.push_back(h);

  ChangeRangeTH1(h);

  return h;
}

TH1* HistogramManager::BookTH2(const std::string& name, const std::string& title, 
			      int xbins, double xmin, double xmax,
			      int ybins, double ymin, double ymax,
			      const std::string& folder) {
    
  TH1* h = nullptr;
  if (fHistogramsMap.count(name)) {
    h = fHistogramsMap[name];
    std::cout<<h->GetName()<<" already exists"<<std::endl;
    return h;
  }

  h = new TH2F(name.c_str(), title.c_str(), xbins, xmin, xmax, ybins, ymin, ymax);
  fHistogramsMap[name] = h;
  fHistograms.push_back(h);

  ChangeRangeTH2(h);
  
  return h;
}

void HistogramManager::ChangeRangeTH1(TH1* h) {
  
  const std::string name = h->GetName();
  const auto& config = ConfigManager::GetInstance()->GetJson();
  if (config.contains("ranges") && config["ranges"].contains(name)) {
    auto& r = config["ranges"][name];
    h->SetBins(r["xbins"], r["xmin"], r["xmax"]);
    h->Reset("ICES");
    std::cout << "[HistogramManager] Updated TH1 bin/range for: " << name << std::endl;
  }

}
    
void HistogramManager::ChangeRangeTH2(TH1* h){

  const std::string name = h->GetName();
  TH2* h2 = dynamic_cast<TH2*>(h);
  
  const auto& config = ConfigManager::GetInstance()->GetJson();
  if (config.contains("ranges") && config["ranges"].contains(name)) {
    auto& r = config["ranges"][name];
  
    if (h2 && r.contains("ybins")) {
      h2->SetBins(r["xbins"], r["xmin"], r["xmax"], 
		  r["ybins"], r["ymin"], r["ymax"]);
      h2->Reset("ICES");
      std::cout << "[HistogramManager] Updated TH2 bins/ranges for: " << name << std::endl;
    }
  }
}


TH1* HistogramManager::GetTH1(const std::string& name) {
  if (fHistogramsMap.count(name)) return fHistogramsMap[name];
  return nullptr;
}

TH2* HistogramManager::GetTH2(const std::string& name) {
  if (fHistogramsMap.count(name)) return dynamic_cast<TH2*>(fHistogramsMap[name]);
  return nullptr;
}


void HistogramManager::RequestResetAll() {
  fResetAllRequested = true;
}

void HistogramManager::ResetAll() {
  //std::cout << "[HistogramManager] Resetting all histograms..." << std::endl;
  auto config = ConfigManager::GetInstance();
  config->ReloadConfig();

  for (auto& h : fHistograms) {
    TH2* h2 = dynamic_cast<TH2*>(h);
    if (h2) ChangeRangeTH2(h);
    else    ChangeRangeTH1(h);
    h->Reset("ICES");
  }
  fResetAllRequested = false;
}

std::vector<std::string> HistogramManager::GetAllNames() {
  std::vector<std::string> paths;
  paths.reserve(fHistograms.size());
  for (const auto& h : fHistograms) {
    paths.push_back(h->GetName());
  }
  return paths;
}

void HistogramManager::PrintListOfHistograms() {

  std::cout<<"HistogramManager::PrintListOfHistograms()"<<std::endl;
  for (const auto& h : fHistograms) {
    std::cout<<h->GetName()<<std::endl;
  }
}
