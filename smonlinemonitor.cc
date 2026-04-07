#include <iostream>
#include <signal.h>
#include <TApplication.h>
#include <TSystem.h>

#include "AnalysisManager.hh"

// 終了フラグ（Ctrl+C 用）
bool gStopAnalysis = false;

// シグナルハンドラ: Ctrl+C が押されたときに安全にループを抜ける
void handle_signal(int sig) {
    std::cout << "\n[Main] Signal (" << sig << ") received. Stopping analysis..." << std::endl;
    gStopAnalysis = true;
}

int main(int argc, char** argv) {
    // --- 1. ROOT アプリケーションの初期化 ---
    // THttpServer や GUI 資産を適切に扱うために TApplication が必要
    TApplication theApp("App", &argc, argv);

    // シグナル（Interrupt）の登録
    signal(SIGINT, handle_signal);

    std::cout << "============================================" << std::endl;
    std::cout << "  SAMURAI Online Monitor Starting...        " << std::endl;
    std::cout << "============================================" << std::endl;

    // --- 2. 各マネージャーの初期化 ---
    // AnalysisManager が内部で HistogramManager や Detectors を生成する設計
    AnalysisManager* analysisManager = new AnalysisManager();

    if (!analysisManager->Initialize()) {
        std::cerr << "[Main] Error: AnalysisManager initialization failed." << std::endl;
        return 1;
    }

    // --- 3. メイン・解析ループ ---
    std::cout << "[Main] Start analysis loop. Press Ctrl+C to stop." << std::endl;
    
    while (!gStopAnalysis) {
        // 1イベント解析
        if (!analysisManager->ProcessEvent()) {
            // データが一時的に途切れた場合は少し待機、完全に終了ならループを抜ける
            gSystem->Sleep(100); 
            // オンライン（RIDFデータのストリーム）の場合は continue、
            // ファイル末尾なら break するなどの判定を ProcessEvent 内で行う
        }

        // --- 4. THttpServer への処理時間を割く ---
        // これを呼ばないとブラウザからのリクエストに応答できない
        gSystem->ProcessEvents();
    }

    // --- 5. 終了処理 ---
    std::cout << "[Main] Cleaning up..." << std::endl;
    analysisManager->Finalize();
    
    delete analysisManager;

    std::cout << "[Main] Finished." << std::endl;
    return 0;
}
