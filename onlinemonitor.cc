#include <iostream>
#include <signal.h>
#include <TApplication.h>
#include <TInterpreter.h>
#include <TSystem.h>
#include <TThread.h>

#include "ConfigManager.hh"
#include "HistogramManager.hh"
#include "AnalysisManager.hh"

//#include "DisplayManager.hh"
#include "HttpOutput.hh"
//#include "CanvasOutput.hh"

bool gStopAnalysis = false;

void handle_signal(int sig) {
  std::cout << "\n[Main] Signal (" << sig << ") received. Stopping analysis..." << std::endl;
  gStopAnalysis = true;
}

int main(int argc, char** argv) {
  TThread::Initialize();
  
  TApplication theApp("App", &argc, argv);

  signal(SIGINT, handle_signal);

  std::cout << "============================================" << std::endl;
  std::cout << "  Online Monitor Starting...                " << std::endl;
  std::cout << "============================================" << std::endl;


  auto cm = ConfigManager::GetInstance();
  cm->LoadConfig("config/config.json");

  HistogramManager* histManager = new HistogramManager();
  

//  DisplayManager* displayManager = new DisplayManager();
//  if (!displayManager->Initialize()){
//    std::cerr << "[Main] Error: DisplayManager initialization failed." << std::endl;
//    return 1;
//  }


  const auto& config = cm->GetJson();
  std::string mode = "web";// default
  if (config.contains("display") && config["display"].contains("mode"))
    mode = config["display"]["mode"];

  DisplayOutput* displayOutput = nullptr;
  if (mode == "web" ){
    displayOutput = new HttpOutput();
  }else if  (mode == "canvas" ){
    //displayOutput = new CanvasOutput();// not yet implemented
  }else{
    std::cerr << "[Main] unknown mode:" <<mode<< std::endl;
    return false;
  }
  
  if (!displayOutput) {
    std::cerr << "[DisplayManager] Failed to create DisplayOutput for mode:" << mode << std::endl;
    return false;
  }
  
  AnalysisManager* analysisManager = new AnalysisManager(histManager);
  if (!analysisManager->Initialize()) {
    std::cerr << "[Main] Error: AnalysisManager initialization failed." << std::endl;
    return 1;
  }
  analysisManager->SetDisplayOutput(displayOutput);
  
  std::cout << "[Main] Start analysis loop. Press Ctrl+C to stop." << std::endl;

  while (!gStopAnalysis) {


    if (!analysisManager->ProcessEvent()) {
      gSystem->Sleep(100); 
    }

//
//    auto startAnalysis = std::chrono::steady_clock::now();
//    while (std::chrono::steady_clock::now() - startAnalysis < std::chrono::milliseconds(anaPeriod)) {
//    }
//
//    // accept http requests
//    displayManager->SetBusy(0);
//    auto startHttp = std::chrono::steady_clock::now();
//    while (std::chrono::steady_clock::now() - startHttp < std::chrono::milliseconds(dispPeriod)) {
//      gSystem->ProcessEvents();
//      gSystem->Sleep(1);
//    }
//    displayManager->SetBusy(1);// busy

  }

  std::cout << "[Main] Cleaning up..." << std::endl;
  analysisManager->Finalize();
    
  delete analysisManager;

  std::cout << "[Main] Finished." << std::endl;
  return 0;
}
