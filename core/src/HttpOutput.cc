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
  : fHttpServer(nullptr), fServerTimeStr(nullptr) {
}

HttpOutput::~HttpOutput() {
  if (fHttpServer) delete fHttpServer;
}

bool HttpOutput::Initialize() {


  ConfigManager* cm = ConfigManager::GetInstance();
  const auto& config = cm->GetJson();
  int port = 8080;
  if (config.contains("http_port"))
    port = config["http_port"];
  std::cout<<"[HttpOutput] port = "<<port<<std::endl;

  const char* home = std::getenv("ONLINEMONITOR_HOME");
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
  fHttpServer->SetDefaultPage("index.html");
  fHttpServer->SetReadOnly(kFALSE);

  SetupHttpCommands();

  fServerTimeStr = new TNamed("ServerTime", "Starting ...");
  fHttpServer->Register("/Status", fServerTimeStr);


  RegisterAnalysisBusyStatus();
  RegisterEntries();
  RegisterAutoResetEnabled();    
  RegisterAutoResetEvents();    

  std::string json_str = cm->GetJson().dump();
  //fHttpServer->Register("/Config",json_str.c_str());
  
  fHttpServer->Register("/Config", new TNamed("ConfigPath", cm->GetConfigPath().c_str()));

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

  std::cout<<"[HttpOutput] RegisterEntries"<<std::endl;  
}

void HttpOutput::RegisterAutoResetEnabled() {
  if (!fHttpServer) return;
  fAutoResetEnabledPrm = new TParameter<Int_t>("AutoResetEnabled", fAutoResetEnabled);
  fHttpServer->Register("/Config", fAutoResetEnabledPrm);

  std::cout<<"[HttpOutput] RegisterAutoResetEnabled"<<std::endl;  
}

void HttpOutput::RegisterAutoResetEvents() {
  if (!fHttpServer) return;
  fAutoResetEventsPrm = new TParameter<Long64_t>("AutoResetEvents", fAutoResetEvents);
  fHttpServer->Register("/Config", fAutoResetEventsPrm);

  std::cout<<"[HttpOutput] RegisterAutoResetEvents"<<std::endl;  
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
