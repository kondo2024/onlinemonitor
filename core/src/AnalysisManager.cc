#include "AnalysisManager.hh"
#include "HistogramManager.hh"
#include "DisplayManager.hh"
#include "TestAnalyzer.hh" // 具象クラスのインクルード
// #include "BDCAnalyzer.hh" // 必要に応じて追加

#include <TArtEventStore.hh>
#include <THttpServer.h>
#include <TSystem.h>
#include <TROOT.h>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp> // JSONライブラリ (要設定)

using json = nlohmann::json;

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
  if (!fHistManager->Init("config/histogram_range.json")) {
    std::cerr << "[AnalysisManager] Warning: Failed to load histogram_range.json. Using defaults." << std::endl;
  }

  if (!LoadConfig("config/config.json")) {
    return false;// skip
  }

  fDispManager = new DisplayManager();
  fDispManager->Init("config/config.json");

  fEventStore = new TArtEventStore();
  if (!fEventStore->Open()) { // デフォルトでオンライン(shm)等を開く設定
    std::cerr << "[AnalysisManager] Error: Cannot open EventStore." << std::endl;
    return false;
  }

  fHttpServer = new THttpServer("http:8080?cors");
  fHttpServer->AddLocation("onlinemonitor/", "web/"); //root dir of URL is web
  fHttpServer->SetDefaultPage("index.html");

  // 例: http://localhost:8080/Canvases/Reset/cmd.json でリセット実行
  fHttpServer->RegisterCommand("/Layout", "gDispManager->GetLayoutJSON()");
  fHttpServer->RegisterCommand("/ShowHistList", "gHistManager->GetListOfHistograms()");
  fHttpServer->RegisterCommand("/ResetAll", "gHistManager->ResetAll()", "button;icons:execute");
  std::cout << "[AnalysisManager] Initialized. Server at http://localhost:8080" << std::endl;
  return true;
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

  // 各検出器の初期化
  for (auto analyzer : fAnalyzers) {
    analyzer->Init(fHistManager);
  }

  //gROOT->GetList()->ls();
  return true;
}

bool AnalysisManager::ProcessEvent() {
  //if (!fEventStore->GetNextEvent()) return false;

  // 各検出器にイベントデータを渡して解析・ヒストグラム充填
  for (auto analyzer : fAnalyzers) {
    analyzer->Process();
  }

  gSystem->ProcessEvents();
  
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
