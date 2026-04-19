#ifndef DisplayOutput_hh
#define DisplayOutput_hh

#include <string>
#include <Rtypes.h>
#include <TParameter.h>

class DisplayOutput {
public:
  virtual ~DisplayOutput() {}

  virtual bool Initialize() = 0;
  virtual void Update() = 0;
  virtual void RegisterAnalysisStatus(Int_t* busyPtr) = 0;

  virtual bool IsKeyPressed(){return false;}
  virtual int ExecuteKeyCommand(){return 0;}
  
protected:
  DisplayOutput()
    : fIsAnalysisBusy(nullptr)
  {}
  TParameter<Int_t>* fIsAnalysisBusy;

};

#endif
