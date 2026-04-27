#ifndef FDC1Analyzer_hh
#define FDC1Analyzer_hh

#include "BaseAnalyzer.hh"
#include <TString.h>
#include <string>

class TArtCalibFDC1Hit;
class TArtCalibFDC1Track;
class TH1;

class FDC1Analyzer : public BaseAnalyzer {
public:
  FDC1Analyzer(std::string name)
    : BaseAnalyzer(name){}
  virtual ~FDC1Analyzer() {}

  virtual bool Init() override;
  virtual void ReconstructData() override;
  virtual void Fill() override;
  virtual void ClearData() override;

  bool LoadDCTDCDistribution();

protected:
  TArtCalibFDC1Hit* fCalibFDC1Hit   = nullptr;
  TArtCalibFDC1Track* fCalibFDC1Track = nullptr;

  TString fdbFileName = "db/SAMURAIFDC1.xml";
  std::string fTDCDistFileName = "";

  TH1* fhfdc1_idtl; 
  TH1* fhfdc1_idtt; 
  TH1* fhfdc1_idtot;

  TH1* fhfdc1_xy;
  TH1* fhfdc1_xa;
  TH1* fhfdc1_yb;

  bool fDoTracking = true;
};

#endif
