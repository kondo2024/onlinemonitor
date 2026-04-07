#ifndef TestAnalyzer_hh
#define TestAnalyzer_hh

#include "BaseAnalyzer.hh"
#include "HistogramManager.hh"
#include <TH1.h>

class TestAnalyzer : public BaseAnalyzer {
public:
    TestAnalyzer() : BaseAnalyzer("Test") {}

    bool Init(HistogramManager* hm) override {
        hTest = hm->GetTH1("hTest", "Test Histogram;Value;Counts", 100, 0, 100, "TestFolder");
        return true;
    }

    bool Process(TArtEventStore* estore) override {
        hTest->Fill(rand() % 100);
        return true;
    }

private:
    TH1* hTest;
};

#endif
