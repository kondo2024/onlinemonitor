#ifndef HistogramManager_hh
#define HistogramManager_hh

#include <string>
#include <map>
#include <TH1.h>
#include <TDirectory.h>

class HistogramManager {
public:
  HistogramManager();
  virtual ~HistogramManager();

  bool Init(const std::string& configPath);

  TH1* GetTH1(const std::string& name, const std::string& title, 
	      int bins, double min, double max, const std::string& folder = "Detectors");

  void ResetAll();

  void UpdateFromJSON(const std::string& configPath);

  void PrintListOfHistograms();

  
private:
  std::map<std::string, TH1*> fHistograms;
  std::map<std::string, std::pair<int, std::pair<double, double>>> fConfigMap; // name -> {bins, {min, max}}

  void LoadJSON(const std::string& path);
};

extern HistogramManager* gHistManager;

#endif
