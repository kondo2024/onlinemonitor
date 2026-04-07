#ifndef AnalysisManager_hh
#define AnalysisManager_hh

#include <vector>
#include <string>

// 前方宣言（コンパイル時間の短縮と循環参照の防止）
class TArtEventStore;
class THttpServer;
class HistogramManager;
class BaseAnalyzer;

class AnalysisManager {
public:
    AnalysisManager();
    virtual ~AnalysisManager();

    // 初期化：EventStoreのオープン、Serverの起動、Detectorsの生成
    bool Initialize();

    // 1イベントごとの処理：EventStoreからデータを読み、各Analyzerに渡す
    bool ProcessEvent();

    // 終了処理：メモリ解放、Serverの停止
    void Finalize();

private:
    // config.json を読み込み、必要な検出器クラスをインスタンス化する
    bool LoadConfig(const std::string& configPath);

    // ANAROOT 関連
    TArtEventStore* fEventStore;

    // ROOT HTTP Server 関連
    THttpServer* fHttpServer;

    // ヒストグラム管理
    HistogramManager* fHistoManager;

    // 検出器アナライザーのリスト（多態性による一括管理）
    std::vector<BaseAnalyzer*> fAnalyzers;

    // 内部フラグ
    bool fIsInitialized;
};

#endif
