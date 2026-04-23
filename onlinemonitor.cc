#include <iostream>
#include <fstream>
#include <signal.h>
#include <TApplication.h>
#include <TInterpreter.h>
#include <TSystem.h>

#include "ConfigManager.hh"
#include "HistogramManager.hh"
#include "AnalysisManager.hh"

#include "HttpOutput.hh"
#include "CanvasOutput.hh"
#include <termios.h>
#include <unistd.h>

bool gStopAnalysis = false;

void handle_signal(int sig) {
  std::cout << "\n[OnlineMonitor] Signal (" << sig << ") received. Stopping analysis..." << std::endl;
  gStopAnalysis = true;

}

int main(int argc, char** argv) {
  std::string mode = "web";// default
  const char* home = std::getenv("ONLINEMONITOR_HOME");
  if (!home){
    std::cerr<<"[OnlineMonitor] ONLINEMONITOR_HOME not set, run setup_onlinemonitor.sh"<<std::endl;
  }
  std::string inputRIDFFile("");
  std::string inputconfigfile = std::string(home) + "/web/config/config.json";

  if (argc > 1) {
    std::string input1(argv[1]);
    if (input1 == "--help" || input1 == "-h" ){
      std::cerr << "Usage: " << argv[0] << " [run0000.ridf] [inputconfig.json]" << std::endl;
      return 1;
    }

    inputRIDFFile = argv[1];
    std::ifstream ifs(inputRIDFFile.c_str());
    if (!(inputRIDFFile=="0") && !ifs.good()){
      std::cerr << "Error: cannot open file: " << inputRIDFFile << std::endl;
      //return 1;
    }
    mode = "canvas";
  }

  if (argc > 2){
    std::string input2(argv[2]);
    inputconfigfile = argv[2];
    std::ifstream ifs(inputconfigfile.c_str());
    if (!ifs.good()){
      std::cerr << "Error: cannot open file: " << inputconfigfile << std::endl;
      return 1;
    }
  }

  TApplication theApp("App", &argc, argv);

  signal(SIGINT, handle_signal);

  std::cout << "============================================" << std::endl;
  std::cout << "  Online Monitor Starting...                " << std::endl;
  std::cout << "============================================" << std::endl;


  //---------------------------------------------
  auto cm = ConfigManager::GetInstance();
  cm->LoadConfig(inputconfigfile);

  //---------------------------------------------
  // display
  const auto& config = cm->GetJson();
  if (config.contains("display") && config["display"].contains("mode"))
    mode = config["display"]["mode"];
  std::cout<<"[OnlineMonitor] mode = "<<mode<<std::endl;

  DisplayOutput* displayOutput = nullptr;
  if (mode == "web" ){
    displayOutput = new HttpOutput();
  }else if  (mode == "canvas" ){
    displayOutput = new CanvasOutput();// not yet implemented
    ((CanvasOutput*)displayOutput)->SetStopFlag(&gStopAnalysis);
  }else{
    std::cerr << "[OnlineMonitor] unknown mode:" <<mode<< std::endl;
    return false;
  }
  
  if (!displayOutput) {
    std::cerr << "[DisplayManager] Failed to create DisplayOutput for mode:" << mode << std::endl;
    return false;
  }
  displayOutput->Initialize();
  
  //---------------------------------------------
  // analysis
  AnalysisManager* analysisManager = new AnalysisManager(inputRIDFFile);
  if (!analysisManager->Initialize()) {
    std::cerr << "[OnlineMonitor] Error: AnalysisManager initialization failed." << std::endl;
    return 1;
  }
  analysisManager->SetDisplayOutput(displayOutput);
  
  //---------------------------------------------
  std::cout<<std::endl;
  std::cout << "[OnlineMonitor] Start analysis loop. Press Ctrl+C to stop." << std::endl;
  while (!gStopAnalysis) {

    gSystem->ProcessEvents();
    if (gStopAnalysis) break;

    

    if (displayOutput->IsKeyPressed()) {
      int ret = displayOutput->ExecuteKeyCommand();
      if (ret == -1) break;
    }
    

    if (!analysisManager->ProcessEvent()) {
      gSystem->Sleep(100); 
    }

  }

  //---------------------------------------------
  std::cout << "[OnlineMonitor] Cleaning up...  " << std::flush;
  delete analysisManager;
  delete displayOutput;
  std::cout << "Done" << std::endl;
  return 0;
}
