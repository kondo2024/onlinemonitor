// ユーザー設定の保持 (localStorage)
let settings = {
    interval: parseInt(localStorage.getItem('sm_interval')) || 3000,
    histsPerPage: 9,
    currentPage: 0
};

let allHistogramPaths = []; // サーバーから取得する全リスト
let activeHists = [];      // チェックが入っているリスト
let updateTimer = null;

// 1. 初期化
JSROOT.connectHttp().then(hp => {
    // サーバー上の全オブジェクトをスキャン
    hp.getHierarchy().then(res => {
        findHistograms(res);
        createUI();
        startMonitoring();
    });
});

function findHistograms(node) {
    if (node._kind && node._kind.startsWith("TH1")) {
        allHistogramPaths.push(node._full_name);
    }
    if (node._childs) {
        node._childs.forEach(child => findHistograms(child));
    }
}

// 2. グリッドの描画
function drawGrid() {
    const container = document.getElementById('grid-container');
    container.innerHTML = ''; // クリア
    
    // 現在のページに表示するヒストグラムを抽出
    const start = settings.currentPage * settings.histsPerPage;
    const pageHists = activeHists.slice(start, start + settings.histsPerPage);

    pageHists.forEach((path, i) => {
        const divId = `draw_div_${i}`;
        const div = document.createElement('div');
        div.id = divId;
        div.className = 'grid-item';
        container.appendChild(div);

        // JSROOT で描画 (THttpServerからデータを取得)
        JSROOT.httpRequest(path + "/root.json.gz", "object").then(obj => {
            // ユーザーがブラウザ上でズーム等を変えても上書きされないよう描画
            JSROOT.redraw(divId, obj, "colz");
        });
    });
    
    document.getElementById('currentPage').innerText = settings.currentPage + 1;
    document.getElementById('totalPages').innerText = Math.ceil(activeHists.length / settings.histsPerPage);
}

// 3. 自動更新 & ページ切り替え
function startMonitoring() {
    if (updateTimer) clearInterval(updateTimer);
    
    updateTimer = setInterval(() => {
        if (document.getElementById('autoUpdate').checked) {
            drawGrid();
        }
        
        if (document.getElementById('autoPage').checked) {
            const maxPage = Math.ceil(activeHists.length / settings.histsPerPage) - 1;
            settings.currentPage = (settings.currentPage >= maxPage) ? 0 : settings.currentPage + 1;
        }
    }, settings.interval);
}

// 4. サーバー側へのコマンド送信 (Reset)
function resetHistograms() {
    if (confirm("本当にリセットしますか？")) {
        JSROOT.httpRequest("ResetAll/cmd.json", "text");
    }
}

// UI生成やイベントリスナーなどは省略...
