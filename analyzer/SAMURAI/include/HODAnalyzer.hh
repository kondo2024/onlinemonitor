#ifndef HODAnalyzer_hh
#define HODAnalyzer_hh

#include "BaseAnalyzer.hh"
#include <TString.h>
#include <string>

class TArtCalibHODPla;
class TH1;

class HODAnalyzer : public BaseAnalyzer {
public:
  HODAnalyzer(std::string name)
    : BaseAnalyzer(name){}
  virtual ~HODAnalyzer() {}

  virtual bool Init() override;
  virtual void ReconstructData() override;
  virtual void Fill() override;
  virtual void ClearData() override;
  
protected:
  TArtCalibHODPla* fCalibHODPla = nullptr;
  TString fdbFileName = "db/SAMURAIHOD.xml";
  
  TH1* fhidtu;
  TH1* fhidtd;
  TH1* fhidqu;
  TH1* fhidqd;
};

#endif
