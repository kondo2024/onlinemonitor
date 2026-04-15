import { openFile, redraw, httpRequest } from './jsroot/modules/main.mjs';

let settings = {
    interval: parseInt(localStorage.getItem('sm_interval')) || 8000,
    rows: 3,
    columns: 3,
    histsPerPage: 9,
    currentPage: 0
};

let allHistogramPaths = []; 
let activeHists = [];      
let updateTimer = null;
//let hp = null;

async function init() {
    try {
        console.log("Direct access to h.json...");
	const hierarchy = await httpRequest("http://localhost:8080/h.json", "object");
        
        if (!hierarchy) {
            throw new Error("cannot access to h.json");
        }

        allHistogramPaths = [];
        findHistograms(hierarchy);
        console.log("Found Paths:", allHistogramPaths);

	activeHists = [...allHistogramPaths];

	setupLayoutControls();
	
	// list for side bar
	updateSidebar();
	
	const btn = document.getElementById('drawBtn');
        if (btn) {
            btn.onclick = async () => {
                console.log("Manual draw triggered.");
                btn.disabled = true;
                btn.innerText = "Drawing...";
                
                await drawGrid();
                
                btn.disabled = false;
                btn.innerText = "Update manually";
            };
        }
	
        await drawGrid();

	startMonitoring();// start timer
	
    } catch (err) {
        console.error("Custom Init Error:", err);
    }
}

function setupLayoutControls() {
    const ir = document.getElementById('inputRows');
    const ic = document.getElementById('inputCols');

    const updateLayout = () => {
        settings.rows = parseInt(ir.value) || 1;
        settings.columns = parseInt(ic.value) || 1;
        settings.histsPerPage = settings.rows * settings.columns;
        settings.currentPage = 0;
        drawGrid();
    };

    ir.addEventListener('change', updateLayout);
    ic.addEventListener('change', updateLayout);
}


function updateSidebar() {
    const listContainer = document.getElementById('hist-list');
    if (!listContainer) return;

    listContainer.innerHTML = '';
    allHistogramPaths.forEach((path, i) => {
        const li = document.createElement('li');
        li.style.padding = "5px 10px";
        li.style.display = "flex";
        li.style.alignItems = "center";

        // チェックボックス
        const cb = document.createElement('input');
        cb.type = 'checkbox';
        cb.id = `cb_${i}`;
        cb.checked = activeHists.includes(path);
        cb.style.marginRight = "10px";
        
        cb.onchange = () => {
            if (cb.checked) {
                if (!activeHists.includes(path)) activeHists.push(path);
            } else {
                activeHists = activeHists.filter(p => p !== path);
            }
        };

        const label = document.createElement('label');
        label.innerText = path;
        label.style.cursor = "pointer";
        label.style.fontSize = "13px";
        label.style.flexGrow = "1";
        label.onclick = () => {
            const target = document.getElementById(`wrapper_div_${path}`);
            if (target) target.scrollIntoView({ behavior: 'smooth' });
        };

        li.appendChild(cb);
        li.appendChild(label);
        listContainer.appendChild(li);
    });
}

function findHistograms(node, path) {
    if (!node) return;

    if (path === undefined) path = "";

    const name = node._name || "";
    const kind = node._kind || "";

    let nextPath = path;
    if (name && name !== "ROOT") {
        nextPath = path ? (path + "/" + name) : name;
    }

    if (kind.indexOf("TH") !== -1 || kind.indexOf("TProfile") !== -1) {
        console.log("Found Hist Path:", nextPath);
        allHistogramPaths.push(nextPath);
    }

    if (node._childs && Array.isArray(node._childs)) {
        for (let i = 0; i < node._childs.length; i++) {
            findHistograms(node._childs[i], nextPath);
        }
    }
}

async function drawGrid() {
    const pageInfo = document.getElementById('page-info');
    const container = document.getElementById('grid-container');
    if (!container) return;

    // page number
    const totalHists = activeHists.length;
    const maxPage = Math.max(1, Math.ceil(totalHists / settings.histsPerPage));
    if (settings.currentPage >= maxPage && maxPage > 0) {
        settings.currentPage = maxPage - 1;
    }
    if (pageInfo) {
        pageInfo.innerText = `(Page ${settings.currentPage + 1}/${maxPage})`;
    }
    const start = settings.currentPage * settings.histsPerPage;
    const end = start + settings.histsPerPage;
    const pageItems = activeHists.slice(start, end);

    container.innerHTML = '';
    container.style.display = 'grid';
    container.style.width = '100%';
    container.style.gridTemplateColumns = `repeat(${settings.columns}, 1fr)`;
    container.style.gridTemplateRows = `repeat(${settings.rows}, 1fr)`; // 行数もJSで指定
    container.style.height = "calc(100vh - 100px)"; // コントロールパネル分を引いた高さ

    const drawPromises = pageItems.map(async (path, i) => {
        const divId = `draw_div_${i}`;
        const wrapper = document.createElement('div');
        wrapper.className = 'grid-wrapper';
	const dynamicHeight = Math.max(200, Math.floor(800 / settings.rows));
        wrapper.innerHTML = `
            <div id="${divId}"  class="grid-item"></div>
        `;
        container.appendChild(wrapper);

        try {
            const obj = await httpRequest(`http://localhost:8080/${path}/root.json.gz`, "object");
            if (obj) await redraw(divId, obj, "colz");
        } catch (e) {
            console.error(e);
        }
    });

    await Promise.all(drawPromises);
}

function startMonitoring() {
    if (updateTimer) clearInterval(updateTimer);

    updateTimer = setInterval(() => {
        const autoUpdate = document.getElementById('autoUpdate')?.checked;

        if (autoUpdate) {
	    const totalHists = activeHists.length;
            const maxPage = Math.max(0, Math.ceil(activeHists.length / settings.histsPerPage) - 1);
            settings.currentPage = (settings.currentPage >= maxPage) ? 0 : settings.currentPage + 1;
            drawGrid();
	}
    }, settings.interval);
}

async function resetHistograms() {
    if (confirm("really want to reset?")) {
        await httpRequest("ResetAll/cmd.json", "text");
    }
}

init();

