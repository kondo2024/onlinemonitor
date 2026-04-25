#ifndef HistogramManager_hh
#define HistogramManager_hh

#include <string>
#include <map>
#include <TH1.h>
#include <TH2.h>
#include <TDirectory.h>
#include <TCanvas.h>

class HistogramManager {
public:

  // singleton
  static HistogramManager* GetInstance() {
    static HistogramManager instance;
    return &instance;
  }
  HistogramManager(const HistogramManager&) = delete;
  HistogramManager& operator=(const HistogramManager&) = delete;

  TH1* BookTH1(const std::string& name, const std::string& title,
	       int bins, double min, double max,
	       const std::string& folder = "Detectors");
  TH1* BookTH2(const std::string& name, const std::string& title,
	       int xbins, double xmin, double xmax,
	       int ybins, double ymin, double ymax,
	       const std::string& folder = "Detectors");

  void ChangeRangeTH1(TH1* h);
  void ChangeRangeTH2(TH1* h);
  
  TH1* GetTH1(const std::string& name);
  TH2* GetTH2(const std::string& name);

  void InitStats();
  void SaveFigures(Long64_t currentEvents, Long64_t resetThreshold);
  
  void RequestResetAll();
  bool IsResetAllRequested() const { return fResetAllRequested; }
  void ClearResetAllRequest() { fResetAllRequested = false; }
  void ResetAll();


  std::vector<std::string> GetAllNames();
  
  void PrintListOfHistograms();

  
private:
  HistogramManager();
  virtual ~HistogramManager();

  bool fResetAllRequested;
  std::map<std::string, TH1*> fHistogramsMap;
  std::vector<TH1*> fHistograms;
  TCanvas* fFigSaveCanvas;
  
  void SetDirectory(TH1* h, const std::string& folder);
};

#endif
