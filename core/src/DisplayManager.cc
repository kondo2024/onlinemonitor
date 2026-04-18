#include "DisplayManager.hh"
#include "HistogramManager.hh"
#include "ConfigManager.hh"
#include <THttpServer.h>
#include <TNamed.h>
#include <TInterpreter.h>

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <cmath>

using json = nlohmann::json;

extern "C" void Internal_EventReset() {
  if (gHistManager) {
    gHistManager->ResetAll();
  } else {
    std::cerr << "Error: gHistManager is null!" << std::endl;
  }
}

DisplayManager* gDispManager = nullptr;

DisplayManager::DisplayManager() : fHttpServer(nullptr) {
}

DisplayManager::~DisplayManager() {
}

bool DisplayManager::Initialize() {

  try {
    fHttpServer = new THttpServer("http:8080?cors");
  } catch (...) {
    std::cerr << "CRITICAL: Failed to allocate THttpServer" << std::endl;
    return false;
  }

  if (!fHttpServer) return false;
  
  fHttpServer->AddLocation("onlinemonitor/", "web/"); //root dir of URL is web
  fHttpServer->SetDefaultPage("index.html");
  fHttpServer->SetReadOnly(kFALSE);

  SetupHttpCommands(fHttpServer);

  fServerTimeStr = new TNamed("ServerTime","Starting ...");
  fServerTimeStr->SetBit(kCanDelete, kFALSE);  
  fHttpServer->Register("/Status",fServerTimeStr);
  
  std::cout << "[DisplayManager] Initialized. Server at http://localhost:8080" << std::endl;
  return true;
}

void DisplayManager::SetupHttpCommands(THttpServer* serv) {
  if (!serv) return;
  long long funcAddr = (long long)(void*)Internal_EventReset;
  TString code;
  code.Form(
	    "extern \"C\" void GlobalReset() { "
	    "  auto f = (void (*)()) %lld; "
	    "  if (f) f(); "
	    "}", funcAddr);
    
  gInterpreter->Declare(code.Data());
  serv->RegisterCommand("/ResetAll", "GlobalReset()");
}


void DisplayManager::SetServerTime() {
  fDatime.Set();
  fServerTimeStr->SetTitle(fDatime.AsSQLString());
}
