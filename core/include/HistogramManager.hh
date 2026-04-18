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

  TH1* GetTH1(const std::string& name, // input
	      const std::string& title="",// inputs if necessary
	      int bins=0, double min=0, double max=0,
	      const std::string& folder = "Detectors");
  TH1* GetTH2(const std::string& name, // input
	      const std::string& title="",// inputs if necessary
	      int xbins=0, double xmin=0, double xmax=0,
	      int ybins=0, double ymin=0, double ymax=0,
	      const std::string& folder = "Detectors");
  // wapper
  TH1* GetTH1(const char* name, const std::string& title="", 
	      int bins=0, double min=0, double max=0,
	      const std::string& folder = "Detectors"){
    std::string name_s(name);
    return GetTH1(name_s,title,bins,min,max,folder);
  }
  TH1* GetTH2(const char* name, const std::string& title="", 
	      int xbins=0, double xmin=0, double xmax=0,
	      int ybins=0, double ymin=0, double ymax=0,
	      const std::string& folder = "Detectors"){
    std::string name_s(name);
    return GetTH2(name_s,title,xbins,xmin,xmax,ybins,ymin,ymax,folder);
  }

  void ResetAll();


  void PrintListOfHistograms();

  
private:
  std::map<std::string, TH1*> fHistograms;
};

extern HistogramManager* gHistManager;

#endif
