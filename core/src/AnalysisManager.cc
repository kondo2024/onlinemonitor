#include "AnalysisManager.hh"
#include "HistogramManager.hh"
#include "DisplayManager.hh"
#include "TestAnalyzer.hh"
// #include "BDCAnalyzer.hh"

#include <TArtEventStore.hh>
#include <THttpServer.h>
#include <TNamed.h>
#include <TDatime.h>
#include <TSystem.h>
#include <TROOT.h>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <TInterpreter.h>

using json = nlohmann::json;


extern "C" void Internal_EventReset() {
    if (gHistManager) {
        gHistManager->ResetAll();
    } else {
        std::cerr << "Error: gHistManager is null!" << std::endl;
    }
}

AnalysisManager::AnalysisManager() 
  : fEventStore(nullptr), fHttpServer(nullptr), fHistManager(nullptr),
    fDispManager(nullptr)
{
}

AnalysisManager::~AnalysisManager() {
  Finalize();
}

bool AnalysisManager::Initialize() {

  fHistManager = new HistogramManager();
  gHistManager = fHistManager;
  if (!fHistManager->Init("config/histogram_range.json")) {
    std::cerr << "[AnalysisManager] Warning: Failed to load histogram_range.json. Using defaults." << std::endl;
  }

  if (!LoadConfig("config/config.json")) {
    return false;// skip
  }

  fDispManager = new DisplayManager();
  fDispManager->Init("config/config.json");

  fEventStore = new TArtEventStore();
  if (!fEventStore->Open()) {
    std::cerr << "[AnalysisManager] Error: Cannot open EventStore." << std::endl;
    return false;
  }

  fHttpServer = new THttpServer("http:8080?cors");
  fHttpServer->AddLocation("onlinemonitor/", "web/"); //root dir of URL is web
  fHttpServer->SetDefaultPage("index.html");
  fHttpServer->SetReadOnly(kFALSE);

  fHttpServer->RegisterCommand("/Layout", "gDispManager->GetLayoutJSON()");
  fHttpServer->RegisterCommand("/ShowHistList", "gHistManager->GetListOfHistograms()");
  SetupHttpCommands(fHttpServer);

  fServerTime = new TNamed("ServerTime","Starting ...");
  fHttpServer->Register("/Status",fServerTime);
  
  std::cout << "[AnalysisManager] Initialized. Server at http://localhost:8080" << std::endl;
  return true;
}

void AnalysisManager::SetupHttpCommands(THttpServer* serv) {
    if (!serv) return;
    long long funcAddr = (long long)(void*)Internal_EventReset;
    TString code;
    code.Form(
        "extern \"C\" void GlobalReset() { "
        "  auto f = (void (*)()) %lld; "
        "  if (f) f(); "
        "}", funcAddr);
    
    gInterpreter->Declare(code.Data());
    serv->RegisterCommand("/ResetAll", "GlobalReset()");
}


bool AnalysisManager::LoadConfig(const std::string& configPath) {
  std::ifstream ifs(configPath);
  if (!ifs.is_open()) return false;

  json conf;
  ifs >> conf;

  // config.json の "detectors": ["BDC", "HOD"] のような記述をパース
  for (auto& detName : conf["detectors"]) {
    if (detName == "Test") {
      fAnalyzers.push_back(new TestAnalyzer());
    }
    // else if (detName == "BDC") fAnalyzers.push_back(new BDCAnalyzer());
  }

  for (auto analyzer : fAnalyzers) {
    analyzer->Init(fHistManager);
  }

  return true;
}

bool AnalysisManager::ProcessEvent() {
  //if (!fEventStore->GetNextEvent()) return false;

  for (auto analyzer : fAnalyzers) {
    analyzer->Process();
  }

  fDatime.Set();
  fServerTime->SetTitle(fDatime.AsSQLString());
  
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
