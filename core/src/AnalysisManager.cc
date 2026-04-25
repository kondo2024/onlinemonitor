#include "AnalysisManager.hh"
#include "ConfigManager.hh"
#include "HistogramManager.hh"
#include "DisplayOutput.hh"
#include "HttpOutput.hh"
#include "TestAnalyzer.hh"
// #include "BDCAnalyzer.hh"

#include <TArtEventStore.hh>
#include <TSystem.h>
#include <TROOT.h>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


AnalysisManager::AnalysisManager(std::string ridffile) 
  : fIsHttpMaster(false),
    fRIDFFile(ridffile), fEventStore(nullptr), fDispOutput(nullptr),
    fAnalysisBusyStatus(1), fAutoResetEnabled(1), fEntries(0),
    fAutoResetEvents(1000000), fIsInitialized(false),
    fFigAutoSave(true), fFigSaveIntervalMinutes(10)
{}

AnalysisManager::~AnalysisManager() {
  for (auto analyzer : fAnalyzers) {
    delete analyzer;
  }
  fAnalyzers.clear();

  if (fEventStore) delete fEventStore;
}

bool AnalysisManager::Initialize() {

  // definition of analyzers
  auto config = ConfigManager::GetInstance()->GetJson();

  if (config.contains("analyzers")){
    std::vector<std::string> analyzerList = config["analyzers"];
    for (const auto& name : analyzerList) {
      if (name == "Test") {
	fAnalyzers.push_back(new TestAnalyzer());
	std::cout << "[AnalysisManager] Analyzer registered: " << name << std::endl;
      }
    }
  }else{
    std::cout<<"[AnalysisManager] Error: no analyzer is defined in config.json"<<std::endl;
    return false;
  }

  for (auto analyzer : fAnalyzers) {
    analyzer->Init();
  }

  // 
  HistogramManager::GetInstance()->InitStats();
  
  // parameters
  fAutoResetEnabled = config.value("auto_reset",1);
  fAutoResetEvents = config.value("auto_reset_events",1000000);

  if (config.contains("fig")){
    if (config["fig"].contains("auto_save"))
      fFigAutoSave = config["fig"]["auto_save"];
    if (config["fig"].contains("save_interval_minutes"))
      fFigSaveIntervalMinutes = config["fig"]["save_interval_minutes"];
  }
  
  fEventStore = new TArtEventStore();
//  if (fRIDFFile=="online"){
//    fEventStore->Open(0);
//  }else if (!fEventStore->Open(fRIDFFile.c_str())) {
//    std::cerr << "[AnalysisManager] Error: Cannot open EventStore." << std::endl;
//    return false;
//  }


  fLastFigSaveTime = std::chrono::steady_clock::now(); 
  
  return true;
}

int AnalysisManager::ProcessEvent() {
  const int anaPeriod = 800;//ms, should be moved somewhere
  const int dispPeriod = 200;//ms

  fDispOutput->SetAnalysisBusyStatus(1);
  
  // analysis is done during some period
  auto startAnalysis = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - startAnalysis < std::chrono::milliseconds(anaPeriod)) {


    // temptemptemp
    //if (!fEventStore->GetNextEvent()) return 1;

    for (auto analyzer : fAnalyzers) {
      analyzer->Process();
    }
    fEntries++;
  }

  fDispOutput->SetAnalysisBusyStatus(0);
  fDispOutput->SetEntries(fEntries);
  // accept http requests
  auto startHttp = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - startHttp < std::chrono::milliseconds(dispPeriod)) {
    fDispOutput->Update();
    gSystem->ProcessEvents();
    gSystem->Sleep(1);
  }


  if (fDispOutput->IsKeyPressed()){
    int ret = fDispOutput->ExecuteKeyCommand(); 
    if (ret == -1) return ret;
  }

  
  // auto fig save
  if (fIsHttpMaster && fFigAutoSave){
    auto now = std::chrono::steady_clock::now();
    if (now - fLastFigSaveTime > std::chrono::minutes(fFigSaveIntervalMinutes)) {
      HistogramManager::GetInstance()->SaveFigures(fEntries, fAutoResetEvents);
      fLastFigSaveTime = now;
    }
  }

  // reset histograms
  if (fAutoResetEnabled && fEntries > fAutoResetEvents){
    HistogramManager::GetInstance()->RequestResetAll();
  }
  
  if (HistogramManager::GetInstance()->IsResetAllRequested()){
    HistogramManager::GetInstance()->ResetAll();
    HistogramManager::GetInstance()->ClearResetAllRequest();
    fEntries = 0;
    fDispOutput->SetEntries(fEntries);
  }
  
  return 0;
}

void AnalysisManager::SetDisplayOutput(DisplayOutput* output){
  fDispOutput = output;
  if (dynamic_cast<HttpOutput*>(fDispOutput)) fIsHttpMaster = true;
  else                                        fIsHttpMaster = false;
}
