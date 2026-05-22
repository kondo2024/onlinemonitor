#ifndef TOGAXSISiAnalyzer_hh
#define TOGAXSISiAnalyzer_hh

#include "BaseAnalyzer.hh"
#include <TString.h>
#include <string>

class TArtCalibTOGAXSISi;
class TH1;

class TOGAXSISiAnalyzer : public BaseAnalyzer {
public:
  TOGAXSISiAnalyzer(std::string name)
    : BaseAnalyzer(name){}
  virtual ~TOGAXSISiAnalyzer() {}

  virtual bool Init() override;
  virtual void ReconstructData() override;
  virtual void Fill() override;
  virtual void ClearData() override;
  
protected:
  TArtCalibTOGAXSISi* fCalibTOGAXSISi = nullptr;
  TString fdbFileName = "db/TOGAXSISi.xml";
  
  TH1* fhidstrip;
};

#endif
