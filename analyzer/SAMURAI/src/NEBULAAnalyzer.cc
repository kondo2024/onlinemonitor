#include "NEBULAAnalyzer.hh"
#include "HistogramManager.hh"
#include "TArtSAMURAIParameters.hh"
#include "TArtCalibNEBULA.hh"
#include "TArtNEBULAPla.hh"
#include <TH1.h>
#include <TH2.h>
#include <iostream>

//--------------------------------------------------------
bool NEBULAAnalyzer::Init(){
  TArtSAMURAIParameters *smprm = TArtSAMURAIParameters::Instance();
  bool ret = smprm->LoadParameter((char*)fdbFileName.Data());
  if (!ret) return false;

  fCalibNEBULA = new TArtCalibNEBULA;

  HistogramManager* hm = HistogramManager::GetInstance();

  fhidtu = hm->BookTH2("NEBULA_idtu","NEBULA ID-TUraw;ID;TUraw",
		       144,0.5,144.5,100,0,80000,"NEBULA");
  fhidtd = hm->BookTH2("NEBULA_idtd","NEBULA ID-TDraw;ID;TDraw",
		       144,0.5,144.5,100,0,80000,"NEBULA");
  fhidqu = hm->BookTH2("NEBULA_idqu","NEBULA ID-QUraw;ID;QUraw",
		       144,0.5,144.5,100,0,4000,"NEBULA");
  fhidqd = hm->BookTH2("NEBULA_idqd","NEBULA ID-QDraw;ID;QDraw",
		       144,0.5,144.5,100,0,4000,"NEBULA");
  return true;
}
//--------------------------------------------------------
void NEBULAAnalyzer::ReconstructData() {
  fCalibNEBULA->ReconstructData();
}
//--------------------------------------------------------
void NEBULAAnalyzer::Fill() {
  TClonesArray *array = fCalibNEBULA->GetNEBULAPlaArray();
  int n=array->GetEntries();
  for (int i=0;i<n;++i){
    TArtNEBULAPla *pla = (TArtNEBULAPla*)array->At(i);
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
void NEBULAAnalyzer::ClearData() {
  fCalibNEBULA->ClearData();
}
//--------------------------------------------------------
//--------------------------------------------------------
