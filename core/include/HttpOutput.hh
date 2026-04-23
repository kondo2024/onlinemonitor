#ifndef HttpOutput_hh
#define HttpOutput_hh

#include "DisplayOutput.hh"

class THttpServer;
class TNamed;

class HttpOutput : public DisplayOutput {
public:
  HttpOutput();
  virtual ~HttpOutput();

  bool Initialize() override;
  void Update() override;
  void RegisterAnalysisBusyStatus() override;
  void RegisterEntries() override;

  void RegisterParameters();
  
private:
  void SetupHttpCommands();
  void RefreshServerTime();

  THttpServer* fHttpServer;
  TNamed* fServerTimeStr;
  TDatime fServerStartTime;
};

#endif
