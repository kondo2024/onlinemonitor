import { redraw, httpRequest } from './jsroot/modules/main.mjs';

//---------------------------------------------------
const state = {
    settings: {
	httpPort: 8080,
        interval: parseInt(localStorage.getItem('sm_interval')) || 8000,
        rows: 3,
        columns: 3,
        get histsPerPage() { return this.rows * this.columns; }
    },
    autoReset: true,
    autoResetEvents: 100000,
    isRunning: false,
    currentPage: 0,
    lastSuccessTime: Date.now(),
    defaultSkipPaths: [],
    allPaths: [],
    activePaths: [],
    lastTotalEntries: 0,
    lastEntryChangeTime: Date.now(),
    updateTimer: null,
    isBusyChecking: false
};

//---------------------------------------------------
async function init() {
    try {

	const config = await httpRequest("config/config.json", "object");

	if (config?.http_port) state.settings.httpPort = parseInt(config.http_port);
	const port = state.settings.httpPort;
	
	if (config?.display?.update_interval_ms) {
            const configInterval = parseInt(config.display.update_interval_ms);
            if (!localStorage.getItem('sm_interval')) state.settings.interval = configInterval;
        }

	state.autoReset = config?.auto_reset || false;
        state.autoResetEvents = config?.auto_reset_events || 100000;
	
	if (config?.display?.default_rows) state.settings.rows = parseInt(config.display.default_rows);
	if (config?.display?.default_columns) state.settings.columns = parseInt(config.display.default_columns);
	
        state.defaultSkipPaths = config?.skip_histograms || [];
	//const hierarchy = await httpRequest("http://localhost:8080/h.json", "object");
	const hierarchy = await httpRequest(`http://localhost:${port}/h.json`, "object");
        if (!hierarchy) throw new Error("Cannot access h.json");

        state.allPaths = [];
        findHistograms(hierarchy);
	state.activePaths = state.allPaths.filter(path => 
            !state.defaultSkipPaths.includes(path)
        );
 
        setupEventListeners();
        updateHistList();
        updateUIStates();
        
        await drawGrid();
    } catch (err) {
        console.error("Initialization Failed:", err);
        showDisconnected(true);
    }
}
//---------------------------------------------------
async function checkBusyStatus() {
    try {
	const port = state.settings.httpPort;
        const res = await httpRequest(`http://localhost:${port}/Status/IsAnalysisBusy/root.json.gz`, "object");
        return res && res.fValue === 1;
    } catch (e) {
        return false;
    }
}
//---------------------------------------------------
function setupEventListeners() {
    const el = (id) => document.getElementById(id);

    el('runBtn').onclick = () => {
        state.isRunning = !state.isRunning;
        if (state.isRunning) {
            startMonitoring();
        } else {
            stopMonitoring();
        }
        updateUIStates();
    };

    el('drawBtn').onclick = () => {
        updateUIStates();
        drawGrid();
    };

    
    el('inputRows').value = state.settings.rows;
    el('inputCols').value = state.settings.columns;
    
    const onLayoutChange = () => {
        state.settings.rows = parseInt(el('inputRows').value) || 1;
        state.settings.columns = parseInt(el('inputCols').value) || 1;
        state.currentPage = 0;
        el('inputPage').value = 1;
    };
    el('inputRows').onchange = onLayoutChange;
    el('inputCols').onchange = onLayoutChange;

    el('inputPage').onchange = (e) => {
        const maxPage = Math.ceil(state.activePaths.length / state.settings.histsPerPage);
        state.currentPage = Math.min(Math.max(0, (parseInt(e.target.value) || 1) - 1), maxPage - 1);
        e.target.value = state.currentPage + 1;
    };

    el('inputInterval').value = state.settings.interval / 1000; // ms -> sec
    el('inputInterval').onchange = (e) => {
        const newSec = parseFloat(e.target.value) || 5;
        state.settings.interval = newSec * 1000;
        localStorage.setItem('sm_interval', state.settings.interval);
    };
    
    el('resetBtn').onclick = async () => {
        if (!confirm("Reset all histograms?")) return;
        try {
            el('resetBtn').disabled = true;
            await httpRequest("/ResetAll/cmd.json", "text");
	    state.lastTotalEntries = 0;
	    state.lastEntryChangeTime = Date.now();
            await drawGrid();
        } catch (err) {
            alert("Failed to reset histograms");
        } finally {
            el('resetBtn').disabled = false;
        }
    };
}

//---------------------------------------------------
function startMonitoring() {
    stopMonitoring();
    const loop = async () => {
        if (!state.isRunning) return;
        
        const maxPage = Math.ceil(state.activePaths.length / state.settings.histsPerPage);
        state.currentPage = (state.currentPage + 1) >= maxPage ? 0 : state.currentPage + 1;
        const elPageInput = document.getElementById('inputPage');
        if (elPageInput) elPageInput.value = state.currentPage + 1;
        
        await drawGrid();
        state.updateTimer = setTimeout(loop, state.settings.interval);
    };
    state.updateTimer = setTimeout(loop, state.settings.interval);

    
//    state.updateTimer = setInterval(async () => {
//        const maxPage = Math.ceil(state.activePaths.length / state.settings.histsPerPage);
//        state.currentPage = (state.currentPage + 1) >= maxPage ? 0 : state.currentPage + 1;
//        document.getElementById('inputPage').value = state.currentPage + 1;
//        await drawGrid();
//    }, state.settings.interval);
}

//---------------------------------------------------
function stopMonitoring() {
    if (state.updateTimer) {
        clearInterval(state.updateTimer);
        state.updateTimer = null;
    }
}

//---------------------------------------------------
async function drawGrid() {

    const isBusy = await checkBusyStatus();
    if (isBusy) {
        console.log("Server is busy (Analysis in progress). Retrying in 100ms...");
        setTimeout(() => drawGrid(), 100);
        return;
    }

    await updateStatusInfo();
    
    const container = document.getElementById('grid-container');
    const elPage = document.getElementById('page-info');
    if (!container) return;

    const total = state.activePaths.length;
    const maxPage = Math.ceil(total / state.settings.histsPerPage) || 1;
    if (elPage) elPage.innerText = `(Page ${state.currentPage + 1}/${maxPage})`;

    const start = state.currentPage * state.settings.histsPerPage;
    const items = state.activePaths.slice(start, start + state.settings.histsPerPage);

    // Grid Layout Layout
    container.innerHTML = '';
    container.style.display = 'grid';
    container.style.gridTemplateColumns = `repeat(${state.settings.columns}, 1fr)`;
    container.style.gridTemplateRows = `repeat(${state.settings.rows}, 1fr)`;


    const promises = items.map(async (path, i) => {
        const divId = `draw_div_${i}`;
        const wrapper = document.createElement('div');
        wrapper.className = 'grid-wrapper';
	wrapper.style.position = 'relative';
        wrapper.innerHTML = `<div id="${divId}" class="grid-item"></div>`;
	if (state.isRunning) {
	    const cover = document.createElement('div');
	    cover.style.position = 'absolute';
	    cover.style.top = '0';
	    cover.style.left = '0';
	    cover.style.width = '100%';
	    cover.style.height = '100%';
	    cover.style.zIndex = '100'; 
	    cover.style.background = 'rgba(255, 255, 255, 0)';
	    cover.style.cursor = 'not-allowed';
	    wrapper.appendChild(cover);
	}
	
        container.appendChild(wrapper);

        try {

	    const port = state.settings.httpPort;
            //const obj = await httpRequest(`http://localhost:8080/${path}/root.json.gz`, "object");
            const obj = await httpRequest(`http://localhost:${port}/${path}/root.json.gz`, "object");
            if (obj) await redraw(divId, obj, "colz");
        } catch (e) {
            console.error(`Failed to draw ${path}:`, e);
        }
    });

    await Promise.all(promises);

}

//---------------------------------------------------
async function updateStatusInfo() {
    const elTime = document.getElementById('server-date-time');
    const elDataWarn = document.getElementById('data-warning');
    
    try {
        const res = await httpRequest("/Status/ServerTime/root.json", "object");
        if (res?.fTitle) {
            elTime.textContent = res.fTitle;
            state.lastSuccessTime = Date.now();
            showDisconnected(false);
        }

	if (state.allPaths.length > 0) {
	    const port = state.settings.httpPort;
            const headPath = state.allPaths[0];
            const obj = await httpRequest(`http://localhost:${port}/Status/Entries/root.json.gz`, "object");
	    
            if (obj && obj.fValue !== undefined) {
                const currentEntries = obj.fValue;

                if (currentEntries > state.lastTotalEntries) {
                    state.lastTotalEntries = currentEntries;
                    state.lastEntryChangeTime = Date.now();
                    elDataWarn.style.display = "none";
                } else {
                    const idleSec = (Date.now() - state.lastEntryChangeTime) / 1000;
                    if (state.isRunning && idleSec >= 10) {
                        elDataWarn.style.display = "block";
                    }
                }
		// auto reset
		if (state.autoReset && currentEntries >= state.autoResetEvents) {
                    console.log(`Auto Reset triggered: ${currentEntries} entries reached.`);
                    await performAutoReset();
		}
            }
        }	
    } catch (err) {
        const idleSec = (Date.now() - state.lastSuccessTime) / 1000;
        if (idleSec > 5) {
            showDisconnected(true);
            state.isRunning = false;
            //stopMonitoring();
            updateUIStates();
        }
    }
}

//---------------------------------------------------
function updateUIStates() {
    const el = (id) => document.getElementById(id);
    const runBtn = el('runBtn');
    const status = el('status-info');
    const autoReset = el('auto-reset-info');
    const inputs = [el('inputRows'), el('inputCols'), el('inputPage'), el('inputInterval'), el('drawBtn'), el('resetBtn')];

    runBtn.innerHTML = state.isRunning ? 'Auto Update<br>ON => OFF' : 'Auto Update<br>OFF => ON';
    if (status) status.innerText = state.isRunning ? 'Auto Update ON' : 'Auto Update OFF';

    if (autoReset) {
	autoReset.innerText = state.autoReset ? 'Auto Reset ON' : 'Auto Reset OFF';
	if (state.autoReset) autoReset.innerText += "\n(" + state.autoResetEvents +" events)";
    }
    
    inputs.forEach(input => {
        if (!input) return;
        input.disabled = state.isRunning;
        input.style.cursor = state.isRunning ? "not-allowed" : "auto";
    });
}

//---------------------------------------------------
function showDisconnected(show) {
    const warn = document.getElementById('process-warning');
    const time = document.getElementById('server-date-time');
    if (warn) warn.style.display = show ? "block" : "none";
    if (show && time) time.textContent = "Disconnected";
}

//---------------------------------------------------
function findHistograms(node, path = "") {
    if (!node) return;
    const name = node._name || "";
    const kind = node._kind || "";
    const nextPath = (name && name !== "ROOT") ? (path ? `${path}/${name}` : name) : path;

    if (kind.includes("TH") || kind.includes("TProfile")) {
        state.allPaths.push(nextPath);
    }

    if (node._childs) {
        node._childs.forEach(child => findHistograms(child, nextPath));
    }
}

//---------------------------------------------------
function updateHistList() {
    const container = document.getElementById('hist-list');
    if (!container) return;

    container.innerHTML = '';
    state.allPaths.forEach((path, i) => {
        const li = document.createElement('li');
        li.className = 'histlist-item';
        
        const cb = document.createElement('input');
        cb.type = 'checkbox';
        cb.checked = state.activePaths.includes(path);
        
        cb.onchange = () => {
            const updatedPaths = state.allPaths.filter((p, index) => {
                const checkbox = document.querySelector(`#cb_${index}`);
                if (index === i) return cb.checked; 
                return checkbox ? checkbox.checked : state.activePaths.includes(p);
            });

            state.activePaths = updatedPaths;
            
            //drawGrid();
        };

        cb.id = `cb_${i}`;

        const label = document.createElement('label');
        label.innerText = path;
        label.setAttribute('for', `cb_${i}`);
        label.style.cursor = "pointer";

        li.appendChild(cb);
        li.appendChild(label);
        container.appendChild(li);
    });
}
//---------------------------------------------------
async function performAutoReset() {
    try {
        const originalAutoReset = state.autoReset;
        state.autoReset = false; 

        await httpRequest("/ResetAll/cmd.json", "text");
        console.log("Auto reset successful.");

        state.lastTotalEntries = 0;
        state.lastEntryChangeTime = Date.now();

        setTimeout(() => {// wait for 2000ms
            state.autoReset = originalAutoReset;
        }, 2000);

    } catch (err) {
        console.error("Auto Reset Failed:", err);
    }
}
//---------------------------------------------------
init();

//---------------------------------------------------
//---------------------------------------------------
//---------------------------------------------------
