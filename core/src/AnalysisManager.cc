#include "AnalysisManager.hh"
#include "ConfigManager.hh"
#include "HistogramManager.hh"
#include "DisplayOutput.hh"
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
  : fEventStore(nullptr), fAnalysisBusy(1), fEntries(0)
{}

AnalysisManager::~AnalysisManager() {
  for (auto analyzer : fAnalyzers) {
    delete analyzer;
  }
  fAnalyzers.clear();

  if (fEventStore) delete fEventStore;
}

bool AnalysisManager::Initialize() {

  auto config = ConfigManager::GetInstance()->GetJson();
  std::vector<std::string> analyzerList = config["analyzers"];
  for (const auto& name : analyzerList) {
    if (name == "Test") {
      fAnalyzers.push_back(new TestAnalyzer());
      std::cout << "[AnalysisManager] Analyzer registered: " << name << std::endl;
    }
  }

  for (auto analyzer : fAnalyzers) {
    analyzer->Init();
  }

  fEventStore = new TArtEventStore();
  if (!fEventStore->Open()) {
    std::cerr << "[AnalysisManager] Error: Cannot open EventStore." << std::endl;
    return false;
  }

  return true;
}

bool AnalysisManager::ProcessEvent() {
  const int anaPeriod = 500;//ms, should be moved somewhere
  const int dispPeriod = 50;//ms

  // analysis is done during some period
  auto startAnalysis = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - startAnalysis < std::chrono::milliseconds(anaPeriod)) {


    fDispOutput->SetAnalysisStatus(1);
    //if (!fEventStore->GetNextEvent()) return false;
    for (auto analyzer : fAnalyzers) {
      analyzer->Process();
    }
    fEntries++;
    //std::cout<<fDispOutput->GetEntries()<<std::endl;
  }
  fDispOutput->SetAnalysisStatus(0);
  fDispOutput->SetEntries(fEntries);

  // accept http requests
  auto startHttp = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - startHttp < std::chrono::milliseconds(dispPeriod)) {
    //    gSystem->ProcessEvents();// called in DisplayOutput

    fDispOutput->Update();
    gSystem->Sleep(1);
  }

  
  if (HistogramManager::GetInstance()->IsResetAllRequested()){
    HistogramManager::GetInstance()->ResetAll();
    HistogramManager::GetInstance()->ClearResetAllRequest();
    fEntries = 0;
    fDispOutput->SetEntries(fEntries);    
  }
  
  return true;
}
