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
  AnalysisManager(std::string ridffile);
  virtual ~AnalysisManager();

  bool Initialize();
  bool ProcessEvent();
  void SetDisplayOutput(DisplayOutput* output){
    fDispOutput = output;
    fDispOutput->SetAutoResetEnabled(fAutoResetEnabled);
    fDispOutput->SetAutoResetEvents(fAutoResetEvents);
  }


  Int_t  GetAnalysisBusyStatus(){return     fAnalysisBusyStatus;}
  Int_t* GetAnalysisBusyStatusPtr(){return &fAnalysisBusyStatus;}
  Int_t  GetAutoResetEnabled(){return     fAutoResetEnabled;}
  Int_t* GetAutoResetEnabledPtr(){return &fAutoResetEnabled;}
  Long64_t  GetEntries(){return     fEntries;}
  Long64_t* GetEntriesPtr(){return &fEntries;}
  Long64_t  GetAutoResetEvents(){return     fAutoResetEvents;}
  Long64_t* GetAutoResetEventsPtr(){return &fAutoResetEvents;}
  
private:
  TArtEventStore* fEventStore;

  DisplayOutput* fDispOutput;

  std::vector<BaseAnalyzer*> fAnalyzers;

  // for THttpServer
  Int_t fAnalysisBusyStatus;// 1: analysis is busy
  Int_t fAutoResetEnabled;
  Long64_t fEntries;
  Long64_t fAutoResetEvents;

  bool fIsInitialized;
};

#endif
