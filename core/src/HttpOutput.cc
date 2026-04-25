#include "HttpOutput.hh"
#include "HistogramManager.hh"
#include "ConfigManager.hh"
#include <THttpServer.h>
#include <TNamed.h>
#include <TInterpreter.h>
#include <TSystem.h>
#include <iostream>

extern "C" void Internal_GlobalReset() {
  HistogramManager::GetInstance()->RequestResetAll();
}

HttpOutput::HttpOutput()
  : DisplayOutput(),
    fHttpServer(nullptr), fServerTimeStr(nullptr) {
}

HttpOutput::~HttpOutput() {
  if (fHttpServer) delete fHttpServer;
}

bool HttpOutput::Initialize() {
  ConfigManager* cm = ConfigManager::GetInstance();
  const auto& config = cm->GetJson();

  // port
  int port = config.value("http_port", 8080);

  // http root
  const char* home = std::getenv("ONLINEMONITOR_HOME");
  if (!home) {
    std::cerr << "[HttpOutput] CRITICAL: ONLINEMONITOR_HOME not set." << std::endl;
    return false;
  }
  
  std::string http_root = std::string(home) + "/web/";
  std::cout<<"[HttpOutput] http root: "<<http_root<<std::endl;
  try {
    TString str = Form("http:%d",port);
    fHttpServer = new THttpServer(str.Data());
  } catch (...) {
    std::cerr << "CRITICAL: Failed to allocate THttpServer" << std::endl;
    return false;
  }

  if (!fHttpServer) return false;  

  fHttpServer->AddLocation("web/", http_root.c_str()); 
  fHttpServer->SetReadOnly(kFALSE);

  SetupHttpCommands();

  fServerTimeStr = new TNamed("ServerTime", "Starting ...");
  fHttpServer->Register("/Status", fServerTimeStr);

  RegisterAnalysisBusyStatus();
  RegisterEntries();

  fHttpServer->Register("/Config", cm->GetConfigContentsPtr());

  fServerStartTime.Set();
  fHttpServer->Register("/Status", new TNamed("ServerStartTime",fServerStartTime.AsSQLString()));
  
  std::cout << "[HttpOutput] Web Server initialized, visit http://localhost:"<<port<<"/web/index.html" << std::endl;

  return true;
}

void HttpOutput::RegisterAnalysisBusyStatus() {
  if (!fHttpServer) return;

  fIsAnalysisBusyStatusPrm = new TParameter<Int_t>("IsAnalysisBusy", fIsAnalysisBusyStatus);
  fHttpServer->Register("/Status", fIsAnalysisBusyStatusPrm);
}

void HttpOutput::RegisterEntries() {
  if (!fHttpServer) return;

  fEntriesPrm = new TParameter<Long64_t>("Entries", fEntries);
  fHttpServer->Register("/Status", fEntriesPrm);
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
