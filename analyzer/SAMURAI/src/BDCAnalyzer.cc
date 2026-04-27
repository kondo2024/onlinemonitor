#include "BDCAnalyzer.hh"
#include "HistogramManager.hh"
#include "ConfigManager.hh"
#include "TArtSAMURAIParameters.hh"
#include "TArtCalibBDC1Hit.hh"
#include "TArtCalibBDC2Hit.hh"
#include "TArtCalibBDC1Track.hh"
#include "TArtCalibBDC2Track.hh"

#include "TArtDCHit.hh"
#include "TArtDCTrack.hh"
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <iostream>

//--------------------------------------------------------
bool BDCAnalyzer::Init(){
  TArtSAMURAIParameters *smprm = TArtSAMURAIParameters::Instance();
  bool ret = smprm->LoadParameter((char*)fdbFileName1.Data());
  if (!ret) return false;
  ret = smprm->LoadParameter((char*)fdbFileName2.Data());
  if (!ret) return false;

  fCalibBDC1Hit = new TArtCalibBDC1Hit;
  fCalibBDC2Hit = new TArtCalibBDC2Hit;
  fCalibBDC1Track = new TArtCalibBDC1Track;
  fCalibBDC2Track = new TArtCalibBDC2Track;

  fCalibBDC1Track->SetTDCWindow(0,3000);
  fCalibBDC2Track->SetTDCWindow(0,3000);
  ret = LoadDCTDCDistribution();
  
  HistogramManager* hm = HistogramManager::GetInstance();

  fhbdc1_idtl = hm->BookTH2("BDC1_idtl","BDC1 ID-T(leading);ID;T(leading)",
			    128,0.5,128.5,100,0,3000,"BDC");
  fhbdc2_idtl = hm->BookTH2("BDC2_idtl","BDC2 ID-T(leading);ID;T(leading)",
			    128,0.5,128.5,100,0,3000,"BDC");

  fhbdc1_idtt = hm->BookTH2("BDC1_idtt","BDC1 ID-T(trailing);ID;T(trailing)",
			    128,0.5,128.5,100,0,3000,"BDC");
  fhbdc2_idtt = hm->BookTH2("BDC2_idtt","BDC2 ID-T(trailing);ID;T(trailing)",
			    128,0.5,128.5,100,0,3000,"BDC");

  fhbdc1_idtot = hm->BookTH2("BDC1_idtot","BDC1 ID-ToT;ID;ToT",
			     128,0.5,128.5,100,0,1000,"BDC");
  fhbdc2_idtot = hm->BookTH2("BDC2_idtot","BDC2 ID-ToT;ID;ToT",
			     128,0.5,128.5,100,0,1000,"BDC");

  fhbdc1_xy = hm->BookTH2("BDC1_xy","BDC1 XY;X;Y",
			  100,-80,80, 100,-80,80,"BDC");
  fhbdc2_xy = hm->BookTH2("BDC2_xy","BDC2 XY;X;Y",
			  100,-80,80, 100,-80,80,"BDC");

  
  return true;
}
//--------------------------------------------------------
void BDCAnalyzer::ReconstructData() {
  fCalibBDC1Hit->ReconstructData();
  fCalibBDC2Hit->ReconstructData();
  if (fDoTracking){
    fCalibBDC1Track->ReconstructData();
    fCalibBDC2Track->ReconstructData();
  }
}
//--------------------------------------------------------
void BDCAnalyzer::Fill() {

  // BDC1 Hit
  TClonesArray *hit_array = fCalibBDC1Hit->GetDCHitArray();
  int n=hit_array->GetEntries();
  for (int i=0;i<n;++i){
    TArtDCHit *hit = (TArtDCHit*)hit_array->At(i);
    Double_t id = hit->GetID();
    Double_t tl = hit->GetTDC();
    Double_t tt = hit->GetTrailTDC();
    fhbdc1_idtl->Fill(id,tl);
    fhbdc1_idtt->Fill(id,tt);
    fhbdc1_idtot->Fill(id,tt-tl);
  }

  // BDC2 Hit
  hit_array = fCalibBDC2Hit->GetDCHitArray();
  n=hit_array->GetEntries();
  for (int i=0;i<n;++i){
    TArtDCHit *hit = (TArtDCHit*)hit_array->At(i);
    Double_t id = hit->GetID();
    Double_t tl = hit->GetTDC();
    Double_t tt = hit->GetTrailTDC();
    fhbdc2_idtl->Fill(id,tl);
    fhbdc2_idtt->Fill(id,tt);
    fhbdc2_idtot->Fill(id,tt-tl);
  }

  if (!fDoTracking) return;// skip tracking

  Double_t BDC1_X=-9999;
  Double_t BDC1_Y=-9999;
  Double_t BDC2_X=-9999;
  Double_t BDC2_Y=-9999;
  // BDC1 Track
  TClonesArray *BDC1Tracks = fCalibBDC1Track->GetDCTrackArray();
  
  if (BDC1Tracks) {
    Int_t BDC1NumberOfTracks = BDC1Tracks->GetEntries();
    Double_t TempXPosition, TempYPosition, TempChi2, MinChi2x =1e6, MinChi2y =1e6;
    //std::cout << BDC1NumberOfTracks << std::endl;
    if(BDC1NumberOfTracks > 0) {
      TArtDCTrack *TrackBDC1;
      
      for(Int_t i = 0; i<BDC1NumberOfTracks; i++) {
        TrackBDC1 = (TArtDCTrack *)BDC1Tracks->At(i);
        
	if(TrackBDC1) {

	  TempXPosition = TrackBDC1->GetPosition(0);
  	  TempYPosition = TrackBDC1->GetPosition(1);
	  TempChi2 = TrackBDC1->GetChi2() / (Double_t)TrackBDC1->GetNDF();
 
	  if(TempChi2 > 0) {
	  
	    if(TMath::Abs(TempXPosition) < 5000 && TempChi2 < MinChi2x) {
	      BDC1_X = TempXPosition;
//  	      fBDC1_ThetaX = TMath::ATan(TrackBDC1->GetAngle(0));
	      MinChi2x = TempChi2;
	    }	      

	    if(TMath::Abs(TempYPosition) < 5000 && TempChi2 < MinChi2y) {
	      BDC1_Y = TempYPosition;
  	      //fBDC1_ThetaY = TMath::ATan(TrackBDC1->GetAngle(1));
	      MinChi2y = TempChi2;
	    }	      
	  }
	}
      }

    //std::cout<<fBDC1_X<<" "<<fBDC1_Y<<std::endl;
      fhbdc1_xy->Fill(BDC1_X,BDC1_Y); 
    }  
  }

  // BDC2 Track
  TClonesArray *BDC2Tracks = fCalibBDC2Track->GetDCTrackArray();

  if (BDC2Tracks) {
    Int_t BDC2NumberOfTracks = BDC2Tracks->GetEntries();
    Double_t TempXPosition, TempYPosition, TempChi2, MinChi2x =1e6, MinChi2y =1e6;

    if(BDC2NumberOfTracks > 0) {
      TArtDCTrack *TrackBDC2;
      
      for(Int_t i = 0; i<BDC2NumberOfTracks; i++) {
        TrackBDC2 = (TArtDCTrack *)BDC2Tracks->At(i);
        
	if(TrackBDC2) {

	  TempXPosition = TrackBDC2->GetPosition(0);
  	  TempYPosition = TrackBDC2->GetPosition(1);
	  TempChi2 = TrackBDC2->GetChi2() / (Double_t)TrackBDC2->GetNDF();

	  if(TempChi2 > 0) {
	  
	    if(TMath::Abs(TempXPosition) < 5000 && TempChi2 < MinChi2x) {
	      BDC2_X = TempXPosition;
  	      //fBDC2_ThetaX = TMath::ATan(TrackBDC2->GetAngle(0));
	      MinChi2x = TempChi2;
	    }	      

	    if(TMath::Abs(TempYPosition) < 5000 && TempChi2 < MinChi2y) {
	      BDC2_Y = TempYPosition;
  	      //fBDC2_ThetaY = TMath::ATan(TrackBDC2->GetAngle(1));
	      MinChi2y = TempChi2;
	    }	      
	  }
	}
      }

      fhbdc2_xy->Fill(BDC2_X,BDC2_Y);  

    }      
  }

}
//--------------------------------------------------------
void BDCAnalyzer::ClearData() {
  fCalibBDC1Hit->ClearData();
  fCalibBDC2Hit->ClearData();
  fCalibBDC1Track->ClearData();
  fCalibBDC2Track->ClearData();
}
//--------------------------------------------------------
bool BDCAnalyzer::LoadDCTDCDistribution() {
  auto config = ConfigManager::GetInstance()->GetJson();
  if (config.contains("analysis") && config["analyzer"].contains("dc_tdc_file")){
    fTDCDistFileName = config["analyzer"]["dc_tdc_file"];
  }else{
    std::cout << "\e[35m " << "Define dc_tdc_file in config.json, if you need tracking " << "\e[0m" << std::endl;
    return false;
  }
  
  TDirectory *pwd = gDirectory;

  TFile *RootFile = new TFile(fTDCDistFileName.c_str(),"READ");

  if(RootFile->IsOpen()) {
    gROOT->cd();
    TH1D *Hist1D = NULL;
    Int_t BDCNumberOfLayers = 8;

    for(Int_t i=0; i<BDCNumberOfLayers; i++) {
      Hist1D = (TH1D*) RootFile->Get(Form("hbdc1tdc%d",i));

      if(Hist1D) {
	fCalibBDC1Track->SetTDCDistribution(Hist1D,i);
	delete Hist1D;
	Hist1D = NULL;
      }
      else
      std::cout << "\e[35m " << "Warning LoadTDCDistribution :: Could not find the following histogram " << Form("hbdc1tdc%d",i) << "\e[0m" << std::endl;
    }

    for(Int_t i=0; i<BDCNumberOfLayers; i++) {
      Hist1D = (TH1D*) RootFile->Get(Form("hbdc2tdc%d",i));

      if(Hist1D) {
	fCalibBDC2Track->SetTDCDistribution(Hist1D,i);
	delete Hist1D;
	Hist1D = NULL;
      }
      else
      std::cout << "\e[35m " << "Warning LoadTDCDistribution :: Could not find the following histogram " << Form("hbdc2tdc%d",i) << "\e[0m" << std::endl;
    }
  }else{
    fDoTracking = false;
    std::cout << "\e[35m " << "Skip BDC tracking " << "\e[0m" << std::endl;    
  }

  pwd->cd();
  return true;
}
//--------------------------------------------------------
