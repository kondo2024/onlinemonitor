#include "HODAnalyzer.hh"
#include "HistogramManager.hh"
#include "TArtSAMURAIParameters.hh"
#include "TArtCalibHODPla.hh"
#include "TArtHODPla.hh"
#include <TH1.h>
#include <TH2.h>
#include <iostream>

//--------------------------------------------------------
bool HODAnalyzer::Init(){
  TArtSAMURAIParameters *smprm = TArtSAMURAIParameters::Instance();
  bool ret = smprm->LoadParameter((char*)fdbFileName.Data());
  if (!ret) return false;

  fCalibHODPla = new TArtCalibHODPla;

  HistogramManager* hm = HistogramManager::GetInstance();

  fhidtu = hm->BookTH2("HOD_idtu","HOD ID-TUraw;ID;TUraw",
		       40,0.5,40.5,100,0,40000,"HOD");
  fhidtd = hm->BookTH2("HOD_idtd","HOD ID-TDraw;ID;TDraw",
		       40,0.5,40.5,100,0,40000,"HOD");
  fhidqu = hm->BookTH2("HOD_idqu","HOD ID-QUraw;ID;QUraw",
		       40,0.5,40.5,100,0,4000,"HOD");
  fhidqd = hm->BookTH2("HOD_idqd","HOD ID-QDraw;ID;QDraw",
		       40,0.5,40.5,100,0,4000,"HOD");

  return true;
}
//--------------------------------------------------------
void HODAnalyzer::ReconstructData() {
  fCalibHODPla->ReconstructData();
}
//--------------------------------------------------------
void HODAnalyzer::Fill() {
  TClonesArray *array = fCalibHODPla->GetHODPlaArray();
  int n=array->GetEntries();
  for (int i=0;i<n;++i){
    TArtHODPla *pla = (TArtHODPla*)array->At(i);
    Double_t id = pla->GetID();
    Double_t turaw = pla->GetTURaw();
    Double_t tdraw = pla->GetTDRaw();
    Double_t quraw = pla->GetQURaw();
    Double_t qdraw = pla->GetQDRaw();

    fhidtu->Fill(id,turaw);
    fhidtd->Fill(id,tdraw);
    fhidqu->Fill(id,quraw);
    fhidqd->Fill(id,qdraw);
  }
}
//--------------------------------------------------------
void HODAnalyzer::ClearData() {
  fCalibHODPla->ClearData();
}
//--------------------------------------------------------
//--------------------------------------------------------
