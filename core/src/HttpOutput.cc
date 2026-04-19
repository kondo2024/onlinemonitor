#include "HttpOutput.hh"
#include "HistogramManager.hh"
#include <THttpServer.h>
#include <TNamed.h>
#include <TParameter.h>
#include <TInterpreter.h>
#include <TSystem.h>
#include <iostream>

extern "C" void Internal_GlobalReset() {
  if (gHistManager) {
    gHistManager->RequestResetAll();
  } else {
    std::cerr << "Error: gHistManager is null!" << std::endl;
  }
}

HttpOutput::HttpOutput()
  : fHttpServer(nullptr), fServerTimeStr(nullptr), fIsAnalysisBusy(nullptr) {
}

HttpOutput::~HttpOutput() {
  if (fHttpServer) delete fHttpServer;
}

bool HttpOutput::Initialize() {
  try {
    fHttpServer = new THttpServer("http:8080?cors");
  } catch (...) {
    std::cerr << "CRITICAL: Failed to allocate THttpServer" << std::endl;
    return false;
  }

  if (!fHttpServer) return false;  

  fHttpServer->AddLocation("onlinemonitor/", "web/"); 
  fHttpServer->AddLocation("config/", "config/");
  fHttpServer->SetDefaultPage("index.html");
  fHttpServer->SetReadOnly(kFALSE);

  SetupHttpCommands();

  fServerTimeStr = new TNamed("ServerTime", "Starting ...");
  fHttpServer->Register("/Status", fServerTimeStr);

  std::cout << "[HttpOutput] Web Server initialized at http://localhost:8080" << std::endl;
  return true;
}

void HttpOutput::RegisterAnalysisStatus(Int_t* busyPtr) {
  if (!fHttpServer || !busyPtr) return;

  fIsAnalysisBusy = new TParameter<Int_t>("fIsBusy", *busyPtr);
  fHttpServer->Register("/Status", fIsAnalysisBusy);
}

void HttpOutput::Update() {
  if (!fHttpServer) return;

  RefreshServerTime();

  // accept http requests
  gSystem->ProcessEvents();
}

void HttpOutput::RefreshServerTime() {
  if (!fServerTimeStr) return;
  fDatime.Set();
  fServerTimeStr->SetTitle(fDatime.AsSQLString());
}

void HttpOutput::SetupHttpCommands() {
  if (!fHttpServer) return;

  long long funcAddr = (long long)(void*)Internal_GlobalReset;
  TString code;
  code.Form(
        "extern \"C\" void GlobalReset() { "
        "  auto f = (void (*)()) %lld; "
        "  if (f) f(); "
        "}", funcAddr);

  gInterpreter->Declare(code.Data());
  fHttpServer->RegisterCommand("/ResetAll", "GlobalReset()");
}
