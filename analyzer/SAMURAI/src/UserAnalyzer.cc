#include "UserAnalyzer.hh"
#include "HistogramManager.hh"
#include "TArtStoreManager.hh"
#include "TArtPlastic.hh"
#include <TH1.h>
#include <TH2.h>
#include <TClonesArray.h>
#include <iostream>

//--------------------------------------------------------
bool UserAnalyzer::Init(){

  HistogramManager* hm = HistogramManager::GetInstance();

  fhsbt1ql = hm->BookTH1("SBT1QL","SBT1 QL;QLraw;Counts",
			 200,0,2000,"User");
  fhsbt1qr = hm->BookTH1("SBT1QR","SBT1 QR;QRraw;Counts",
			 200,0,2000,"User");
  fhsbttdiff21 = hm->BookTH1("SBTTaveDiff21","SBT Tave2-Tave1;Tave2-Tave1;Counts",
			     200,-5,5,"User");
  return true;
}
//--------------------------------------------------------
void UserAnalyzer::ReconstructData() {
}
//--------------------------------------------------------
void UserAnalyzer::Fill() {

  TArtStoreManager* sman = TArtStoreManager::Instance();
  TClonesArray* pla_array = (TClonesArray *)sman->FindDataContainer("BigRIPSPlastic");
  if (pla_array==NULL){
    std::cout<<"onlinemonitor: Please include Plastic in config.json for PID at BDCAnalyzer"<<std::endl;
    return;
  }

  TArtPlastic *f13pla1 = nullptr;
  TArtPlastic *f13pla2 = nullptr;
  
  int npla = pla_array->GetEntries();
  for (int i=0;i<npla;++i){
    TArtPlastic *pla = (TArtPlastic*)pla_array->At(i);
    int id = pla->GetID();
    if      (id==4) f13pla1 = pla;
    else if (id==5) f13pla2 = pla;
  }

  if (f13pla1){
    fhsbt1ql->Fill(f13pla1->GetQLRaw());
    fhsbt1qr->Fill(f13pla1->GetQRRaw());

    if (f13pla2){
      double tave1 = f13pla1->GetTime();
      double tave2 = f13pla2->GetTime();
      fhsbttdiff21->Fill(tave2-tave1);
    }
  }


  if (fhsbt1ql->Integral()>10) fhsbt1ql->Fit("gaus","Q");
  if (fhsbt1qr->Integral()>10) fhsbt1qr->Fit("gaus","Q");
  if (fhsbttdiff21->Integral()>10) fhsbttdiff21->Fit("gaus","Q");
  
}
//--------------------------------------------------------
void UserAnalyzer::ClearData() {
}
//--------------------------------------------------------
//--------------------------------------------------------
