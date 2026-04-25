#ifndef DisplayOutput_hh
#define DisplayOutput_hh

#include <string>
#include <Rtypes.h>
#include <TParameter.h>
#include <iostream>
#include <TDatime.h>

class DisplayOutput {
public:
  virtual ~DisplayOutput() {}

  virtual bool Initialize() = 0;
  virtual void Update() = 0;
  virtual void RegisterAnalysisBusyStatus() = 0;
  virtual void RegisterEntries(){
    fEntriesPrm = new TParameter<Long64_t>("Entries", fEntries);
  }
  virtual void SetAnalysisBusyStatus(Int_t i){fIsAnalysisBusyStatusPrm->SetVal(i);}
  virtual void SetEntries(Long64_t i){fEntriesPrm->SetVal(i);}

  
  virtual bool IsKeyPressed(){return false;}
  virtual int ExecuteKeyCommand(){return 0;}

  virtual Long64_t GetEntries(){return fEntriesPrm->GetVal();}
  
protected:
  DisplayOutput();
  Int_t fIsAnalysisBusyStatus;
  Long64_t fEntries;

  TParameter<Int_t>* fIsAnalysisBusyStatusPrm;
  TParameter<Long64_t>* fEntriesPrm;

  TDatime fDatime;
  
};

#endif
