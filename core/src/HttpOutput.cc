#include "HttpOutput.hh"
#include "HistogramManager.hh"
#include "ConfigManager.hh"
#include <THttpServer.h>
#include <TNamed.h>
#include <TInterpreter.h>
#include <TSystem.h>
#include <iostream>

extern "C" void Internal_GlobalReset() {
//  if (gHistManager) {
//    gHistManager->RequestResetAll();
//  } else {
//    std::cerr << "Error: gHistManager is null!" << std::endl;
//  }
  HistogramManager::GetInstance()->RequestResetAll();
}

HttpOutput::HttpOutput()
  : fHttpServer(nullptr), fServerTimeStr(nullptr) {
}

HttpOutput::~HttpOutput() {
  if (fHttpServer) delete fHttpServer;
}

bool HttpOutput::Initialize() {


  const auto& config = ConfigManager::GetInstance()->GetJson();
  int port = 8080;
  if (config.contains("http_port"))
    port = config["http_port"];
  std::cout<<"[HttpOutput] port:"<<port<<std::endl;

  try {
    //fHttpServer = new THttpServer("http:8080?cors");
    TString str = Form("http:%d",port);
    fHttpServer = new THttpServer(str.Data());
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

  std::cout << "[HttpOutput] Web Server initialized at http://localhost:"<<port << std::endl;
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
  gSystem->ProcessEvents();// accept http requests
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
