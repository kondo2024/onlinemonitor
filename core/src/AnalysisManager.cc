#include "AnalysisManager.hh"
#include "HistogramManager.hh"
#include "TestAnalyzer.hh" // 具象クラスのインクルード
// #include "BDCAnalyzer.hh" // 必要に応じて追加

#include <TArtEventStore.hh>
#include <THttpServer.h>
#include <TSystem.h>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp> // JSONライブラリ (要設定)

using json = nlohmann::json;

AnalysisManager::AnalysisManager() 
    : fEventStore(nullptr), fHttpServer(nullptr), fHistoManager(nullptr) {
}

AnalysisManager::~AnalysisManager() {
    Finalize();
}

bool AnalysisManager::Initialize() {
    // 1. HistogramManagerの初期化
    fHistoManager = new HistogramManager();
    if (!fHistoManager->Init("config/histogram_range.json")) {
        std::cerr << "[AnalysisManager] Warning: Failed to load histogram_range.json. Using defaults." << std::endl;
    }

    // 2. config.json を読み込み、検出器を動的に生成
    if (!LoadConfig("config/config.json")) {
        return false;
    }

    // 3. ANAROOT EventStore の準備 (オンラインストリーム)
    fEventStore = new TArtEventStore();
    if (!fEventStore->Open()) { // デフォルトでオンライン(shm)等を開く設定
        std::cerr << "[AnalysisManager] Error: Cannot open EventStore." << std::endl;
        return false;
    }

    // 4. THttpServer の起動 (ポート 8080)
    fHttpServer = new THttpServer("http:8080");
    fHttpServer->AddLocation("monitoring/", "web/"); // UI資産の場所を指定
    fHttpServer->SetDefaultPage("monitoring/index.html");

    // 5. ブラウザから叩ける「コマンド」の登録
    // 例: http://localhost:8080/Canvases/Reset/cmd.json でリセット実行
    fHttpServer->RegisterCommand("/ResetAll", "gHistoManager->ResetAll()", "button;icons:execute");

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
        analyzer->Init(fHistoManager);
    }
    return true;
}

bool AnalysisManager::ProcessEvent() {
    if (!fEventStore->GetNextEvent()) return false;

    // 各検出器にイベントデータを渡して解析・ヒストグラム充填
    for (auto analyzer : fAnalyzers) {
        analyzer->Process(fEventStore);
    }

    return true;
}

void AnalysisManager::Finalize() {
    for (auto analyzer : fAnalyzers) {
        delete analyzer;
    }
    fAnalyzers.clear();

    if (fEventStore) delete fEventStore;
    if (fHttpServer) delete fHttpServer;
    if (fHistoManager) delete fHistoManager;
}
