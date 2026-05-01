#include "FDC2Analyzer.hh"
#include "HistogramManager.hh"
#include "ConfigManager.hh"
#include "TArtSAMURAIParameters.hh"
#include "TArtCalibFDC2Hit.hh"
#include "TArtCalibFDC2Track.hh"

#include "TArtDCHit.hh"
#include "TArtDCTrack.hh"
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <iostream>

//--------------------------------------------------------
bool FDC2Analyzer::Init(){
  TArtSAMURAIParameters *smprm = TArtSAMURAIParameters::Instance();
  bool ret = smprm->LoadParameter((char*)fdbFileName.Data());
  if (!ret) return false;

  fCalibFDC2Hit = new TArtCalibFDC2Hit;
  fCalibFDC2Track = new TArtCalibFDC2Track;

  fCalibFDC2Track->SetTDCWindow(0,4000);
  ret = LoadDCTDCDistribution();

  HistogramManager* hm = HistogramManager::GetInstance();

  fhfdc2_idtl = hm->BookTH2("FDC2_idtl","FDC2 ID-T(leading);ID;T(leading)",
                            1568,0.5,1568.5,100,0,4000,"FDC2");

  fhfdc2_idtt = hm->BookTH2("FDC2_idtt","FDC2 ID-T(trailing);ID;T(trailing)",
                            1568,0.5,1568.5,100,0,4000,"FDC2");

  fhfdc2_idtot = hm->BookTH2("FDC2_idtot","FDC2 ID-ToT;ID;ToT",
                             1568,0.5,1568.5,100,0,1000,"FDC2");

  fhfdc2_xy = hm->BookTH2("FDC2_xy","FDC2 XY;X;Y",
                          100,-1200,1200, 100,-600,600,"FDC2");

  fhfdc2_xa = hm->BookTH2("FDC2_xa","FDC2 XA;X;A",
                          100,-1200,1200, 100,-0.5,0.5,"FDC2");

  fhfdc2_yb = hm->BookTH2("FDC2_yb","FDC2 YB;X;B",
                          100,-1200,1200, 100,-0.5,0.5,"FDC2");

  return true;
}

//--------------------------------------------------------
void FDC2Analyzer::ReconstructData() {
  fCalibFDC2Hit->ReconstructData();
  if (fDoTracking){
    fCalibFDC2Track->ReconstructData();
  }
}

//--------------------------------------------------------
void FDC2Analyzer::Fill() {

  // FDC2 Hit
  TClonesArray *hit_array = fCalibFDC2Hit->GetDCHitArray();
  if (hit_array) {
    int n = hit_array->GetEntries();
    for (int i=0; i<n; ++i){
      TArtDCHit *hit = (TArtDCHit*)hit_array->At(i);
      Double_t id = hit->GetID();
      Double_t tl = hit->GetTDC();
      Double_t tt = hit->GetTrailTDC();
      fhfdc2_idtl->Fill(id,tl);
      fhfdc2_idtt->Fill(id,tt);
      fhfdc2_idtot->Fill(id,tt-tl);
    }
  }

  if (!fDoTracking) return;

  // FDC2 Track
  TClonesArray *FDC2Tracks = fCalibFDC2Track->GetDCTrackArray();

  Double_t FDC2_X = -9999;
  Double_t FDC2_Y = -9999;
  Double_t FDC2_A = -9999;
  Double_t FDC2_B = -9999;

  if(FDC2Tracks->GetEntries() > 0) {
    TArtDCTrack* TrackFDC2 = (TArtDCTrack *)FDC2Tracks->At(0);
    FDC2_X = TrackFDC2->GetPosition(0);
    FDC2_Y = TrackFDC2->GetPosition(1);
    FDC2_A = TrackFDC2->GetAngle(0);
    FDC2_B = TrackFDC2->GetAngle(1);
  }
  fhfdc2_xy->Fill(FDC2_X, FDC2_Y);
  fhfdc2_xa->Fill(FDC2_X, FDC2_A);
  fhfdc2_yb->Fill(FDC2_Y, FDC2_B);
  
}

//--------------------------------------------------------
void FDC2Analyzer::ClearData() {
  fCalibFDC2Hit->ClearData();
  fCalibFDC2Track->ClearData();
}

//--------------------------------------------------------
bool FDC2Analyzer::LoadDCTDCDistribution() {
  auto config = ConfigManager::GetInstance()->GetJson();
  if (config.contains("analyzer") && config["analyzer"].contains("dc_tdc_file")){
    fTDCDistFileName = config["analyzer"]["dc_tdc_file"];
  } else {
    std::cout << "\e[35m Define dc_tdc_file in config.json for FDC2 tracking \e[0m" << std::endl;
    return false;
  }

  TDirectory *pwd = gDirectory;
  TFile *RootFile = new TFile(fTDCDistFileName.c_str(),"READ");

  if(RootFile && RootFile->IsOpen()) {
    gROOT->cd();
    TH1D *Hist1D = NULL;
    Int_t FDC2NumberOfLayers = 14; 

    for(Int_t i=0; i<FDC2NumberOfLayers; i++) {
      Hist1D = (TH1D*) RootFile->Get(Form("hfdc2tdc%d",i));

      if(Hist1D) {
        fCalibFDC2Track->SetTDCDistribution(Hist1D,i);
        delete Hist1D;
        Hist1D = NULL;
      }
      else {
        std::cout << "\e[35m Warning :: Could not find " << Form("hfdc2tdc%d",i) << "\e[0m" << std::endl;
      }
    }
    RootFile->Close();
  } else {
    fDoTracking = false;
    std::cout << "\e[35m Skip FDC2 tracking \e[0m" << std::endl;
  }

  pwd->cd();
  return true;
}
