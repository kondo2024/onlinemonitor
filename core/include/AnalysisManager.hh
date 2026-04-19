#ifndef AnalysisManager_hh
#define AnalysisManager_hh

#include <vector>
#include <string>
#include <TDatime.h>
#include "DisplayOutput.hh"

class TArtEventStore;
class HistogramManager;
class DisplayOutput;
class BaseAnalyzer;

class AnalysisManager {
public:
  AnalysisManager(HistogramManager* histManager);
  virtual ~AnalysisManager();

  bool Initialize();

  bool ProcessEvent();

  void Finalize();

  void SetDisplayOutput(DisplayOutput* output){
    fDispOutput = output;
    fDispOutput->RegisterAnalysisStatus(&fAnalysisBusy);
  }
  
private:
  TArtEventStore* fEventStore;

  HistogramManager* fHistManager;
  DisplayOutput* fDispOutput;

  std::vector<BaseAnalyzer*> fAnalyzers;

  Int_t fAnalysisBusy;// 1: analysis is busy
  
  bool fIsInitialized;
};

#endif
