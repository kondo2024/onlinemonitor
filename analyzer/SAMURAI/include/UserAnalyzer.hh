#ifndef UserAnalyzer_hh
#define UserAnalyzer_hh

#include "BaseAnalyzer.hh"
#include <TString.h>
#include <string>

class TH1;

class UserAnalyzer : public BaseAnalyzer {
public:
  UserAnalyzer(std::string name)
    : BaseAnalyzer(name){}
  virtual ~UserAnalyzer() {}

  virtual bool Init() override;
  virtual void ReconstructData() override;
  virtual void Fill() override;
  virtual void ClearData() override;
  
protected:
  TH1* fhsbt1ql;
  TH1* fhsbt1qr;
  TH1* fhsbttdiff21;
};

#endif
