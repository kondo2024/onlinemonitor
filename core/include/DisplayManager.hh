#ifndef DisplayManager_hh
#define DisplayManager_hh

#include <TDatime.h>

#include <string>
#include <vector>
#include <map>
class THttpServer;
class TNamed;

class DisplayManager {
public:
  DisplayManager();
  virtual ~DisplayManager();
  
  bool Initialize();
  void SetServerTime();

private:
  void SetupHttpCommands(THttpServer* serv);
  
  THttpServer* fHttpServer;
  TNamed* fServerTimeStr;
  TDatime fDatime;
  
};

#endif
