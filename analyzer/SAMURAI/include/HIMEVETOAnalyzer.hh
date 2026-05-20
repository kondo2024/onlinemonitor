#ifndef HIMEVETOAnalyzer_hh
#define HIMEVETOAnalyzer_hh

#include "BaseAnalyzer.hh"
#include <TString.h>
#include <string>

class TArtCalibHIMEVETO;
class TH1;

class HIMEVETOAnalyzer : public BaseAnalyzer {
public:
  HIMEVETOAnalyzer(std::string name)
    : BaseAnalyzer(name){}
  virtual ~HIMEVETOAnalyzer() {}

  virtual bool Init() override;
  virtual void ReconstructData() override;
  virtual void Fill() override;
  virtual void ClearData() override;
  
protected:
  TArtCalibHIMEVETO* fCalibHIMEVETO = nullptr;
  TString fdbFileName = "db/HIMEVETO.xml";
  
  TH1* fhidtl0;
  TH1* fhidtl1;
  TH1* fhidtot0;
  TH1* fhidtot1;
};

#endif
