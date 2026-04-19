#ifndef HttpOutput_hh
#define HttpOutput_hh

#include "DisplayOutput.hh"
#include <TDatime.h>

class THttpServer;
class TNamed;

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

  THttpServer* fHttpServer;
  TNamed* fServerTimeStr;
  TDatime fDatime;
};

#endif
