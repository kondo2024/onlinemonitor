#include "NINJAAnalyzer.hh"
#include "HistogramManager.hh"
#include "TArtSAMURAIParameters.hh"
#include "TArtCalibNINJA.hh"
#include "TArtNINJAPla.hh"
#include <TH1.h>
#include <TH2.h>
#include <iostream>

//--------------------------------------------------------
bool NINJAAnalyzer::Init(){
  TArtSAMURAIParameters *smprm = TArtSAMURAIParameters::Instance();
  bool ret = smprm->LoadParameter((char*)fdbFileName.Data());
  if (!ret) return false;

  fCalibNINJA = new TArtCalibNINJA;

  HistogramManager* hm = HistogramManager::GetInstance();

  fhidlelu = hm->BookTH2("NINJA_idlelu","NINJA ID LELU",
			 30,0.5,30.5,100,0,80000, "NINJA");
  fhidlerd = hm->BookTH2("NINJA_idlerd","NINJA ID LERD",
			 30,0.5,30.5,100,0,80000,"NINJA");
  fhidtotlu = hm->BookTH2("NINJA_idtotlu","NINJA ID TOTLU",
			  30,0.5,30.5,100,0,100, "NINJA");
  fhidtotrd = hm->BookTH2("NINJA_idtotrd","NINJA ID TOTRD",
			  30,0.5,30.5,100,0,100, "NINJA");

  fhidxidy = hm->BookTH2("NINJA_idxidy","NINJA IDX IDY",
			 30,0.5,30.5,30,0.5,30.5, "NINJA");
  
  return true;
}
//--------------------------------------------------------
void NINJAAnalyzer::ReconstructData() {
  fCalibNINJA->ReconstructData();
}
//--------------------------------------------------------
void NINJAAnalyzer::Fill() {

  TClonesArray *array = fCalibNINJA->GetNINJAPlaArray();
  int n=array->GetEntries();
  int id_x = -9999;
  int id_y = -9999;

  for (int i=0;i<n;++i){
    TArtNINJAPla *NINJA = (TArtNINJAPla*)array->At(i);
    Double_t id = NINJA->fID;
    Double_t LERawLU = NINJA->fT_LERaw[0];
    Double_t LERawRD = NINJA->fT_LERaw[1];
    Double_t TOTLU = NINJA->fTOTCal[0];
    Double_t TOTRD = NINJA->fTOTCal[1];
    Double_t avgtime = (TOTLU + TOTRD)/2.;
    Double_t avgTOT = pow(TOTLU * TOTRD,0.5);


    fhidlelu->Fill(id,LERawLU);
    fhidlerd->Fill(id,LERawRD);
    fhidtotlu->Fill(id,TOTLU);
    fhidtotrd->Fill(id,TOTRD);

    if (id < 18 ) id_x = id;
    if (id > 17 ) id_y = id;

    if (id_x >-1 && id_y>-1) fhidxidy->Fill(id_x,id_y);
 
 }

}
//--------------------------------------------------------
void NINJAAnalyzer::ClearData() {
  fCalibNINJA->ClearData();
}
//--------------------------------------------------------
//--------------------------------------------------------
