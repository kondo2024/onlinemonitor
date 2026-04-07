#ifndef BaseAnalyzer_hh
#define BaseAnalyzer_hh

#include <string>

class TArtEventStore;
class HistogramManager;

class BaseAnalyzer {
public:
    BaseAnalyzer(const std::string& name) : fName(name) {}
    virtual ~BaseAnalyzer() {}

    virtual bool Init(HistogramManager* hm) = 0;
    virtual bool Process(TArtEventStore* estore) = 0;
    virtual void Reset() {}

    std::string GetName() const { return fName; }

protected:
    std::string fName;
};

#endif
