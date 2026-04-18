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
let isRunning = false;
let lastSuccessTime = Date.now();

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

	setupControls();
	updateSidebar();
	
        await drawGrid();

	startMonitoring();// start timer
	
    } catch (err) {
        console.error("Custom Init Error:", err);
    }
}

function setupControls() {
    const runBtn = document.getElementById('runBtn');
    const pauseBtn = document.getElementById('pauseBtn');
    const resetBtn = document.getElementById('resetBtn');
    const ir = document.getElementById('inputRows');
    const ic = document.getElementById('inputCols');
    const ip = document.getElementById('inputPage');
    const statusInfo = document.getElementById('status-info');
    const editControls = [ir, ic, ip, resetBtn];

    const toggleUI = () => {
        isRunning = !isRunning;

        editControls.forEach(el =>{
	    el.disabled = isRunning;
	    if (isRunning){
		el.style.cursor = "not-allowed";
	    }else{
		el.style.cursor = "auto";
	    }
	});
        
        if (isRunning) {
	    runBtn.textContent = 'Pause';
	    startMonitoring();
	    if (statusInfo) statusInfo.innerText = `Auto Update ON`;

        } else {
	    runBtn.textContent = 'Auto Update ON';
	    if (statusInfo) statusInfo.innerText = `Paused`;
            if (updateTimer) clearInterval(updateTimer);
        }
    };

    runBtn.onclick = () => toggleUI();

    const updateLayout = () => {
        settings.rows = parseInt(ir.value) || 1;
        settings.columns = parseInt(ic.value) || 1;
        settings.histsPerPage = settings.rows * settings.columns;
        settings.currentPage = 0;
        ip.value = 1;
        drawGrid();
    };

    ir.onchange = updateLayout;
    ic.onchange = updateLayout;

    ip.onchange = () => {
        const targetPage = (parseInt(ip.value) || 1) - 1;
        const maxPageIdx = Math.max(0, Math.ceil(activeHists.length / settings.histsPerPage) - 1);
        
        settings.currentPage = Math.min(Math.max(0, targetPage), maxPageIdx);
        ip.value = settings.currentPage + 1;
        drawGrid();
    };

    
    if (resetBtn) {
        resetBtn.onclick = async () => {
            const confirmed = confirm("Reset all histogram statistics?");
            if (confirmed) {
                try {
                    resetBtn.disabled = true;
                    resetBtn.innerText = "Resetting...";
                    await httpRequest("/ResetAll/cmd.json", "text");
                    console.log("Histogram reset successful.");
                    await drawGrid();

                } catch (err) {
                    console.error("Reset Error:", err);
                    alert("failed to reset histograms");
                } finally {
                    resetBtn.disabled = false;
                    resetBtn.innerText = "Reset events";
                }
            }
        };
    }

    
    toggleUI();
}


function updateSidebar() {
    const listContainer = document.getElementById('hist-list');
    if (!listContainer) return;

    listContainer.innerHTML = '';
    allHistogramPaths.forEach((path, i) => {
        const li = document.createElement('li');
        li.style.padding = "0px 10px";
        li.style.display = "flex";
        li.style.alignItems = "left";

	//check boxes
        const cb = document.createElement('input');
        cb.type = 'checkbox';
        cb.id = `cb_${i}`;
        cb.checked = activeHists.includes(path);
        cb.style.marginRight = "10px";
        
        cb.onchange = () => {
	    const checkedPaths = allHistogramPaths.filter((p, index) => {
                const checkbox = document.getElementById(`cb_${index}`);
                return checkbox ? checkbox.checked : activeHists.includes(p);
            });
	    activeHists = checkedPaths;
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

async function updateTimeDisplay() {
    const warningElement = document.getElementById('process-warning');
    const timeDisplay = document.getElementById('server-date-time');
    const statusInfo = document.getElementById('status-info');
    
    try {
        const res = await httpRequest("/Status/ServerTime/root.json", "object");
        
        if (res && res.fTitle) {
            timeDisplay.textContent = res.fTitle;
            lastSuccessTime = Date.now();
            warningElement.style.display = "none";
        }
    } catch (err) {
        console.error("Error at getting server date/time:", err);
        
        if (isRunning) {
            const idleSeconds = (Date.now() - lastSuccessTime) / 1000;
            console.error(idleSeconds);
            
            if (idleSeconds > 5) {
                warningElement.style.display = "block";
                timeDisplay.textContent = "Disconnected";
		isRunning = false;
            }
        }
    }
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
        //console.log("Found Hist Path:", nextPath);
        allHistogramPaths.push(nextPath);
    }

    if (node._childs && Array.isArray(node._childs)) {
        for (let i = 0; i < node._childs.length; i++) {
            findHistograms(node._childs[i], nextPath);
        }
    }
}

async function drawGrid() {
    await updateTimeDisplay();

    const pageInfo = document.getElementById('page-info');
    const container = document.getElementById('grid-container');
    const ip = document.getElementById('inputPage');
    if (!container) return;

    // page number
    const totalHists = activeHists.length;
    const maxPage = Math.max(1, Math.ceil(totalHists / settings.histsPerPage));
    if (pageInfo) {
        pageInfo.innerText = `(Page ${settings.currentPage + 1}/${maxPage})`;
    }
    if (ip&&!isRunning) ip.max = maxPage;
    
    const start = settings.currentPage * settings.histsPerPage;
    const end = start + settings.histsPerPage;
    const pageItems = activeHists.slice(start, end);

    container.innerHTML = '';
    container.style.display = 'grid';
    container.style.width = '100%';
    container.style.gridTemplateColumns = `repeat(${settings.columns}, 1fr)`;
    container.style.gridTemplateRows = `repeat(${settings.rows}, 1fr)`;
    container.style.height = "calc(100vh - 100px)";

    const drawPromises = pageItems.map(async (path, i) => {
        const divId = `draw_div_${i}`;
        const wrapper = document.createElement('div');
        wrapper.className = 'grid-wrapper';
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

	const totalHists = activeHists.length;
        const maxPage = Math.max(0, Math.ceil(activeHists.length / settings.histsPerPage) - 1);
        settings.currentPage = (settings.currentPage >= maxPage) ? 0 : settings.currentPage + 1;
	document.getElementById('inputPage').value = settings.currentPage + 1;
	updateTimeDisplay();
        drawGrid();
    }, settings.interval);
}

async function resetHistograms() {
    if (confirm("really want to reset?")) {
        await httpRequest("ResetAll/cmd.json", "text");
    }
}

init();

