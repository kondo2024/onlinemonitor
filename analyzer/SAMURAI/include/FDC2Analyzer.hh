#ifndef FDC2Analyzer_hh
#define FDC2Analyzer_hh

#include "BaseAnalyzer.hh"
#include <TString.h>
#include <string>

class TArtCalibFDC2Hit;
class TArtCalibFDC2Track;
class TH1;

class FDC2Analyzer : public BaseAnalyzer {
public:
  FDC2Analyzer(std::string name)
    : BaseAnalyzer(name){}
  virtual ~FDC2Analyzer() {}

  virtual bool Init() override;
  virtual void ReconstructData() override;
  virtual void Fill() override;
  virtual void ClearData() override;

  bool LoadDCTDCDistribution();

protected:
  TArtCalibFDC2Hit* fCalibFDC2Hit   = nullptr;
  TArtCalibFDC2Track* fCalibFDC2Track = nullptr;

  TString fdbFileName = "db/SAMURAIFDC2.xml";
  std::string fTDCDistFileName = "";

  TH1* fhfdc2_idtl;
  TH1* fhfdc2_idtt;
  TH1* fhfdc2_idtot;

  TH1* fhfdc2_xy;
  TH1* fhfdc2_xa;
  TH1* fhfdc2_yb;

  bool fDoTracking = true;
};

#endif
