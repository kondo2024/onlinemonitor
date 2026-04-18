#ifndef TestAnalyzer_hh
#define TestAnalyzer_hh

#include "BaseAnalyzer.hh"
#include "HistogramManager.hh"
#include <TH1.h>
#include <iostream>

class TestAnalyzer : public BaseAnalyzer {
public:
  TestAnalyzer() : BaseAnalyzer("Test") {}

  bool Init(HistogramManager* hm) override {

    for (int i=0;i<11;++i){
      hTest[i] = hm->GetTH1(Form("hTest%d",i),
			    Form("Test Histogram %d;Value;Counts",i),
			    100, 0, 100, "TestFolder");
      hTest[i]->SetMinimum(0);
      //hTest[i]->Print();
    }
    return true;
  }

  bool Process() override {
    for (int i=0;i<11;++i)
      hTest[i]->Fill(rand()%100 + i*10);

    return true;
  }

private:
  TH1* hTest[11];
};

#endif
