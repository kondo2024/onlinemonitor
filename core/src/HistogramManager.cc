#include "HistogramManager.hh"
#include "ConfigManager.hh"
#include <TROOT.h>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <TH1.h>
#include <TH2.h>
#include <TDatime.h>
#include <TLatex.h>
#include <TSystem.h>
#include <TStyle.h>

using json = nlohmann::json;

HistogramManager::HistogramManager()
  : fResetAllRequested(false), fFigSaveCanvas(nullptr)
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

  //SetDirectory(h, folder);
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

  //SetDirectory(h, folder);
  ChangeRangeTH2(h);
  return h;
}

void HistogramManager::ChangeRangeTH1(TH1* h) {
  
  const std::string name = h->GetName();
  const auto& config = ConfigManager::GetInstance()->GetJson();
  if (config.contains("ranges") && config["ranges"].contains(name)) {
    auto& r = config["ranges"][name];

    int bins = r.value("xbins", h->GetNbinsX());
    double min = r.value("xmin", h->GetXaxis()->GetXmin());
    double max = r.value("xmax", h->GetXaxis()->GetXmax());
    h->SetBins(bins, min, max);
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
      int   xbins = r.value("xbins", h2->GetNbinsX());
      double xmin = r.value("xmin",  h2->GetXaxis()->GetXmin());
      double xmax = r.value("xmax",  h2->GetXaxis()->GetXmax());
      int   ybins = r.value("ybins", h2->GetNbinsY());
      double ymin = r.value("ymin",  h2->GetYaxis()->GetXmin());
      double ymax = r.value("ymax",  h2->GetYaxis()->GetXmax());
      //h->SetBins(bins, min, max);
      
      h2->SetBins(xbins, xmin, xmax, ybins, ymin, ymax);
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


void HistogramManager::SetDirectory(TH1* h, const std::string& folder) {
  if (!h) return;
  TDirectory* dir = gROOT->GetDirectory(folder.c_str());
  if (!dir) dir = gROOT->mkdir(folder.c_str());
  h->SetDirectory(dir);
}

void HistogramManager::InitStats() {// dummy draw for reflecting gStyle settings

  SetStyle();
  TVirtualPad *savePad = gPad;
  gROOT->SetBatch(kTRUE);
  if (!fFigSaveCanvas)
    fFigSaveCanvas = new TCanvas("cFigSave", "Batch Save", 1200, 900);

  float rm = fFigSaveCanvas->GetRightMargin();
  float tm = fFigSaveCanvas->GetTopMargin();
//  gStyle->SetStatX(1.0-rm-0.03);
//  gStyle->SetStatY(1.0-tm);
//  gStyle->SetStatW(0.25);
//  gStyle->SetStatH(0.2);

  for (const auto& h : fHistograms) h->Paint();
  fFigSaveCanvas->Update();
  gROOT->SetBatch(kFALSE);
  if (savePad) savePad->cd();
}
void HistogramManager::SetStyle(){
  gStyle->SetHistFillColor(7);
  gStyle->SetHistFillStyle(3002);
  gStyle->SetHistLineColor(kBlue);
  gStyle->SetFuncColor(kRed);
  gStyle->SetFrameLineWidth(2);
  gStyle->SetPadGridX(1);
  gStyle->SetPadGridY(1);
  gStyle->SetCanvasColor(0);
  gStyle->SetTitleFillColor(0);
  gStyle->SetTitleStyle(0);
  gStyle->SetStatColor(0);
  gStyle->SetStatStyle(0);
  gStyle->SetStatX(0.9);
  gStyle->SetStatY(0.9);
  gStyle->SetPalette(1,0);
  gStyle->SetOptLogz(1);
  gStyle->SetOptFit(1);
  gStyle->SetOptStat(1111111);
  gStyle->SetPadBorderMode(1);
  gStyle->SetOptDate(1);

  gStyle->SetLabelFont(132,"XYZ");
  gStyle->SetTitleFont(132,"XYZ");
  gStyle->SetTitleFont(132,"");
  gStyle->SetTextFont(132);
  gStyle->SetStatFont(132);
  gStyle->SetCanvasDefW(800);
  gStyle->SetCanvasDefH(800);
  gStyle->SetPaperSize(20,20);
}

void HistogramManager::SaveFigures(Long64_t currentEvents, Long64_t resetThreshold) {
  TVirtualPad *savePad = gPad;

  if (currentEvents < (resetThreshold * 0.1)) {
    std::cout << "AutoSave skipped: Insufficient statistics." << std::endl;
    return;
  }

  const char* home = std::getenv("ONLINEMONITOR_HOME");
  TDatime dt;
  char ts[20];
  snprintf(ts, sizeof(ts), "%08d_%06d", dt.GetDate(), dt.GetTime());
  std::string timestamp(ts);

  const int rows = 3;
  const int cols = 3;
  const int padsPerPage = rows * cols;

  gROOT->SetBatch(kTRUE);
  if (!fFigSaveCanvas)
    fFigSaveCanvas = new TCanvas("cFigSave", "Batch Save", 1200, 900);
  
  fFigSaveCanvas->Divide(cols, rows);
  int totalHists = fHistograms.size();
  int pageNum = 1;

  for (int i = 0; i < totalHists; ++i) {
    int padIdx = (i % padsPerPage) + 1;
    fFigSaveCanvas->cd(padIdx);

    fHistograms[i]->Draw();

    if (padIdx == padsPerPage || i == totalHists - 1) {
      std::string filename = (std::string)home + "/figs/onlinemonitor_" + timestamp + "_" + std::to_string(pageNum) + ".png";
      fFigSaveCanvas->SaveAs(filename.c_str());
          
      fFigSaveCanvas->Clear();
      fFigSaveCanvas->Divide(cols, rows);
      pageNum++;
    }
  }

  gROOT->SetBatch(kFALSE);
  if (savePad) savePad->cd();
  //std::cout << "[AutoSave] Completed: " << (pageNum - 1) << " pages saved." << std::endl;

  gSystem->ProcessEvents();

}    


void HistogramManager::RequestResetAll() {
  fResetAllRequested = true;
}

void HistogramManager::ResetAll() {
  //std::cout << "[HistogramManager] Resetting all histograms..." << std::endl;
  ConfigManager::GetInstance()->ReloadConfig();

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
