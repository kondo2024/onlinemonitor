#include <iostream>
#include <fstream>
#include <signal.h>
#include <TApplication.h>
#include <TInterpreter.h>
#include <TSystem.h>
#include <TStyle.h>

#include "ConfigManager.hh"
#include "HistogramManager.hh"
#include "AnalysisManager.hh"

#include "HttpOutput.hh"
#include "CanvasOutput.hh"
#include <termios.h>
#include <filesystem>
#include <unistd.h>

volatile sig_atomic_t gStopAnalysis = false;

void handle_signal(int sig) {
  std::cout << "\n[OnlineMonitor] Signal (" << sig << ") received. Stopping analysis..." << std::endl;
  gStopAnalysis = 1;

}

bool endWith(const std::string& str, const std::string& suffix) {
  if (str.size() < suffix.size()) return false;
  return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int main(int argc, char** argv) {
  std::string mode = "web";// default
  const char* home = std::getenv("ONLINEMONITOR_HOME");
  if (!home){
    std::cerr<<"[OnlineMonitor] ONLINEMONITOR_HOME not set, run setup_onlinemonitor.sh"<<std::endl;
    return 1;
  }
  std::string inputRIDFFile("online");
  std::string inputconfigfile = std::string(home) + "/config/config.json";

  if (argc > 1) {
    std::string input1(argv[1]);
    if (input1 == "--help" || input1 == "-h" ){
      std::cout << "Usage: " << argv[0] << " [inputconfig.json] [run0000.ridf]" << std::endl;
      return 0;
    }

    if (endWith(input1, ".ridf") || endWith(input1, ".ridf.gz")) {
      inputRIDFFile = input1;
    }else if (endWith(input1, ".json")) {
      inputconfigfile = input1;
    }
  }

  if (argc > 2){
    std::string input2(argv[2]);
    if (endWith(input2, ".ridf") || endWith(input2, ".ridf.gz")) {
      inputRIDFFile = input2;
    }else if (endWith(input2, ".json")) {
      inputconfigfile = input2;
    }
  }
  
  std::ifstream ifs_config(inputconfigfile.c_str());
  if (!ifs_config.good()){
    std::cerr << "Error: cannot open file: " << inputconfigfile << std::endl;
    return 1;
  }

  if (inputRIDFFile != "online"){
    std::ifstream ifs_ridf(inputRIDFFile.c_str());
    if (!(inputRIDFFile=="0") && !ifs_ridf.good()){
      std::cerr << "Error: cannot open file: " << inputRIDFFile << std::endl;
      //return 1;
    }
    mode = "canvas";
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
    static_cast<CanvasOutput*>(displayOutput)->SetStopFlag((bool*)&gStopAnalysis);
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
  }
  analysisManager->SetDisplayOutput(displayOutput);
  //---------------------------------------------
  // main loop
  std::cout<<std::endl;
  std::cout << "[OnlineMonitor] Start analysis loop. Press Ctrl+C to stop." << std::endl;
  while (!gStopAnalysis) {
    gSystem->ProcessEvents();

    int ret = analysisManager->ProcessEvent();
    if      (ret == -1) break;// quit
    else if (ret != 0)  gSystem->Sleep(100); // no data, wait
  }
  gSystem->ProcessEvents();
  gSystem->ProcessEvents();
  gSystem->ProcessEvents();
  theApp.Terminate(0);
  //---------------------------------------------
  std::cout << "[OnlineMonitor] Cleaning up...  " << std::flush;
  delete analysisManager;
  delete displayOutput;
  std::cout << "Done" << std::endl;
  return 0;
}
