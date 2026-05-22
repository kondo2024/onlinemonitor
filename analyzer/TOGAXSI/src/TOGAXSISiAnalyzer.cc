#include "TOGAXSISiAnalyzer.hh"
#include "HistogramManager.hh"
#include "TArtTOGAXSIParameters.hh"
#include "TArtCalibTOGAXSISi.hh"
#include "TArtTOGAXSISi.hh"
#include <TH1.h>
#include <TH2.h>
#include <TClonesArray.h>
#include <iostream>

//--------------------------------------------------------
bool TOGAXSISiAnalyzer::Init(){
  TArtTOGAXSIParameters *prm = TArtTOGAXSIParameters::Instance();
  bool ret = prm->LoadParameter((char*)fdbFileName.Data());
  if (!ret) return false;

  fCalibTOGAXSISi = new TArtCalibTOGAXSISi;

  HistogramManager* hm = HistogramManager::GetInstance();

  fhidstrip = hm->BookTH2("TOGAXSISi_idstrip","TOGAXSI Si ID stripID",
			  20,0.5,20.5,200,0,1200, "TOGAXSI");
  return true;
}
//--------------------------------------------------------
void TOGAXSISiAnalyzer::ReconstructData() {
  fCalibTOGAXSISi->ReconstructData();
}
//--------------------------------------------------------
void TOGAXSISiAnalyzer::Fill() {

  TClonesArray *array = fCalibTOGAXSISi->GetSiArray();
  int n = array->GetEntries();
  int id_x = -9999;
  int id_y = -9999;

  for (int i=0;i<n;++i){
    TArtTOGAXSISi *si = (TArtTOGAXSISi*)array->At(i);
    Int_t nhit = si->GetMulti();
    Double_t id = si->GetID();

    for (int j=0;j<nhit;++j){
      Double_t stripid = si->GetStripID(j);
      fhidstrip->Fill(id,stripid);
    }
 
 }

}
//--------------------------------------------------------
void TOGAXSISiAnalyzer::ClearData() {
  fCalibTOGAXSISi->ClearData();
}
//--------------------------------------------------------
//--------------------------------------------------------
