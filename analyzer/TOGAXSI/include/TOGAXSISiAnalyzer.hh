#ifndef TOGAXSISiAnalyzer_hh
#define TOGAXSISiAnalyzer_hh

#include "BaseAnalyzer.hh"
#include <TString.h>
#include <string>

class TArtCalibTOGAXSISi;
class TArtRecoTOGAXSIVertex;
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
  
  virtual void SetEntriesPtr(Long64_t *ptr){fEntriesPtr = ptr;}
  
protected:
  TArtCalibTOGAXSISi* fCalibTOGAXSISi = nullptr;
  TArtRecoTOGAXSIVertex *fRecoTOGAXSIVertex = nullptr;
  TString fdbFileName = "db/TOGAXSISi.xml";

  Long64_t *fEntriesPtr = nullptr;
  
  TH1* fhidstrip;

  TH1* fhvxy;
  TH1* fhvzx;
  TH1* fhvzy;
  TH1* fhentvy;
};

#endif
