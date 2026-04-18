#include <iostream>
#include <signal.h>
#include <TApplication.h>
#include <TInterpreter.h>
#include <TSystem.h>
#include <TThread.h>

#include "ConfigManager.hh"
#include "HistogramManager.hh"
#include "AnalysisManager.hh"
#include "DisplayManager.hh"

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
  std::cout << "  SAMURAI Online Monitor Starting...        " << std::endl;
  std::cout << "============================================" << std::endl;


  auto cm = ConfigManager::GetInstance();
  cm->LoadConfig("config/config.json");

  HistogramManager* histManager = new HistogramManager();
  
  AnalysisManager* analysisManager = new AnalysisManager(histManager);
  if (!analysisManager->Initialize()) {
    std::cerr << "[Main] Error: AnalysisManager initialization failed." << std::endl;
    return 1;
  }

  DisplayManager* displayManager = new DisplayManager();
  if (!displayManager->Initialize()){
    std::cerr << "[Main] Error: DisplayManager initialization failed." << std::endl;
    return 1;
  }
    
  std::cout << "[Main] Start analysis loop. Press Ctrl+C to stop." << std::endl;
    
  while (!gStopAnalysis) {
    if (!analysisManager->ProcessEvent()) {
      gSystem->Sleep(100); 
    }

    displayManager->SetServerTime();

    gSystem->ProcessEvents();
  }

  std::cout << "[Main] Cleaning up..." << std::endl;
  analysisManager->Finalize();
    
  delete analysisManager;

  std::cout << "[Main] Finished." << std::endl;
  return 0;
}
