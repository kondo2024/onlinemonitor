#ifndef CanvasOutput_hh
#define CanvasOutput_hh

#include "DisplayOutput.hh"
#include <TCanvas.h>
#include <TStopwatch.h>
#include <vector>
#include <string>

class CanvasOutput : public DisplayOutput {
public:
  CanvasOutput();
  virtual ~CanvasOutput();

  bool Initialize() override;
  void Update() override;
  void RegisterAnalysisStatus() override;

  void Draw();

  bool IsKeyPressed() override;
  int ExecuteKeyCommand() override;

  void SetStopFlag(bool* ptr){fStopFlagPtr = ptr; }
  
private:
  bool CheckKeyboardInput();
  
  TCanvas* fCanvas;

  int fRows;
  int fCols;
  int fCurrentPage;
  double fIntervalMs;
  TStopwatch fTimer;
  std::vector<std::string> fSkipPaths;

  bool* fStopFlagPtr;
  
};

#endif
