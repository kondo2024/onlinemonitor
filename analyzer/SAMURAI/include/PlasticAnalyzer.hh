#ifndef PlasticAnalyzer_hh
#define PlasticAnalyzer_hh

#include "BaseAnalyzer.hh"
#include <TString.h>
#include <string>

class TArtCalibPlastic;
class TH1;

class PlasticAnalyzer : public BaseAnalyzer {
public:
  PlasticAnalyzer(std::string name)
    : BaseAnalyzer(name){}
  virtual ~PlasticAnalyzer() {}

  virtual bool Init() override;
  virtual void ReconstructData() override;
  virtual void Fill() override;
  virtual void ClearData() override;
  
protected:
  TArtCalibPlastic* fCalibPlastic = nullptr;
  TString fdbFileName = "db/SAMURAIPlastic.xml";
  
  TH1* fhidtl;
  TH1* fhidtr;
  TH1* fhidql;
  TH1* fhidqr;
};

#endif
