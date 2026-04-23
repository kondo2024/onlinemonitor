#ifndef DisplayOutput_hh
#define DisplayOutput_hh

#include <string>
#include <Rtypes.h>
#include <TParameter.h>
#include <iostream>

class DisplayOutput {
public:
  virtual ~DisplayOutput() {}

  virtual bool Initialize() = 0;
  virtual void Update() = 0;
  virtual void RegisterAnalysisBusyStatus() = 0;
  virtual void RegisterEntries(){
    fEntriesPrm = new TParameter<Long64_t>("Entries", fEntries);
  }
  virtual void RegisterAutoResetEnabled(){
    fAutoResetEnabledPrm = new TParameter<Int_t>("AutoResetEnabled", fAutoResetEnabled);
  }
  virtual void RegisterAutoResetEvents(){
    fAutoResetEventsPrm = new TParameter<Long64_t>("AutoResetEvents", fAutoResetEvents);
  }
  virtual void SetAnalysisBusyStatus(Int_t i){fIsAnalysisBusyStatusPrm->SetVal(i);}
  virtual void SetEntries(Long64_t i){fEntriesPrm->SetVal(i);}
  virtual void SetAutoResetEnabled(Int_t i){
    fAutoResetEnabledPrm->SetVal(i);
  }
  virtual void SetAutoResetEvents(Long64_t i){
    fAutoResetEventsPrm->SetVal(i);
  }

  
  virtual bool IsKeyPressed(){return false;}
  virtual int ExecuteKeyCommand(){return 0;}

  virtual Long64_t GetEntries(){return fEntriesPrm->GetVal();}
  virtual bool IsAutoResetEnabled(){return fAutoResetEnabledPrm->GetVal()==1 ? true : false;}
  virtual Long64_t GetAutoResetEvents(){return fAutoResetEventsPrm->GetVal();}
  
protected:
  DisplayOutput()
    : fIsAnalysisBusyStatusPrm(nullptr), fEntriesPrm(nullptr), fAutoResetEventsPrm(nullptr)
  {}
  Int_t fIsAnalysisBusyStatus;
  Int_t fAutoResetEnabled;
  Long64_t fEntries;
  Long64_t fAutoResetEvents;

  TParameter<Int_t>* fIsAnalysisBusyStatusPrm;
  TParameter<Int_t>* fAutoResetEnabledPrm;
  TParameter<Long64_t>* fEntriesPrm;
  TParameter<Long64_t>* fAutoResetEventsPrm;

};

#endif
