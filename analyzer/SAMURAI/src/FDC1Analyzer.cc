#include "FDC1Analyzer.hh"
#include "HistogramManager.hh"
#include "ConfigManager.hh"
#include "TArtSAMURAIParameters.hh"
#include "TArtCalibFDC1Hit.hh"
#include "TArtCalibFDC1Track.hh"

#include "TArtDCHit.hh"
#include "TArtDCTrack.hh"
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <iostream>

//--------------------------------------------------------
bool FDC1Analyzer::Init(){
  TArtSAMURAIParameters *smprm = TArtSAMURAIParameters::Instance();
  bool ret = smprm->LoadParameter((char*)fdbFileName.Data());
  if (!ret) return false;

  fCalibFDC1Hit = new TArtCalibFDC1Hit;
  fCalibFDC1Track = new TArtCalibFDC1Track;

  fCalibFDC1Track->SetTDCWindow(0,3000);
  ret = LoadDCTDCDistribution();

  HistogramManager* hm = HistogramManager::GetInstance();

  fhfdc1_idtl = hm->BookTH2("FDC1_idtl","FDC1 ID-T(leading);ID;T(leading)",
                            448,0.5,448.5,100,0,3000,"FDC1");

  fhfdc1_idtt = hm->BookTH2("FDC1_idtt","FDC1 ID-T(trailing);ID;T(trailing)",
                            448,0.5,448.5,100,0,3000,"FDC1");

  fhfdc1_idtot = hm->BookTH2("FDC1_idtot","FDC1 ID-ToT;ID;ToT",
                             448,0.5,448.5,100,0,1000,"FDC1");

  fhfdc1_xy = hm->BookTH2("FDC1_xy","FDC1 XY;X;Y",
                          100,-250,250, 100,-250,250,"FDC1");

  fhfdc1_xa = hm->BookTH2("FDC1_xa","FDC1 XA;X;A",
                          100,-250,250, 100,-0.5,0.5,"FDC1");

  fhfdc1_yb = hm->BookTH2("FDC1_yb","FDC1 YB;Y;B",
                          100,-250,250, 100,-0.5,0.5,"FDC1");

  return true;
}

//--------------------------------------------------------
void FDC1Analyzer::ReconstructData() {
  fCalibFDC1Hit->ReconstructData();
  if (fDoTracking){
    fCalibFDC1Track->ReconstructData();
  }
}

//--------------------------------------------------------
void FDC1Analyzer::Fill() {

  // FDC1 Hit
  TClonesArray *hit_array = fCalibFDC1Hit->GetDCHitArray();
  if (hit_array) {
    int n = hit_array->GetEntries();
    for (int i=0; i<n; ++i){
      TArtDCHit *hit = (TArtDCHit*)hit_array->At(i);
      Double_t id = hit->GetID();
      Double_t tl = hit->GetTDC();
      Double_t tt = hit->GetTrailTDC();
      fhfdc1_idtl->Fill(id,tl);
      fhfdc1_idtt->Fill(id,tt);
      fhfdc1_idtot->Fill(id,tt-tl);
    }
  }

  if (!fDoTracking) return; // skip tracking

  // FDC1 Track
  TClonesArray *FDC1Tracks = fCalibFDC1Track->GetDCTrackArray();

  Double_t FDC1_X = -9999;
  Double_t FDC1_Y = -9999;
  Double_t FDC1_A = -9999;
  Double_t FDC1_B = -9999;

  if(FDC1Tracks->GetEntries() > 0) {
    TArtDCTrack* TrackFDC1 = (TArtDCTrack *)FDC1Tracks->At(0);
    FDC1_X = TrackFDC1->GetPosition(0);
    FDC1_Y = TrackFDC1->GetPosition(1);
    FDC1_A = TrackFDC1->GetAngle(0);
    FDC1_B = TrackFDC1->GetAngle(1);
  }
  fhfdc1_xy->Fill(FDC1_X, FDC1_Y);
  fhfdc1_xa->Fill(FDC1_X, FDC1_A);
  fhfdc1_yb->Fill(FDC1_Y, FDC1_B);


}

//--------------------------------------------------------
void FDC1Analyzer::ClearData() {
  fCalibFDC1Hit->ClearData();
  fCalibFDC1Track->ClearData();
}

//--------------------------------------------------------
bool FDC1Analyzer::LoadDCTDCDistribution() {
  auto config = ConfigManager::GetInstance()->GetJson();
  if (config.contains("analyzer") && config["analyzer"].contains("dc_tdc_file")){
    fTDCDistFileName = config["analyzer"]["dc_tdc_file"];
  } else {
    std::cout << "\e[35m Define dc_tdc_file in config.json for FDC1 tracking \e[0m" << std::endl;
    return false;
  }

  TDirectory *pwd = gDirectory;
  TFile *RootFile = new TFile(fTDCDistFileName.c_str(),"READ");

  if(RootFile && RootFile->IsOpen()) {
    gROOT->cd();
    TH1D *Hist1D = NULL;
    Int_t FDC1NumberOfLayers = 14;

    for(Int_t i=0; i<FDC1NumberOfLayers; i++) {
      Hist1D = (TH1D*) RootFile->Get(Form("hfdc1tdc%d",i));

      if(Hist1D) {
        fCalibFDC1Track->SetTDCDistribution(Hist1D,i);
        delete Hist1D;
        Hist1D = NULL;
      }
      else {
        std::cout << "\e[35m Warning :: Could not find " << Form("hfdc1tdc%d",i) << "\e[0m" << std::endl;
      }
    }
    RootFile->Close();
  } else {
    fDoTracking = false;
    std::cout << "\e[35m Skip FDC1 tracking (No drift time distribution) \e[0m" << std::endl;
  }

  pwd->cd();
  return true;
}
