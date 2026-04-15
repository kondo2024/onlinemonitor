import { openFile, redraw, httpRequest } from './jsroot/modules/main.mjs';

let settings = {
    interval: parseInt(localStorage.getItem('sm_interval')) || 8000,
    columns: parseInt(localStorage.getItem('sm_columns')) || 3,
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

//function updateSidebar() {
//    const listContainer = document.getElementById('hist-list');
//    if (!listContainer) return;
//
//    listContainer.innerHTML = '';
//    allHistogramPaths.forEach((path, i) => {
//        const li = document.createElement('li');
//        li.style.padding = "5px 10px";
//        li.style.borderBottom = "1px solid #eee";
//        li.style.fontSize = "13px";
//        li.style.cursor = "pointer";
//        
//        li.onclick = () => {
//            const target = document.getElementById(`wrapper_div_${i}`);
//            if (target) target.scrollIntoView({ behavior: 'smooth' });
//        };
//
//        li.onmouseover = () => li.style.backgroundColor = "#f0f0f0";
//        li.onmouseout = () => li.style.backgroundColor = "transparent";
//        
//        li.innerText = path;
//        listContainer.appendChild(li);
//    });
//}

function updateSidebar() {
    const listContainer = document.getElementById('hist-list');
    if (!listContainer) return;

    listContainer.innerHTML = '';
    allHistogramPaths.forEach((path, i) => {
        const li = document.createElement('li');
        li.style.padding = "5px 10px";
        li.style.display = "flex";
        li.style.alignItems = "center";
        li.style.borderBottom = "1px solid #eee";

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

        // ラベル（クリックでスクロール）
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
//async function drawGrid() {
//    const container = document.getElementById('grid-container');
//    if (!container) return;
//
//    container.innerHTML = '';
//    container.style.display = 'grid';
//    container.style.gridTemplateColumns = `repeat(${settings.columns}, 1fr)`;
//
////    const drawPromises = allHistogramPaths.map(async (path, i) => {
//    const drawPromises = activeHists.map(async (path, i) => {
//        const divId = `draw_div_${i}`;
//        const wrapper = document.createElement('div');
//        wrapper.className = 'grid-wrapper';
//        wrapper.innerHTML = `<div class="hist-label">${path}</div><div id="${divId}" class="grid-item" style="height:300px; border:1px solid #ccc;"></div>`;
//        container.appendChild(wrapper);
//
//        try {
//            const objUrl = `http://localhost:8080/${path}/root.json.gz`;
//            const obj = await httpRequest(objUrl, "object");
//            if (obj) {
//                return redraw(divId, obj, "colz");
//            }
//        } catch (e) {
//            console.error(`Failed to load ${path}:`, e);
//        }
//    });
//
//    await Promise.all(drawPromises);
//    console.log("drawGrid() done");
//    
//}
//
//function startMonitoring() {
//    if (updateTimer) clearInterval(updateTimer);
//
//    updateTimer = setInterval(() => {
//        const autoUpdate = document.getElementById('autoUpdate')?.checked;
//        const autoPage = document.getElementById('autoPage')?.checked;
//
//        if (autoUpdate) {
//            drawGrid();
//        }
//
//        if (autoPage) {
//            const maxPage = Math.ceil(activeHists.length / settings.histsPerPage) - 1;
//            settings.currentPage = (settings.currentPage >= maxPage) ? 0 : settings.currentPage + 1;
//        }
//    }, settings.interval);
//}


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
    container.style.gridTemplateColumns = `repeat(${settings.columns}, 1fr)`;

    const drawPromises = pageItems.map(async (path, i) => {
        const divId = `draw_div_${i}`;
        const wrapper = document.createElement('div');
        wrapper.className = 'grid-wrapper';
	wrapper.style.marginBottom = "20px";
        wrapper.innerHTML = `
            <div id="${divId}" style="height:280px; margin:0 2px;"></div>
        `;
//        wrapper.innerHTML = `
//            <div style="text-align:center; font-size:12px; font-weight:bold; margin-bottom:5px;">${path}</div>
//            <div id="${divId}" style="height:280px; border:1px solid #ccc; margin:0 5px;"></div>
//        `;

//	wrapper.innerHTML = `
//            <div style="text-align:center; font-size:11px;">[Page ${settings.currentPage+1}] ${path}</div>
//            <div id="${divId}" style="height:250px; border:1px solid #ccc; margin:5px;"></div>
//        `;
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
        const autoPage = document.getElementById('autoPage')?.checked;

        if (autoPage) {
	    const totalHists = activeHists.length;
            const maxPage = Math.max(0, Math.ceil(activeHists.length / settings.histsPerPage) - 1);
            settings.currentPage = (settings.currentPage >= maxPage) ? 0 : settings.currentPage + 1;
            drawGrid();
        }else if (autoUpdate){
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

