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
        configPath: "",
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
        const status = await httpRequest("/Status/ServerStartTime/root.json", "object");
        if (status) state.server.startTime = status.fTitle;

        await syncServerState();

        setupEventListeners();
        updateUIStates();
        await drawGrid();
        
        console.log("Initialization done.");
    } catch (err) {
        showDisconnected(true);
    }
    
//    try {
//        const [status, hierarchy, configContent] = await Promise.all([
//            httpRequest("/Status/ServerStartTime/root.json", "object"),
//            httpRequest("/h.json", "object"),
//	    httpRequest("/Config/Contents/root.json", "object")
//        ]);
//
//        if (status) state.server.startTime = status.fTitle;
//
//        if (configContent && configContent.fTitle) {
//            try {
//                const config = JSON.parse(configContent.fTitle);
//                state.server.autoReset = config.auto_reset || false;
//                state.server.autoResetEvents = config.auto_reset_events || 1000000;
//                state.server.skipPaths = config.skip_histograms || [];
//            } catch (e) {
//                console.error("Failed to parse server config contents", e);
//            }
//        }
//
//        state.allPaths = [];
//        findHistograms(hierarchy);
//
//        state.activePaths = state.allPaths.filter(path => 
//            !state.server.skipPaths.includes(path)
//        );
//
//        setupEventListeners();
//        updateHistList();
//        updateUIStates();
//
//        await drawGrid();
//        console.log("Initialization done. Server Start Time:", state.server.startTime);
//    } catch (err) {
//        console.error("Initialization Failed:", err);
//        showDisconnected(true);
//    }
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
        if (retryCount > 10) break;
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
    container.style.gridTemplateRows = `repeat(${state.settings.rows}, 1fr)`;
    
    const promises = items.map(async (path, i) => {
        const divId = `draw_div_${i}`;
        const wrapper = document.createElement('div');
        wrapper.className = 'grid-wrapper';
	wrapper.style.position = 'relative';	
        wrapper.innerHTML = `<div id="${divId}" class="grid-item" style="height:300px;"></div>`;
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

        if (startRes && startRes.fTitle) {
	    const currentServerStart = startRes.fTitle;

	    if (state.server.startTime === "") {
		state.server.startTime = currentServerStart;
	    } 
	    else if (currentServerStart !== state.server.startTime) {
		console.warn("Server restart detected. Re-syncing...");
		state.server.startTime = currentServerStart;

		await syncServerState();

		state.currentPage = 0;
		await drawGrid();     
		return;
	    }
        }
	
	if (configRes && configRes.fTitle) {
            const config = JSON.parse(configRes.fTitle);
            state.server.autoReset = config.auto_reset;
            state.server.autoResetEvents = config.auto_reset_events;
        }
	
        if (busyRes && busyRes.fVal === 1) {
            elTime.textContent += " (BUSY)";
        }

	if (entriesRes) {
            const currentEntries = entriesRes.fVal;
            state.lastEntries = currentEntries;

	    if (autoResetEl) {
		const statusStr = state.server.autoReset ? 'Auto Reset ON' : 'Auto Reset OFF';
                autoResetEl.innerText = `${statusStr}\nEntries: ${currentEntries}`;
                if (state.server.autoReset) {
                    autoResetEl.innerText += " / " + state.server.autoResetEvents;
                }
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
                state.server.autoReset = config.auto_reset || false;
                state.server.autoResetEvents = config.auto_reset_events || 1000000;
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
            <input type="checkbox" id="cb_${i}" ${state.activePaths.includes(path) ? 'checked' : ''}>
            <label for="cb_${i}">${path}</label>
        </li>
    `).join('');
}

init();


////---------------------------------------------------
////---------------------------------------------------
////---------------------------------------------------
////---------------------------------------------------
////---------------------------------------------------
////---------------------------------------------------
////---------------------------------------------------
//const state = {
//    settings: {
//        interval: parseInt(localStorage.getItem('sm_interval')) || 8000,
//        rows: 3,
//        columns: 3,
//        get histsPerPage() { return this.rows * this.columns; }
//    },
//    server: {
//	autoReset: false,
//        autoResetEvents: 1000000,
//        startTime: "",
//        configPath: ""
//    }
//    isRunning: false,
//    currentPage: 0,
//    lastSuccessTime: Date.now(),
//    defaultSkipPaths: [],
//    allPaths: [],
//    activePaths: [],
//    lastTotalEntries: 0,
//    lastEntryChangeTime: Date.now(),
//    updateTimer: null,
//    isBusyChecking: false
//};
//
//import { redraw, httpRequest } from './jsroot/modules/main.mjs';
//
//async function init() {
//    try {
//
//	const config = await httpRequest("config/config.json", "object");
//
//	if (config?.http_port) state.settings.httpPort = parseInt(config.http_port);
//	const port = state.settings.httpPort;
//	
//	if (config?.display?.update_interval_ms) {
//            const configInterval = parseInt(config.display.update_interval_ms);
//            if (!localStorage.getItem('sm_interval')) state.settings.interval = configInterval;
//        }
//
//	state.autoReset = config?.auto_reset || false;
//	if (config?.auto_reset_events) state.autoResetEvents = config["auto_reset_events"];
//	if (config?.display?.default_rows) state.settings.rows = parseInt(config.display.default_rows);
//	if (config?.display?.default_columns) state.settings.columns = parseInt(config.display.default_columns);
//	
//        state.defaultSkipPaths = config?.skip_histograms || [];
//	const hierarchy = await httpRequest(`http://localhost:${port}/h.json`, "object");
//        if (!hierarchy) throw new Error("Cannot access h.json");
//
//        state.allPaths = [];
//        findHistograms(hierarchy);
//	state.activePaths = state.allPaths.filter(path => 
//            !state.defaultSkipPaths.includes(path)
//        );
// 
//        setupEventListeners();
//        updateHistList();
//        updateUIStates();
//        
//        await drawGrid();
//        console.log("Initialization done");	
//    } catch (err) {
//        console.error("Initialization Failed:", err);
//        showDisconnected(true);
//    }
//}
////---------------------------------------------------
//async function checkBusyStatus() {
//    try {
//        const res = await httpRequest("/Status/IsAnalysisBusy/root.json", "object");
//        return res && res.fVal === 1;
//    } catch (e) {
//        return false;
//    }
//}
////---------------------------------------------------
//function setupEventListeners() {
//    const el = (id) => document.getElementById(id);
//
//    el('runBtn').onclick = () => {
//        state.isRunning = !state.isRunning;
//        if (state.isRunning) {
//            startMonitoring();
//        } else {
//            stopMonitoring();
//        }
//        updateUIStates();
//    };
//
//    el('drawBtn').onclick = () => {
//        updateUIStates();
//        drawGrid();
//    };
//
//    
//    el('inputRows').value = state.settings.rows;
//    el('inputCols').value = state.settings.columns;
//    
//    const onLayoutChange = () => {
//        state.settings.rows = parseInt(el('inputRows').value) || 1;
//        state.settings.columns = parseInt(el('inputCols').value) || 1;
//        state.currentPage = 0;
//        el('inputPage').value = 1;
//    };
//    el('inputRows').onchange = onLayoutChange;
//    el('inputCols').onchange = onLayoutChange;
//
//    el('inputPage').onchange = (e) => {
//        const maxPage = Math.ceil(state.activePaths.length / state.settings.histsPerPage);
//        state.currentPage = Math.min(Math.max(0, (parseInt(e.target.value) || 1) - 1), maxPage - 1);
//        e.target.value = state.currentPage + 1;
//    };
//
//    el('inputInterval').value = state.settings.interval / 1000; // ms -> sec
//    el('inputInterval').onchange = (e) => {
//        const newSec = parseFloat(e.target.value) || 5;
//        state.settings.interval = newSec * 1000;
//        localStorage.setItem('sm_interval', state.settings.interval);
//    };
//    
//    el('resetBtn').onclick = async () => {
//        if (!confirm("Reset all histograms?")) return;
//        try {
//            el('resetBtn').disabled = true;
//            await httpRequest("/ResetAll/cmd.json", "text");
//	    state.lastTotalEntries = 0;
//	    state.lastEntryChangeTime = Date.now();
//            await drawGrid();
//        } catch (err) {
//            alert("Failed to reset histograms");
//        } finally {
//            el('resetBtn').disabled = false;
//        }
//    };
//}
//
////---------------------------------------------------
//function startMonitoring() {
//    stopMonitoring();
//    const loop = async () => {
//        if (!state.isRunning) return;
//        
//        const maxPage = Math.ceil(state.activePaths.length / state.settings.histsPerPage);
//        state.currentPage = (state.currentPage + 1) >= maxPage ? 0 : state.currentPage + 1;
//        const elPageInput = document.getElementById('inputPage');
//        if (elPageInput) elPageInput.value = state.currentPage + 1;
//        
//        await drawGrid();
//        state.updateTimer = setTimeout(loop, state.settings.interval);
//    };
//    state.updateTimer = setTimeout(loop, state.settings.interval);
//}
//
////---------------------------------------------------
//function stopMonitoring() {
//    if (state.updateTimer) {
//        clearInterval(state.updateTimer);
//        state.updateTimer = null;
//    }
//}
//
////---------------------------------------------------
//async function drawGrid() {
//
//    const isBusy = await checkBusyStatus();
//    if (isBusy) {
//        console.log("Server is busy (Analysis in progress). Retrying in 100ms...");
//        setTimeout(() => drawGrid(), 100);
//        return;
//    }
//
//    await updateStatusInfo();
//    
//    const container = document.getElementById('grid-container');
//    const elPage = document.getElementById('page-info');
//    if (!container) return;
//
//    const total = state.activePaths.length;
//    const maxPage = Math.ceil(total / state.settings.histsPerPage) || 1;
//    if (elPage) elPage.innerText = `(Page ${state.currentPage + 1}/${maxPage})`;
//
//    const start = state.currentPage * state.settings.histsPerPage;
//    const items = state.activePaths.slice(start, start + state.settings.histsPerPage);
//
//    // Grid Layout Layout
//    container.innerHTML = '';
//    container.style.display = 'grid';
//    container.style.gridTemplateColumns = `repeat(${state.settings.columns}, 1fr)`;
//    container.style.gridTemplateRows = `repeat(${state.settings.rows}, 1fr)`;
//
//
//    const promises = items.map(async (path, i) => {
//        const divId = `draw_div_${i}`;
//        const wrapper = document.createElement('div');
//        wrapper.className = 'grid-wrapper';
//	wrapper.style.position = 'relative';
//        wrapper.innerHTML = `<div id="${divId}" class="grid-item"></div>`;
//	if (state.isRunning) {
//	    const cover = document.createElement('div');
//	    cover.style.position = 'absolute';
//	    cover.style.top = '0';
//	    cover.style.left = '0';
//	    cover.style.width = '100%';
//	    cover.style.height = '100%';
//	    cover.style.zIndex = '100'; 
//	    cover.style.background = 'rgba(255, 255, 255, 0)';
//	    cover.style.cursor = 'not-allowed';
//	    wrapper.appendChild(cover);
//	}
//	
//        container.appendChild(wrapper);
//
//        try {
//
//	    const port = state.settings.httpPort;
//            const obj = await httpRequest(`http://localhost:${port}/${path}/root.json.gz`, "object");
//            if (obj) await redraw(divId, obj, "colz");
//        } catch (e) {
//            console.error(`Failed to draw ${path}:`, e);
//        }
//    });
//
//    await Promise.all(promises);
//
//}
//
////---------------------------------------------------
//async function updateStatusInfo() {
//    const elTime = document.getElementById('server-date-time');
//    const elDataWarn = document.getElementById('data-warning');
//    const autoReset = document.getElementById('auto-reset-info');
//
//    try {
//        const res = await httpRequest("/Status/ServerTime/root.json", "object");
//        if (res?.fTitle) {
//            elTime.textContent = res.fTitle;
//            state.lastSuccessTime = Date.now();
//            showDisconnected(false);
//        }
//
//	if (state.allPaths.length > 0) {
//            const headPath = state.allPaths[0];
//            const obj = await httpRequest("/Status/Entries/root.json", "object");
//            const rstevt = await httpRequest("/Status/AutoResetEvents/root.json", "object");
//	    
//            if (obj && obj.fVal !== undefined &&
//		rstevt && rstevt.fVal !== undefined    ) {
//                const currentEntries = obj.fVal;
//		state.autoResetEvents = rstevt.fVal;
//
//		autoReset.innerText = state.autoReset ? 'Auto Reset ON' : 'Auto Reset OFF';
//		if (autoReset) {
//		    if (state.autoReset) autoReset.innerText += "\n(" + state.autoResetEvents +" events)";
//		}
//
//                if (currentEntries > state.lastTotalEntries) {
//                    state.lastTotalEntries = currentEntries;
//                    state.lastEntryChangeTime = Date.now();
//                    elDataWarn.style.display = "none";
//                } else {
//                    const idleSec = (Date.now() - state.lastEntryChangeTime) / 1000;
//                    if (state.isRunning && idleSec >= 10) {
//                        elDataWarn.style.display = "block";
//                    }
//                }
//		// auto reset
//		if (state.autoReset && currentEntries >= state.autoResetEvents) {
//                    //console.log(`Auto Reset triggered: ${currentEntries} entries reached.`);
//                    await performAutoReset();
//		}
//            }
//        }	
//    } catch (err) {
//        const idleSec = (Date.now() - state.lastSuccessTime) / 1000;
//        if (idleSec > 5) {
//            showDisconnected(true);
//            state.isRunning = false;
//            //stopMonitoring();
//            updateUIStates();
//        }
//    }
//}
//
////---------------------------------------------------
//function updateUIStates() {
//    const el = (id) => document.getElementById(id);
//    const runBtn = el('runBtn');
//    const status = el('status-info');
//    const autoReset = el('auto-reset-info');
//    const inputs = [el('inputRows'), el('inputCols'), el('inputPage'), el('inputInterval'), el('drawBtn'), el('resetBtn')];
//
//    runBtn.innerHTML = state.isRunning ? 'Auto Update<br>ON => OFF' : 'Auto Update<br>OFF => ON';
//    if (status) status.innerText = state.isRunning ? 'Auto Update ON' : 'Auto Update OFF';
//
//    autoReset.innerText = state.autoReset ? 'Auto Reset ON' : 'Auto Reset OFF';
//    if (autoReset) {
//	if (state.autoReset) autoReset.innerText += "\n(" + state.autoResetEvents +" events)";
//    }
//    
//    inputs.forEach(input => {
//        if (!input) return;
//        input.disabled = state.isRunning;
//        input.style.cursor = state.isRunning ? "not-allowed" : "auto";
//    });
//}
//
////---------------------------------------------------
//function showDisconnected(show) {
//    const warn = document.getElementById('process-warning');
//    const time = document.getElementById('server-date-time');
//    if (warn) warn.style.display = show ? "block" : "none";
//    if (show && time) time.textContent = "Disconnected";
//}
//
////---------------------------------------------------
//function findHistograms(node, path = "") {
//    if (!node) return;
//    const name = node._name || "";
//    const kind = node._kind || "";
//    const nextPath = (name && name !== "ROOT") ? (path ? `${path}/${name}` : name) : path;
//
//    if (kind.includes("TH") || kind.includes("TProfile")) {
//        state.allPaths.push(nextPath);
//    }
//
//    if (node._childs) {
//        node._childs.forEach(child => findHistograms(child, nextPath));
//    }
//}
//
////---------------------------------------------------
//function updateHistList() {
//    const container = document.getElementById('hist-list');
//    if (!container) return;
//
//    container.innerHTML = '';
//    state.allPaths.forEach((path, i) => {
//        const li = document.createElement('li');
//        li.className = 'histlist-item';
//        
//        const cb = document.createElement('input');
//        cb.type = 'checkbox';
//        cb.checked = state.activePaths.includes(path);
//        
//        cb.onchange = () => {
//            const updatedPaths = state.allPaths.filter((p, index) => {
//                const checkbox = document.querySelector(`#cb_${index}`);
//                if (index === i) return cb.checked; 
//                return checkbox ? checkbox.checked : state.activePaths.includes(p);
//            });
//
//            state.activePaths = updatedPaths;
//            
//            //drawGrid();
//        };
//
//        cb.id = `cb_${i}`;
//
//        const label = document.createElement('label');
//        label.innerText = path;
//        label.setAttribute('for', `cb_${i}`);
//        label.style.cursor = "pointer";
//
//        li.appendChild(cb);
//        li.appendChild(label);
//        container.appendChild(li);
//    });
//}
////---------------------------------------------------
//async function performAutoReset() {
//    try {
//        const originalAutoReset = state.autoReset;
//        state.autoReset = false; 
//
//        await httpRequest("/ResetAll/cmd.json", "text");
//        //console.log("Auto reset successful.");
//
//        state.lastTotalEntries = 0;
//        state.lastEntryChangeTime = Date.now();
//
//        setTimeout(() => {// wait for 2000ms
//            state.autoReset = originalAutoReset;
//        }, 2000);
//
//    } catch (err) {
//        console.error("Auto Reset Failed:", err);
//    }
//}
////---------------------------------------------------
//init();
//
////---------------------------------------------------
////---------------------------------------------------
////---------------------------------------------------
//
