#ifndef AnalysisManager_hh
#define AnalysisManager_hh

#include <vector>
#include <string>
#include <TDatime.h>

class TArtEventStore;
class HistogramManager;
class DisplayManager;
class BaseAnalyzer;

class AnalysisManager {
public:
  AnalysisManager(HistogramManager* histManager);
  virtual ~AnalysisManager();

  bool Initialize();

  bool ProcessEvent();

  void Finalize();

  void SetDisplayManager(DisplayManager* displayManager){fDispManager = displayManager;}
  
private:
  TArtEventStore* fEventStore;

  HistogramManager* fHistManager;
  DisplayManager* fDispManager;

  std::vector<BaseAnalyzer*> fAnalyzers;

  bool fIsInitialized;
};

#endif
