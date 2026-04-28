#include "CoinAnalyzer.hh"
#include "HistogramManager.hh"
#include "TArtCalibCoin.hh"
#include <TH1.h>
#include <iostream>

//--------------------------------------------------------
bool CoinAnalyzer::Init(){
  fCalibCoin = new TArtCalibCoin;

  HistogramManager* hm = HistogramManager::GetInstance();

  fhcoin = hm->BookTH1("Coin","Coin;ID;Counts",
		       10,0.5,10.5,"Coin");
  return true;
}
//--------------------------------------------------------
void CoinAnalyzer::ReconstructData() {
  fCalibCoin->ReconstructData();
}
//--------------------------------------------------------
void CoinAnalyzer::Fill() {
  fCalibCoin->ReconstructData();
  for (int i=1;i<kNch+1;++i){ // start from ch=1
    if(fCalibCoin->IsChTrue(i)) fhcoin->Fill(i);;
  }
}
//--------------------------------------------------------
void CoinAnalyzer::ClearData() {
  fCalibCoin->ClearData();
}
//--------------------------------------------------------
//--------------------------------------------------------
