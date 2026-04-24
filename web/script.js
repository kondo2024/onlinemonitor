import { redraw, httpRequest } from './jsroot/modules/main.mjs';

//---------------------------------------------------
const state = {
    settings: {
        interval: parseInt(localStorage.getItem('sm_interval')) || 8000,
        rows: 3,
        columns: 3,
        get histsPerPage() { return this.rows * this.columns; }
    },
    server: {
        autoReset: false,
        autoResetEvents: 1000000,
        startTime: "", 
	skipPaths: []
    },
    isRunning: false,
    currentPage: 0,
    lastSuccessTime: Date.now(),
    allPaths: [],
    activePaths: [],
    lastEntries: 0,
    updateTimer: null
};

//---------------------------------------------------
async function init() {
    try {

        const success = await syncServerState();
	if (!success) throw new Error("Server not responding");
	
        setupEventListeners();
        updateUIStates();
        await drawGrid();
        
        console.log("Initialization done.");
    } catch (err) {
	console.error("Init failed:", err);
        showDisconnected(true);
    }
}
//---------------------------------------------------
function setupEventListeners() {
    const el = (id) => document.getElementById(id);

    el('runBtn').onclick = () => {
        state.isRunning = !state.isRunning;
        state.isRunning ? startMonitoring() : stopMonitoring();
        updateUIStates();
    };

    el('drawBtn').onclick = () => drawGrid();

    el('inputRows').value = state.settings.rows;
    el('inputCols').value = state.settings.columns;
    const onLayoutChange = () => {
        state.settings.rows = parseInt(el('inputRows').value) || 1;
        state.settings.columns = parseInt(el('inputCols').value) || 1;
        state.currentPage = 0;
    };
    el('inputRows').onchange = onLayoutChange;
    el('inputCols').onchange = onLayoutChange;

    el('inputPage').onchange = (e) => {
        const maxPage = Math.ceil(state.activePaths.length / state.settings.histsPerPage);
        state.currentPage = Math.min(Math.max(0, (parseInt(e.target.value) || 1) - 1), maxPage - 1);
        e.target.value = state.currentPage + 1;
    };

    el('inputInterval').value = state.settings.interval / 1000;
    el('inputInterval').onchange = (e) => {
        state.settings.interval = (parseFloat(e.target.value) || 5) * 1000;
        localStorage.setItem('sm_interval', state.settings.interval);
    };

    el('resetBtn').onclick = async () => {
        if (!confirm("Request Server to Reset all histograms?")) return;
        try {
            el('resetBtn').disabled = true;
            await httpRequest("/ResetAll/cmd.json", "text");
            drawGrid();
        } catch (err) {
	    alert("Failed to send reset command");
	} finally {
            el('resetBtn').disabled = false;
        }

    };
}

//---------------------------------------------------
async function drawGrid() {

    let retryCount = 0;
    while (await checkBusyStatus()) {
        if (retryCount > 20) break;
        await new Promise(r => setTimeout(r, 50));
        retryCount++;
    }
    
    await updateStatusInfo();

    const container = document.getElementById('grid-container');
    if (!container) return;

    const start = state.currentPage * state.settings.histsPerPage;
    const items = state.activePaths.slice(start, start + state.settings.histsPerPage);

    container.innerHTML = '';
    container.style.display = 'grid';
    container.style.gridTemplateColumns = `repeat(${state.settings.columns}, 1fr)`;
    container.style.gridTemplateRows = `repeat(${state.settings.rows}, auto)`; 
    container.style.alignContent = 'start';
    
    const promises = items.map(async (path, i) => {
        const divId = `draw_div_${i}`;
        const wrapper = document.createElement('div');
        wrapper.className = 'grid-wrapper';
	wrapper.style.position = 'relative';	

	const boxHeight = "30vh"; 
        wrapper.innerHTML = `<div id="${divId}" class="grid-item" style="height:${boxHeight}; min-height:250px;"></div>`;
        // --------------------
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
            const obj = await httpRequest(`/${path}/root.json`, "object");
            if (obj) await redraw(divId, obj, "colz");
        } catch (e) {
            console.error(`Failed to draw ${path}:`, e);
            if (e.status === 404) {
                console.warn("Histogram lost. Refreshing list...");
            }
        }
    });

    await Promise.all(promises);
}

//---------------------------------------------------
async function updateStatusInfo() {
    const elTime = document.getElementById('server-date-time');
    const autoResetEl = document.getElementById('auto-reset-info');

    try {
        const [timeRes, startRes, configRes, entriesRes, busyRes] = await Promise.all([
            httpRequest("/Status/ServerTime/root.json", "object"),
            httpRequest("/Status/ServerStartTime/root.json", "object"),
            httpRequest("/Config/Contents/root.json", "object"),
            httpRequest("/Status/Entries/root.json", "object"),
            httpRequest("/Status/IsAnalysisBusy/root.json", "object")
        ]);

        if (timeRes) elTime.textContent = timeRes.fTitle;

	if (startRes && state.server.startTime !== "" && startRes.fTitle !== state.server.startTime) {
            location.reload();
            return;
        }
	if (startRes) state.server.startTime = startRes.fTitle;

        if (busyRes && busyRes.fVal === 1) {
            elTime.textContent += " (BUSY)";
        }

	if (entriesRes && autoResetEl) {
            const val = entriesRes.fVal;
	    const statusStr = state.server.autoReset ? 'Auto Reset ON' : 'Auto Reset OFF';
            autoResetEl.innerText = `${statusStr}\nEntries: ${val.toLocaleString()}`;
            if (state.server.autoReset) {
                autoResetEl.innerText += " / " + state.server.autoResetEvents.toLocaleString();
            }
        }

	const elPage = document.getElementById('page-info');
	const total = state.activePaths.length;
	const maxPage = Math.ceil(total / state.settings.histsPerPage) || 1;
	if (elPage) elPage.innerText = `(Page ${state.currentPage + 1}/${maxPage})`;
	
        showDisconnected(false);
        state.lastSuccessTime = Date.now();

    } catch (err) {
        if ((Date.now() - state.lastSuccessTime) > 5000) showDisconnected(true);
    }
}
//---------------------------------------------------
async function syncServerState() {
    console.log("Syncing server state (Config and Histogram hierarchy)...");
    
    try {
        const [configRes, hierarchy] = await Promise.all([
            httpRequest("/Config/Contents/root.json", "object"),
            httpRequest("/h.json", "object")
        ]);

        if (configRes && configRes.fTitle) {
            try {
                const config = JSON.parse(configRes.fTitle);
                state.server.autoReset = config.auto_reset;
                state.server.autoResetEvents = config.auto_reset_events;
                state.server.skipPaths = config.skip_histograms || [];
            } catch (e) {
                console.error("Failed to parse config JSON", e);
            }
        }

        state.allPaths = [];
        findHistograms(hierarchy);

        state.activePaths = state.allPaths.filter(path => 
            !state.server.skipPaths.includes(path)
        );

        updateHistList();
        
        return true;
    } catch (err) {
        console.error("Failed to sync server state:", err);
        return false;
    }
}

//---------------------------------------------------
async function checkBusyStatus() {
    try {
        const res = await httpRequest("/Status/IsAnalysisBusy/root.json", "object");
        return res && res.fVal === 1;
    } catch (e) {
        return false;
    }
}
//---------------------------------------------------
function startMonitoring() {
    stopMonitoring();
    const loop = async () => {
        if (!state.isRunning) return;
        
        const maxPage = Math.ceil(state.activePaths.length / state.settings.histsPerPage);
        if (maxPage > 1) {
            state.currentPage = (state.currentPage + 1) % maxPage;
            const elPageInput = document.getElementById('inputPage');
            if (elPageInput) elPageInput.value = state.currentPage + 1;
        }

        await drawGrid();
        state.updateTimer = setTimeout(loop, state.settings.interval);
    };
    state.updateTimer = setTimeout(loop, state.settings.interval);
}

function stopMonitoring() {
    if (state.updateTimer) {
        clearTimeout(state.updateTimer);
        state.updateTimer = null;
    }
}

function updateUIStates() {
    const el = (id) => document.getElementById(id);
    const runBtn = el('runBtn');
    const status = el('status-info');
    const inputs = [el('inputRows'), el('inputCols'), el('inputPage'), el('inputInterval'), el('drawBtn'), el('resetBtn')];

    if (runBtn) runBtn.innerHTML = state.isRunning ? 'Stop Auto Update' : 'Start Auto Update';
    if (status) status.innerText = state.isRunning ? 'Auto Update ON' : 'Auto Update OFF';
    inputs.forEach(input => {
        if (!input) return;
        input.disabled = state.isRunning;
        input.style.cursor = state.isRunning ? "not-allowed" : "auto";
    });
    
}

function showDisconnected(show) {
    const warn = document.getElementById('process-warning');
    if (warn) warn.style.display = show ? "block" : "none";
}

function findHistograms(node, path = "") {
    if (!node) return;
    const name = node._name || "";
    const kind = node._kind || "";
    const nextPath = (name && name !== "ROOT") ? (path ? `${path}/${name}` : name) : path;
    if (kind.includes("TH") || kind.includes("TProfile")) state.allPaths.push(nextPath);
    if (node._childs) node._childs.forEach(child => findHistograms(child, nextPath));

}

function updateHistList() {
    const container = document.getElementById('hist-list');
    if (!container) return;

    container.innerHTML = state.allPaths.map((path, i) => `
        <li class="histlist-item">
            <input type="checkbox" class="hist-cb" data-path="${path}" id="cb_${i}" 
                   ${state.activePaths.includes(path) ? 'checked' : ''}>
            <label for="cb_${i}">${path}</label>
        </li>
    `).join('');

    container.querySelectorAll('.hist-cb').forEach(cb => {
        cb.onchange = () => {
            
            const checkedPaths = new Set();
            container.querySelectorAll('.hist-cb:checked').forEach(checkedEl => {
                checkedPaths.add(checkedEl.dataset.path);
            });

            state.activePaths = state.allPaths.filter(path => checkedPaths.has(path));
            
            const elPage = document.getElementById('page-info');
            const maxPage = Math.ceil(state.activePaths.length / state.settings.histsPerPage) || 1;
            if (elPage) elPage.innerText = `(Page ${state.currentPage + 1}/${maxPage})`;
        };
    });
    
}

init();

