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
  virtual void RegisterAnalysisStatus() = 0;
  virtual void RegisterEntries(){
    fEntriesPrm = new TParameter<Long64_t>("Entries", fEntries);
    std::cout<<"DisplayOutput: RegisterEntries"<<std::endl;
  }
  virtual void SetAnalysisStatus(Int_t i){fIsAnalysisBusyPrm->SetVal(i);}
  virtual void SetEntries(Long64_t i){fEntriesPrm->SetVal(i);}

  
  virtual bool IsKeyPressed(){return false;}
  virtual int ExecuteKeyCommand(){return 0;}

  virtual Long64_t GetEntries(){return fEntriesPrm->GetVal();}
  
protected:
  DisplayOutput()
    : fIsAnalysisBusyPrm(nullptr), fEntriesPrm(nullptr)
  {}
  Int_t fIsAnalysisBusy;
  Long64_t fEntries;

  TParameter<Int_t>* fIsAnalysisBusyPrm;
  TParameter<Long64_t>* fEntriesPrm;

};

#endif
