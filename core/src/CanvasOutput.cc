#include "CanvasOutput.hh"
#include <TSystem.h>
#include <TH1.h>
#include "ConfigManager.hh"
#include "HistogramManager.hh"
#include <TLatex.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

void SetTerminalRawMode(bool enable) {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    if (enable) {
        t.c_lflag &= ~(ICANON | ECHO);
    } else {
        t.c_lflag |= (ICANON | ECHO);
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

CanvasOutput::CanvasOutput()
  : DisplayOutput(),
    fCanvas(nullptr), fRows(3), fCols(3),
    fCurrentPage(-1), fIntervalMs(8000.0),
    fStopFlagPtr(nullptr)
{
}

CanvasOutput::~CanvasOutput() {
  fCanvas = nullptr;
}

bool CanvasOutput::Initialize() {
  const auto& config = ConfigManager::GetInstance()->GetJson();
  if (config.contains("display")){
    if (config["display"].contains("default_rows")) fRows = config["display"]["default_rows"];
    if (config["display"].contains("default_columns")) fCols = config["display"]["default_columns"];
    if (config["display"].contains("update_interval_ms")) fIntervalMs = config["display"]["update_interval_ms"];
  }	

  if (config.contains("skip_histograms")) {
    fSkipPaths = config["skip_histograms"].get<std::vector<std::string>>();
    if (fSkipPaths.size()>0){
      std::cout<<"Skip drawing histograms: "<<std::flush;
      for (const auto& s : fSkipPaths) {
	std::cout<<s<<" "<<std::flush;
      }
      std::cout<<std::endl;
    }
  }

  int width  = config["display"].value("canvas_width", 800);
  int height = config["display"].value("canvas_height", 600);
  
  fCanvas = new TCanvas("cCanvasOutput", "Online Monitor (Canvas Mode)", width, height);

  RegisterAnalysisBusyStatus();
  RegisterEntries();
  
  fTimer.Start();

  return true;
}

void CanvasOutput::RegisterAnalysisBusyStatus() {
  fIsAnalysisBusyStatusPrm = new TParameter<Int_t>("IsAnalysisBusy", fIsAnalysisBusyStatus);
}

void CanvasOutput::Update() {
  if (!fCanvas) return;

  if (fCurrentPage == -1 || fTimer.RealTime() * 1000.0 > fIntervalMs) {
    fTimer.Start();


    fDatime.Set();
    TString str = "Online Monitor (Canvas Mode) ";
    fCanvas->SetTitle(str + fDatime.AsSQLString());
    
    Draw();

    //std::cout<<"[CanvasOutput] event = "<<GetEntries()<<std::endl;
    
  } else {
    fTimer.Continue();
  }

  gSystem->ProcessEvents();
  
}

void CanvasOutput::Draw() {

  fCanvas->Clear();
  fCanvas->Divide(fCols, fRows, 0.01, 0.03);
  std::vector<std::string> allPaths = HistogramManager::GetInstance()->GetAllNames();
  std::vector<std::string> activePaths;
  for (const auto& path : allPaths) {
    bool skip = false;
    for (const auto& s : fSkipPaths) {
      if (path == s) { skip = true; break; }
    }
    if (!skip) activePaths.push_back(path);
  }

  if (activePaths.empty()) return;

    
  int histsPerPage = fRows * fCols;
  int maxPage = (activePaths.size() + histsPerPage - 1) / histsPerPage;

  fCurrentPage = (fCurrentPage + 1) % maxPage;

  int startIdx = fCurrentPage * histsPerPage;
  for (int i = 0; i < histsPerPage; ++i) {
    int pathIdx = startIdx + i;
    TVirtualPad* pad = fCanvas->cd(i + 1);
    if (!pad) continue;
    pad->SetFillStyle(0); //transparent background

    if (pathIdx < (int)activePaths.size()) {
      TH1* h = HistogramManager::GetInstance()->GetTH1(activePaths[pathIdx]);
      if (h) h->Draw("colz");
    }
  }

  fCanvas->Modified();
  fCanvas->Update();
  gSystem->ProcessEvents();
  
}

bool CanvasOutput::IsKeyPressed() {
  if (!fCanvas) return false;

  EEventType event = static_cast<EEventType>(fCanvas->GetEvent());

  if (event == kKeyPress) {
    return true;
  }

  return false;
}

int CanvasOutput::ExecuteKeyCommand() {
  std::cout << "\n[PAUSED] [s]:Resume, [q]:Quit, [n]:Next, [b]:Back, [r]:Reset > " << std::flush;

  SetTerminalRawMode(true);
  while (!(*fStopFlagPtr)) {
    gSystem->ProcessEvents(); 

    if (CheckKeyboardInput()){
      char input = getchar();

      if (input == 's') {
	std::cout << " Resume" << std::endl;
	SetTerminalRawMode(false);
	return 0; 
      } else if (input == 'q') { 
	std::cout << " Quit" << std::endl;
	SetTerminalRawMode(false);
	return -1; 
      } else if (input == 'n') {
	Draw();
	fCanvas->Modified();
	fCanvas->Update();
	gSystem->ProcessEvents(); 
      } else if (input == 'b') { 

	std::vector<std::string> allPaths = HistogramManager::GetInstance()->GetAllNames();
	std::vector<std::string> activePaths;
	for (const auto& path : allPaths) {
	  bool skip = false;
	  for (const auto& s : fSkipPaths) {
	    if (path == s) { skip = true; break; }
	  }
	  if (!skip) activePaths.push_back(path);
	}

	int histsPerPage = fRows * fCols;
	int maxPage = (activePaths.size() + histsPerPage - 1) / histsPerPage;

	fCurrentPage -= 2; 
	if (fCurrentPage < -1) fCurrentPage = maxPage-2;
	Draw();
	fCanvas->Modified();
	fCanvas->Update();
	gSystem->ProcessEvents(); 
      } else if (input == 'r') {
	HistogramManager::GetInstance()->ResetAll();
	Draw();
      }
    }
    gSystem->Sleep(10);
  }
  SetTerminalRawMode(false);
  return -1;
}

bool CanvasOutput::CheckKeyboardInput() {
    struct timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}
