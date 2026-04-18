#include <iostream>
#include <signal.h>
#include <TApplication.h>
#include <TInterpreter.h>
#include <TSystem.h>

#include "AnalysisManager.hh"
#include "HistogramManager.hh"

bool gStopAnalysis = false;

void handle_signal(int sig) {
    std::cout << "\n[Main] Signal (" << sig << ") received. Stopping analysis..." << std::endl;
    gStopAnalysis = true;
}

int main(int argc, char** argv) {
  
  TApplication theApp("App", &argc, argv);

    signal(SIGINT, handle_signal);

    std::cout << "============================================" << std::endl;
    std::cout << "  SAMURAI Online Monitor Starting...        " << std::endl;
    std::cout << "============================================" << std::endl;

    AnalysisManager* analysisManager = new AnalysisManager();

    if (!analysisManager->Initialize()) {
        std::cerr << "[Main] Error: AnalysisManager initialization failed." << std::endl;
        return 1;
    }

    std::cout << "[Main] Start analysis loop. Press Ctrl+C to stop." << std::endl;
    
    while (!gStopAnalysis) {
        if (!analysisManager->ProcessEvent()) {
            gSystem->Sleep(100); 
        }

        gSystem->ProcessEvents();
    }

    std::cout << "[Main] Cleaning up..." << std::endl;
    analysisManager->Finalize();
    
    delete analysisManager;

    std::cout << "[Main] Finished." << std::endl;
    return 0;
}
