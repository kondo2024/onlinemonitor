#include "PlasticAnalyzer.hh"
#include "HistogramManager.hh"
#include "TArtBigRIPSParameters.hh"
#include "TArtCalibPlastic.hh"
#include "TArtPlastic.hh"
#include <TH1.h>
#include <TH2.h>
#include <iostream>

//--------------------------------------------------------
bool PlasticAnalyzer::Init(){
  TArtBigRIPSParameters *brprm = TArtBigRIPSParameters::Instance();
  bool ret = brprm->LoadParameter((char*)fdbFileName.Data());
  if (!ret) return false;

  fCalibPlastic = new TArtCalibPlastic;

  HistogramManager* hm = HistogramManager::GetInstance();

  fhidtl = hm->BookTH2("Plastic_idtl","Plastic ID-TLraw;ID;TLraw",
		       10,0.5,10.5,100,0,80000,"Plastic");
  fhidtr = hm->BookTH2("Plastic_idtr","Plastic ID-TRraw;ID;TRraw",
		       10,0.5,10.5,100,0,80000,"Plastic");
  fhidql = hm->BookTH2("Plastic_idql","Plastic ID-QLraw;ID;QLraw",
		       10,0.5,10.5,100,0,4000,"Plastic");
  fhidqr = hm->BookTH2("Plastic_idqr","Plastic ID-QRraw;ID;QRraw",
		       10,0.5,10.5,100,0,4000,"Plastic");
  return true;
}
//--------------------------------------------------------
void PlasticAnalyzer::ReconstructData() {
  fCalibPlastic->ReconstructData();
}
//--------------------------------------------------------
void PlasticAnalyzer::Fill() {
  TClonesArray *array = fCalibPlastic->GetPlasticArray();
  int n=array->GetEntries();
  for (int i=0;i<n;++i){
    TArtPlastic *pla = (TArtPlastic*)array->At(i);
    Double_t id = pla->GetID();
    Double_t tlraw = pla->GetTLRaw();
    Double_t trraw = pla->GetTRRaw();
    Double_t qlraw = pla->GetQLRaw();
    Double_t qrraw = pla->GetQRRaw();

    fhidtl->Fill(id,tlraw);
    fhidtr->Fill(id,trraw);
    fhidql->Fill(id,qlraw);
    fhidqr->Fill(id,qrraw);
  }
}
//--------------------------------------------------------
void PlasticAnalyzer::ClearData() {
  fCalibPlastic->ClearData();
}
//--------------------------------------------------------
//--------------------------------------------------------
