#ifndef HistogramManager_hh
#define HistogramManager_hh

#include <string>
#include <map>
#include <TH1.h>
#include <TDirectory.h>

class HistogramManager {
public:
  HistogramManager();
  virtual ~HistogramManager();

  // 初期化：JSON読み込みとディレクトリ作成
  bool Init(const std::string& configPath);

  // ヒストグラムの取得または生成
  // デフォルト値(bins, min, max)を指定し、JSONがあれば上書きする
  TH1* GetTH1(const std::string& name, const std::string& title, 
	      int bins, double min, double max, const std::string& folder = "Detectors");

  // 全ヒストグラムのリセット（THttpServerから呼ばれる用）
  void ResetAll();

  // 特定のヒストグラムの再設定（JSON更新時など）
  void UpdateFromJSON(const std::string& configPath);

  void PrintListOfHistograms();

  
private:
  std::map<std::string, TH1*> fHistograms;
  std::map<std::string, std::pair<int, std::pair<double, double>>> fConfigMap; // name -> {bins, {min, max}}

  void LoadJSON(const std::string& path);
};

// THttpServerのコマンドからアクセスしやすくするため、グローバルポインタを用意
extern HistogramManager* gHistoManager;

#endif
