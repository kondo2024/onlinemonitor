#include "HIMEVETOAnalyzer.hh"
#include "HistogramManager.hh"
#include "TArtSAMURAIParameters.hh"
#include "TArtCalibHIMEVETO.hh"
#include "TArtHIMEVETOPla.hh"
#include <TH1.h>
#include <TH2.h>
#include <iostream>

//--------------------------------------------------------
bool HIMEVETOAnalyzer::Init(){
  TArtSAMURAIParameters *smprm = TArtSAMURAIParameters::Instance();
  bool ret = smprm->LoadParameter((char*)fdbFileName.Data());
  if (!ret) return false;

  fCalibHIMEVETO = new TArtCalibHIMEVETO;

  HistogramManager* hm = HistogramManager::GetInstance();

  fhidtl0 = hm->BookTH2("HIMEVETO_idtl0","HIMEVETO ID-Tleading0;ID;Tleading0",
			3,0.5,3.5,100,0,100000,"HIMEVETO");
  fhidtl1 = hm->BookTH2("HIMEVETO_idtl1","HIMEVETO ID-Tleading1;ID;Tleading1",
			3,0.5,3.5,100,0,100000,"HIMEVETO");
  fhidtot0 = hm->BookTH2("HIMEVETO_idtot0","HIMEVETO ID-TOT0;ID;TOT0",
			 3,0.5,3.5,100,0,2000,"HIMEVETO");
  fhidtot1 = hm->BookTH2("HIMEVETO_idtot1","HIMEVETO ID-TOT1;ID;TOT1",
			 3,0.5,3.5,100,0,2000,"HIMEVETO");
  return true;
}
//--------------------------------------------------------
void HIMEVETOAnalyzer::ReconstructData() {
  fCalibHIMEVETO->ReconstructData();
}
//--------------------------------------------------------
void HIMEVETOAnalyzer::Fill() {
  TClonesArray *array = fCalibHIMEVETO->GetHIMEVETOPlaArray();
  int n=array->GetEntries();
  for (int i=0;i<n;++i){
    TArtHIMEVETOPla *pla = (TArtHIMEVETOPla*)array->At(i);
    Double_t id = pla->fID;
    Double_t tl0 = pla->fT_LERaw[0];
    Double_t tl1 = pla->fT_LERaw[1];
    Double_t tot0 = pla->fTOTCal[0];
    Double_t tot1 = pla->fTOTCal[1];

    fhidtl0->Fill(id,tl0);
    fhidtl1->Fill(id,tl1);
    fhidtot0->Fill(id,tot0);
    fhidtot1->Fill(id,tot1);
  }
}
//--------------------------------------------------------
void HIMEVETOAnalyzer::ClearData() {
  fCalibHIMEVETO->ClearData();
}
//--------------------------------------------------------
//--------------------------------------------------------
