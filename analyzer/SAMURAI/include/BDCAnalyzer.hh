#ifndef BDCAnalyzer_hh
#define BDCAnalyzer_hh

#include "BaseAnalyzer.hh"
#include <TString.h>
#include <string>

class TArtCalibBDC1Hit;
class TArtCalibBDC2Hit;
class TArtCalibBDC1Track;
class TArtCalibBDC2Track;
class TH1;

class BDCAnalyzer : public BaseAnalyzer {
public:
  BDCAnalyzer(std::string name)
    : BaseAnalyzer(name){}
  virtual ~BDCAnalyzer() {}

  virtual bool Init() override;
  virtual void ReconstructData() override;
  virtual void Fill() override;
  virtual void ClearData() override;

  bool LoadDCTDCDistribution();
  
protected:
  TArtCalibBDC1Hit* fCalibBDC1Hit = nullptr;
  TArtCalibBDC2Hit* fCalibBDC2Hit = nullptr;
  TArtCalibBDC1Track* fCalibBDC1Track = nullptr;
  TArtCalibBDC2Track* fCalibBDC2Track = nullptr;
  TString fdbFileName1 = "db/SAMURAIBDC1.xml";
  TString fdbFileName2 = "db/SAMURAIBDC2.xml";
  std::string fTDCDistFileName = "";
  TH1* fhbdc1_idtl;
  TH1* fhbdc1_idtt;
  TH1* fhbdc1_idtot;
  TH1* fhbdc2_idtl;
  TH1* fhbdc2_idtt;
  TH1* fhbdc2_idtot;

  TH1* fhbdc1_xy;
  TH1* fhbdc2_xy;

  bool fDoTracking = true;
};

#endif
