#include "AnalysisManager.hh"
#include "ConfigManager.hh"
#include "HistogramManager.hh"
#include "DisplayManager.hh"
#include "TestAnalyzer.hh"
// #include "BDCAnalyzer.hh"

#include <TArtEventStore.hh>
#include <TSystem.h>
#include <TROOT.h>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


AnalysisManager::AnalysisManager(HistogramManager* histManager) 
  : fEventStore(nullptr), fHistManager(histManager)
{}

AnalysisManager::~AnalysisManager() {
  Finalize();
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
    analyzer->Init(fHistManager);
  }

  fEventStore = new TArtEventStore();
  if (!fEventStore->Open()) {
    std::cerr << "[AnalysisManager] Error: Cannot open EventStore." << std::endl;
    return false;
  }

  return true;
}

bool AnalysisManager::ProcessEvent() {
  const int anaPeriod = 500;//ms
  const int dispPeriod = 50;//ms

  // analysis
  auto startAnalysis = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - startAnalysis < std::chrono::milliseconds(anaPeriod)) {

    //if (!fEventStore->GetNextEvent()) return false;
    for (auto analyzer : fAnalyzers) {
      analyzer->Process();
    }
  }
  fDispManager->SetServerTime();

  // accept http requests
  fDispManager->SetBusy(0);
  auto startHttp = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - startHttp < std::chrono::milliseconds(dispPeriod)) {
    gSystem->ProcessEvents();
    gSystem->Sleep(1);
  }
  
  if (fHistManager->IsResetAllRequested()){
    fHistManager->ResetAll();
    fHistManager->ClearResetAllRequest();
  }
  fDispManager->SetBusy(1);// busy
  
  return true;
}

void AnalysisManager::Finalize() {
  for (auto analyzer : fAnalyzers) {
    delete analyzer;
  }
  fAnalyzers.clear();

  //if (fEventStore) delete fEventStore;// segmentation fault
  //if (fHttpServer) delete fHttpServer;// segmentation fault
  //if (fHistManager) delete fHistManager;// segmentation fault
}
