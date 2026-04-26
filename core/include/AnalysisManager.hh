#ifndef AnalysisManager_hh
#define AnalysisManager_hh

#include <vector>
#include <string>
#include <chrono>
#include <TDatime.h>
#include "DisplayOutput.hh"

class TArtEventStore;
class HistogramManager;
class DisplayOutput;
class BaseAnalyzer;

class AnalysisManager {
public:
  AnalysisManager(std::string ridffile="online");
  virtual ~AnalysisManager();

  bool Initialize();
  int ProcessEvent();
  void SetDisplayOutput(DisplayOutput* output); 

  Int_t  GetAnalysisBusyStatus(){return     fAnalysisBusyStatus;}
  Int_t* GetAnalysisBusyStatusPtr(){return &fAnalysisBusyStatus;}
  Int_t  GetAutoResetEnabled(){return     fAutoResetEnabled;}
  Int_t* GetAutoResetEnabledPtr(){return &fAutoResetEnabled;}
  Long64_t  GetEntries(){return     fEntries;}
  Long64_t* GetEntriesPtr(){return &fEntries;}
  Long64_t  GetAutoResetEvents(){return     fAutoResetEvents;}
  Long64_t* GetAutoResetEventsPtr(){return &fAutoResetEvents;}
  
private:
  bool fIsHttpMaster = false;

  std::string fRIDFFile;
  TArtEventStore* fEventStore = nullptr;

  DisplayOutput* fDispOutput = nullptr;

  std::vector<BaseAnalyzer*> fAnalyzers;

  // for THttpServer
  Int_t fAnalysisBusyStatus = 1;// 1: analysis is busy
  Int_t fAutoResetEnabled = 1;
  Long64_t fEntries = 0;
  Long64_t fAutoResetEvents = 1000000;

  std::chrono::steady_clock::time_point fLastFigSaveTime;
  bool fFigAutoSave = true;
  int fFigSaveIntervalMinutes = 10;
  
  bool fIsInitialized = false;
};

#endif
