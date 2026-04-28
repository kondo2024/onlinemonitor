#ifndef NINJAAnalyzer_hh
#define NINJAAnalyzer_hh

#include "BaseAnalyzer.hh"
#include <TString.h>
#include <string>

class TArtCalibNINJA;
class TH1;

class NINJAAnalyzer : public BaseAnalyzer {
public:
  NINJAAnalyzer(std::string name)
    : BaseAnalyzer(name){}
  virtual ~NINJAAnalyzer() {}

  virtual bool Init() override;
  virtual void ReconstructData() override;
  virtual void Fill() override;
  virtual void ClearData() override;
  
protected:
  TArtCalibNINJA* fCalibNINJA = nullptr;
  TString fdbFileName = "db/NINJA.xml";
  
  TH1* fhidlelu;
  TH1* fhidlerd;
  TH1* fhidtotlu;
  TH1* fhidtotrd;
  TH1* fhidxidy;
};

#endif
