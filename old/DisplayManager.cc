#include "DisplayManager.hh"
#include "HistogramManager.hh"
#include "ConfigManager.hh"
#include <THttpServer.h>
#include <TNamed.h>
#include <TParameter.h>
#include <TInterpreter.h>

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <cmath>

using json = nlohmann::json;

DisplayManager* gDispManager = nullptr;

DisplayManager::DisplayManager()
  : fOutput(nullptr)
{
  gDispManager = this;
}

DisplayManager::~DisplayManager() {
  if (fOutput) delete fOutput;
}

bool DisplayManager::Initialize() {

  const auto& config = ConfigManager::GetInstance()->GetJson();
  std::string mode = "web";// default
  if (config.contains("display") && config["display"].contains("mode"))
    mode = config["display"]["mode"];

  fOutput = DisplayOutput::Create(mode);
  if (!fOutput) {
    std::cerr << "[DisplayManager] Failed to create DisplayOutput for mode:" << mode << std::endl;
    return false;
  }
  
  
//  try {
//    fHttpServer = new THttpServer("http:8080?cors");
//  } catch (...) {
//    std::cerr << "CRITICAL: Failed to allocate THttpServer" << std::endl;
//    return false;
//  }
//
//  if (!fHttpServer) return false;
//  
//  fHttpServer->AddLocation("onlinemonitor/", "web/"); //root dir of URL is web
//  fHttpServer->AddLocation("config/", "config/"); //root dir of URL is web
//  fHttpServer->SetDefaultPage("index.html");
//  fHttpServer->SetReadOnly(kFALSE);
//
//  SetupHttpCommands(fHttpServer);
//
//  fServerTimeStr = new TNamed("ServerTime","Starting ...");
//  fServerTimeStr->SetBit(kCanDelete, kFALSE);  
//  fHttpServer->Register("/Status",fServerTimeStr);
//  fIsBusy = new TParameter<Int_t>("fIsBusy",1);
//  fHttpServer->Register("/Status",fIsBusy);
//  
//  std::cout << "[DisplayManager] Initialized. Server at http://localhost:8080" << std::endl;
//  return true;

  return true;// should be checked
}

void DisplayManager::Update() {
  if (fOutput) {
    fOutput->Update();
  }
}

void DisplayManager::RegisterStatus(Int_t* busyPtr) {
  if (fOutput) {
    fOutput->RegisterStatus(busyPtr);
  }
}





//void DisplayManager::SetupHttpCommands(THttpServer* serv) {
//  if (!serv) return;
//  long long funcAddr = (long long)(void*)Internal_EventReset;
//  TString code;
//  code.Form(
//	    "extern \"C\" void GlobalReset() { "
//	    "  auto f = (void (*)()) %lld; "
//	    "  if (f) f(); "
//	    "}", funcAddr);
//    
//  gInterpreter->Declare(code.Data());
//  serv->RegisterCommand("/ResetAll", "GlobalReset()");
//}
//
//
//void DisplayManager::SetServerTime() {
//  fDatime.Set();
//  fServerTimeStr->SetTitle(fDatime.AsSQLString());
//}
//
