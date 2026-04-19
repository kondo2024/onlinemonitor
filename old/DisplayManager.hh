#ifndef DisplayManager_hh
#define DisplayManager_hh

#include <TDatime.h>
#include <TParameter.h>

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

  bool IsBusy(){return fIsBusy;}
  void SetBusy(Int_t i){fIsBusy->SetVal(i);}
  
private:
  void SetupHttpCommands(THttpServer* serv);
  
  THttpServer* fHttpServer;
  TNamed* fServerTimeStr;
  TDatime fDatime;
  TParameter<Int_t>* fIsBusy;
  
};

#endif
