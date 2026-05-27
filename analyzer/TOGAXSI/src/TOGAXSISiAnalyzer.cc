#include "TOGAXSISiAnalyzer.hh"
#include "HistogramManager.hh"
#include "TArtTOGAXSIParameters.hh"
#include "TArtCalibTOGAXSISi.hh"
#include "TArtRecoTOGAXSIVertex.hh"
#include "TArtTOGAXSIVertex.hh"
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
  fRecoTOGAXSIVertex = new TArtRecoTOGAXSIVertex;

  HistogramManager* hm = HistogramManager::GetInstance();

  fhidstrip = hm->BookTH2("TOGAXSISi_idstrip","TOGAXSI Si ID stripID;ID;stripID",
			  20,0.5,20.5,200,0,1200, "TOGAXSI");

  fhidadc = hm->BookTH2("TOGAXSISi_idadc","TOGAXSI Si ID ADC-8192;ID;ADC-8192",
			  20,0.5,20.5,100,0,2000, "TOGAXSI");

  fhvxy   = hm->BookTH2("TOGAXSI_vxy","TOGAXSI Vertex XY;X;Y",
			200,-50,50, 200,-50,50, "TOGAXSI");
  fhvzx   = hm->BookTH2("TOGAXSI_vzx","TOGAXSI Vertex ZX;Z;X",
			200,-200,200, 200,-50,50, "TOGAXSI");
  fhvzy   = hm->BookTH2("TOGAXSI_vzy","TOGAXSI Vertex ZY;Z;Y",
			200,-200,200, 200,-50,50, "TOGAXSI");
  fhentvy = hm->BookTH2("TOGAXSI_entvy","TOGAXSI Entries VY;Entries;Y",
			200,0,1.E+6, 200,-50,50, "TOGAXSI");
  
  return true;
}
//--------------------------------------------------------
void TOGAXSISiAnalyzer::ReconstructData() {
  fCalibTOGAXSISi->ReconstructData();
  fRecoTOGAXSIVertex->ReconstructData();
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
      Double_t adc = si->GetADC(j) - 8192;
      fhidstrip->Fill(id,stripid);
      fhidadc->Fill(id,adc);
    }
  }

  TClonesArray* vertex_array = (TClonesArray*)fRecoTOGAXSIVertex->GetTOGAXSIVertexArray();
  Int_t nv = vertex_array->GetEntries();
//  std::cout<<"TOGAXSISI: nv="<<nv<<std::endl;
//  std::cout<<"TOGAXSISI: entries="<<&fEntries<<std::endl;
  
  for (int iv=0;iv<nv;++iv){
    TArtTOGAXSIVertex *vertex = (TArtTOGAXSIVertex*)vertex_array->At(iv);
    //Int_t ntr = vertex->GetNumOfTracks();
    Int_t seg0 = vertex->GetSegment(0);
    Int_t seg1 = vertex->GetSegment(1);
    TVector3 pos = vertex->GetPosition();

    if (seg0<5 && seg1>4){// Recoil/Cluster
      fhvxy->Fill(pos.x(), pos.y());
      fhvzx->Fill(pos.z(), pos.x());
      fhvzy->Fill(pos.z(), pos.y());
      if (fEntriesPtr) fhentvy->Fill(*fEntriesPtr,pos.y());
    } else if (seg0>4 && seg1<5){// Cluster/Recoil
      fhvxy->Fill(pos.x(), pos.y());
      fhvzx->Fill(pos.z(), pos.x());
      fhvzy->Fill(pos.z(), pos.y());
      if (fEntriesPtr) fhentvy->Fill(*fEntriesPtr,pos.y());
    }
  }

  

}
//--------------------------------------------------------
void TOGAXSISiAnalyzer::ClearData() {
  fCalibTOGAXSISi->ClearData();
  fRecoTOGAXSIVertex->ClearData();
}
//--------------------------------------------------------
//--------------------------------------------------------
