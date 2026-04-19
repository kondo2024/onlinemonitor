#ifndef DisplayOutput_hh
#define DisplayOutput_hh

#include <string>
#include <Rtypes.h>

class DisplayOutput {
public:
  virtual ~DisplayOutput() {}

  virtual bool Initialize() = 0;
  virtual void Update() = 0;
  virtual void RegisterAnalysisStatus(Int_t* busyPtr) = 0;
  static DisplayOutput* Create(const std::string& mode);

protected:
  DisplayOutput() {}
};

#endif
