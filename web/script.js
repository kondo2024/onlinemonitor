import { redraw, httpRequest } from './jsroot/modules/main.mjs';

//---------------------------------------------------
const state = {
    settings: {
        interval: parseInt(localStorage.getItem('sm_interval')) || 8000,
        rows: 3,
        columns: 3,
        get histsPerPage() { return this.rows * this.columns; }
    },
    isRunning: false,
    currentPage: 0,
    lastSuccessTime: Date.now(),
    defaultSkipPaths: [],
    allPaths: [],
    activePaths: [],
    lastTotalEntries: 0,
    lastEntryChangeTime: Date.now(),
    updateTimer: null
};

//---------------------------------------------------
async function init() {
    try {

	const config = await httpRequest("config/config.json", "object");
        state.defaultSkipPaths = config?.skip_histograms || [];

	const hierarchy = await httpRequest("http://localhost:8080/h.json", "object");
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
    state.updateTimer = setInterval(async () => {
        const maxPage = Math.ceil(state.activePaths.length / state.settings.histsPerPage);
        state.currentPage = (state.currentPage + 1) >= maxPage ? 0 : state.currentPage + 1;
        document.getElementById('inputPage').value = state.currentPage + 1;
        await drawGrid();
    }, state.settings.interval);
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
        wrapper.innerHTML = `<div id="${divId}" class="grid-item"></div>`;
        container.appendChild(wrapper);

        try {
            const obj = await httpRequest(`http://localhost:8080/${path}/root.json.gz`, "object");
            if (obj) {
		await redraw(divId, obj, "colz");
	    }
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
            const headPath = state.allPaths[0];
            const obj = await httpRequest(`http://localhost:8080/${headPath}/root.json.gz`, "object");

            if (obj && obj.fEntries !== undefined) {
                const currentEntries = obj.fEntries;

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
    const inputs = [el('inputRows'), el('inputCols'), el('inputPage'), el('drawBtn'), el('resetBtn')];

    runBtn.textContent = state.isRunning ? 'Pause' : 'Auto Update ON';
    if (status) status.innerText = state.isRunning ? 'Auto Update ON' : 'Auto Update OFF';

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

////---------------------------------------------------
////---------------------------------------------------
init();

//---------------------------------------------------
//---------------------------------------------------
//---------------------------------------------------
