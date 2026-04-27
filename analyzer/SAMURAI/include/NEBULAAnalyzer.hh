#ifndef NEBULAAnalyzer_hh
#define NEBULAAnalyzer_hh

#include "BaseAnalyzer.hh"
#include <TString.h>
#include <string>

class TArtCalibNEBULA;
class TH1;

class NEBULAAnalyzer : public BaseAnalyzer {
public:
  NEBULAAnalyzer(std::string name)
    : BaseAnalyzer(name){}
  virtual ~NEBULAAnalyzer() {}

  virtual bool Init() override;
  virtual void ReconstructData() override;
  virtual void Fill() override;
  virtual void ClearData() override;
  
protected:
  TArtCalibNEBULA* fCalibNEBULA = nullptr;
  TString fdbFileName = "db/NEBULA.xml";
  
  TH1* fhidtu;
  TH1* fhidtd;
  TH1* fhidqu;
  TH1* fhidqd;
};

#endif
