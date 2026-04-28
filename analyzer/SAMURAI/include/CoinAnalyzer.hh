#ifndef CoinAnalyzer_hh
#define CoinAnalyzer_hh

#include "BaseAnalyzer.hh"
#include <string>

class TArtCalibCoin;
class TH1;

class CoinAnalyzer : public BaseAnalyzer {
public:
  CoinAnalyzer(std::string name)
    : BaseAnalyzer(name){}
  virtual ~CoinAnalyzer() {}

  virtual bool Init() override;
  virtual void ReconstructData() override;
  virtual void Fill() override;
  virtual void ClearData() override;
  
protected:
  TArtCalibCoin* fCalibCoin = nullptr;
  TH1* fhcoin;
  static const int kNch = 8;// number of channels  
};

#endif
