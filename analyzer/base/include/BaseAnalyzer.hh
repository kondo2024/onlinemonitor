#ifndef BaseAnalyzer_hh
#define BaseAnalyzer_hh

#include <string>

class TArtEventStore;
class HistogramManager;

class BaseAnalyzer {
public:
  BaseAnalyzer(const std::string& name) : fName(name) {}
  virtual ~BaseAnalyzer() {}

  virtual bool Init() = 0;
  virtual void ReconstructData() = 0;
  virtual void Fill() = 0;
  virtual void ClearData() = 0;

  std::string GetName() const { return fName; }

protected:
  std::string fName;
};

#endif
