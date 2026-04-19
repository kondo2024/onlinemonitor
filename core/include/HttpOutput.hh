#ifndef HttpOutput_hh
#define HttpOutput_hh

#include "DisplayOutput.hh"
#include <TDatime.h>

class THttpServer;
class TNamed;
template <typename T> class TParameter;

class HttpOutput : public DisplayOutput {
public:
  HttpOutput();
  virtual ~HttpOutput();

  bool Initialize() override;
  void Update() override;
  void RegisterAnalysisStatus(Int_t* busyPtr) override;

private:
  void SetupHttpCommands();
  void RefreshServerTime();

  THttpServer* fHttpServer;    ///< ROOTのHTTPサーバー本体
  TNamed* fServerTimeStr; ///< 稼働時間を表示するための文字列オブジェクト
  TDatime              fDatime;        ///< 時刻取得用
  TParameter<Int_t>* fIsAnalysisBusy; ///< Int_t* をROOTオブジェクトとして扱うためのラッパー
};

#endif
