#ifndef AnalysisManager_hh
#define AnalysisManager_hh

#include <vector>
#include <string>
#include <TDatime.h>

class TArtEventStore;
class THttpServer;
class HistogramManager;
class DisplayManager;
class BaseAnalyzer;
class TNamed;

class AnalysisManager {
public:
  AnalysisManager();
  virtual ~AnalysisManager();

  bool Initialize();

  void SetupHttpCommands(THttpServer* serv);
  
  bool ProcessEvent();

  void Finalize();

private:
  bool LoadConfig(const std::string& configPath);

  TArtEventStore* fEventStore;

  THttpServer* fHttpServer;

  HistogramManager* fHistManager;
  DisplayManager* fDispManager;

  TNamed* fServerTime;
  TDatime fDatime;

  std::vector<BaseAnalyzer*> fAnalyzers;

  bool fIsInitialized;
};

#endif
