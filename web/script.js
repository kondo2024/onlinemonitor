import { openFile, redraw, httpRequest } from './jsroot/modules/main.mjs';

let settings = {
    interval: parseInt(localStorage.getItem('sm_interval')) || 3000,
    columns: parseInt(localStorage.getItem('sm_columns')) || 3,
    histsPerPage: 9,
    currentPage: 0
};

let allHistogramPaths = []; 
let activeHists = [];      
let updateTimer = null;
let hp = null; // サーバーオブジェクトをグローバルで保持

async function init() {
//    try {
//	console.log(" init() start ...");
//        // 1. サーバーに接続 (末尾のスラッシュが重要)
//        hp = await openFile("http://localhost:8080");
//	console.log("hp:",hp);
//        if (!hp) throw new Error("Could not connect to THttpServer");
//
////        const hierarchy = await httpRequest("http://localhost:8080/h.json","object");
////	if (!hierarchy) {
////            console.error("Could not fetch h.json. Check CORS settings or ROOT server.");
////            return;
////        }
////
////        console.log("Hierarchy success:", hierarchy);
//
//	let hierarchy = null;
//        if (typeof hp.getHierarchy === 'function') {
//	    console.log("Fetching hierarchy by hp.getHierarchy");
//            hierarchy = await hp.getHierarchy();
//        } else if (hp._childs) {
//            // THttpServerの場合、hpオブジェクト自体がルートノードになっていることが多い
//            hierarchy = hp;
//        } else {
//            // それでもダメなら、サーバーのツリー構造を直接取得しにいく
//	    console.log("Fetching hierarchy from h.json ...");
//            hierarchy = await httpRequest("http://localhost:8080/h.json", "object");
//        }
//
//	if (!hierarchy){
//	    throw new Error("Could not obtain hierarchy");
//	}
//
//	console.log("Hierarchy obtained:",hierarchy);
//	
//        // 2. ヒストグラムを検索
//        allHistogramPaths = []; // 初期化
//        findHistograms(hierarchy);
//        
//        // とりあえず全ヒストグラムを表示対象にする（運用に合わせて変更してください）
//        activeHists = [...allHistogramPaths];
//
//        // 3. イベント登録
//        document.getElementById('drawBtn').addEventListener('click', manualDraw);
//        // 他のUI要素（自動更新チェックボックスなど）があればここで登録
//	const resetBtn = document.getElementById('resetBtn');
//        if (resetBtn) {
//            resetBtn.addEventListener('click', resetHistograms);
//	}	
//        // 初回描画
//        await drawGrid();
//        
//        // 監視スタート
//        startMonitoring();
//
//    } catch (err) {
//        console.error("JSROOT Connection Error:", err);
//    }

    try {
        console.log("Direct access to h.json...");

        //hp = await openFile("http://localhost:8080/Objects/root.json");
	
        // 1. サーバーの階層情報を直接取得
	const hierarchy = await httpRequest("http://localhost:8080/h.json", "object");
        
        if (!hierarchy) {
            throw new Error("h.json にアクセスできません。CORSかネットワークを確認してください。");
        }

        // 2. パスを解析
        allHistogramPaths = [];
        findHistograms(hierarchy);
        console.log("Found Paths:", allHistogramPaths);

	const btn = document.getElementById('drawBtn');
        if (btn) {
            btn.onclick = async () => {
                console.log("Manual draw triggered.");
                btn.disabled = true; // 連打防止
                btn.innerText = "描画中...";
                
                await drawGrid(); // マニュアルで再描画実行
                
                btn.disabled = false;
                btn.innerText = "🔄 マニュアル描画（最新取得）";
            };
        }
	
        // 3. 描画実行
        await drawGrid();

    } catch (err) {
        console.error("Custom Init Error:", err);
    }

}
function findHistograms(node, path) {
    if (!node) return;

    // 初回呼び出し（pathが未定義）の場合は空文字をセット
    if (path === undefined) path = "";

    const name = node._name || "";
    const kind = node._kind || "";

    // パスを組み立てる（"ROOT" はスキップ）
    let nextPath = path;
    if (name && name !== "ROOT") {
        nextPath = path ? (path + "/" + name) : name;
    }

    // ヒストグラム判定
    if (kind.indexOf("TH") !== -1 || kind.indexOf("TProfile") !== -1) {
        console.log("✅ Found Hist Path:", nextPath);
        allHistogramPaths.push(nextPath);
    }

    // 子要素を探索
    if (node._childs && Array.isArray(node._childs)) {
        for (let i = 0; i < node._childs.length; i++) {
            // 親のパス (nextPath) を子に渡す
            findHistograms(node._childs[i], nextPath);
        }
    }
}
//// 描画関数を async に修正
//async function drawGrid() {
//    const container = document.getElementById('grid-container');
//    if (!container) return;
//
//    container.innerHTML = ''; 
//    container.style.gridTemplateColumns = `repeat(${settings.columns}, 1fr)`;
//
//    const start = settings.currentPage * settings.histsPerPage;
//    //const pageHists = activeHists.slice(start, start + settings.histsPerPage);
//    const pageHists = allHistogramPaths.slice(start, start + settings.histsPerPage);
//
//    // Promise.all を使うため、map の中で async/await を使用
//    const drawPromises = pageHists.map(async (path, i) => {
//        const divId = `draw_div_${i}`;
//        const wrapper = document.createElement('div');
//        wrapper.className = 'grid-wrapper';
//        wrapper.innerHTML = `<div class="hist-label">${path}</div><div id="${divId}" class="grid-item" style="height:300px;"></div>`;
//        container.appendChild(wrapper);
//
//        try {
//            // サーバーオブジェクト(hp)からオブジェクトを取得
//            const obj = await hp.readObject(path);
//            return redraw(divId, obj, "colz");
//        } catch (e) {
//            console.error(`Failed to draw ${path}:`, e);
//        }
//    });
//
//    await Promise.all(drawPromises);
//}

async function drawGrid() {
    const container = document.getElementById('grid-container');
    if (!container) return;

    container.innerHTML = '';
    container.style.gridTemplateColumns = `repeat(${settings.columns}, 1fr)`;

    const drawPromises = allHistogramPaths.map(async (path, i) => {
        const divId = `draw_div_${i}`;
        const wrapper = document.createElement('div');
        wrapper.className = 'grid-wrapper';
        wrapper.innerHTML = `<div class="hist-label">${path}</div><div id="${divId}" class="grid-item" style="height:300px; border:1px solid #ccc;"></div>`;
        container.appendChild(wrapper);

        try {
            // 重要：readObjectを使わず、直接 json.gz を取得して描画する
            const objUrl = `http://localhost:8080/${path}/root.json.gz`;
            const obj = await httpRequest(objUrl, "object");
            if (obj) {
                return redraw(divId, obj, "colz");
            }
        } catch (e) {
            console.error(`Failed to load ${path}:`, e);
        }
    });

    await Promise.all(drawPromises);
    console.log("drawGrid() done");
    
}


//function manualDraw() {
//    const btn = document.getElementById('drawBtn');
//    btn.disabled = true;
//    btn.innerText = "Drawing...";
//
//    drawGrid().then(() => {
//        setTimeout(() => {
//            btn.disabled = false;
//            btn.innerText = "Draw (Manual Refresh)";
//        }, 500);
//    });
//    console.log("manualDraw() done");
//}

function startMonitoring() {
    if (updateTimer) clearInterval(updateTimer);

    updateTimer = setInterval(() => {
        const autoUpdate = document.getElementById('autoUpdate')?.checked;
        const autoPage = document.getElementById('autoPage')?.checked;

        if (autoUpdate) {
            drawGrid();
        }

        if (autoPage) {
            const maxPage = Math.ceil(activeHists.length / settings.histsPerPage) - 1;
            settings.currentPage = (settings.currentPage >= maxPage) ? 0 : settings.currentPage + 1;
        }
    }, settings.interval);
}

// サーバーコマンド送信
async function resetHistograms() {
    if (confirm("本当にリセットしますか？")) {
        // http://localhost:8080/ResetAll/cmd.json にリクエストを送る例
        await httpRequest("ResetAll/cmd.json", "text");
    }
}

// 実行
init();

