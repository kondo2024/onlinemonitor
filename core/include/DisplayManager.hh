#ifndef DisplayManager_hh
#define DisplayManager_hh

#include <string>
#include <vector>
#include <map>

class DisplayManager {
public:
    DisplayManager();
    virtual ~DisplayManager();

    // config.json 等から表示レイアウトを初期化
    bool Init(const std::string& configPath);

    // ページごとのヒストグラムリストを取得（JSROOT側へ渡す用）
    std::vector<std::string> GetPageHistograms(int pageIndex);

    // 全体のページ数を取得
    int GetTotalPages() const { return fTotalPages; }

    // JSROOTで使いやすい形式（JSON等）でレイアウト情報を出力
    std::string GetLayoutJSON();

private:
    struct PageConfig {
        std::vector<std::string> histogramNames;
    };

    std::vector<PageConfig> fPages;
    int fTotalPages;
    int fHistsPerPage; // 1ページあたりの表示数（デフォルト9）
};

#endif
