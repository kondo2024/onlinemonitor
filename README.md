# Online Monitor

A real-time visualization tool using ANAROOT, ROOT-THttpServer and JSROOT.

## Requirements
* ANAROOT
* ROOT 6 (with THttpServer enabled)
* nlohmann/json (C++ JSON library)
* C++17 compatible compiler
* JSROOT (compatible with ver 7.10.3)

## Installation
You can skip 1-3 if you install sm_online_utils.
1. download
```
git clone --recursive https://github.com/kondo2024/onlinemonitor.git
```
2. Set ANAROOT Environment Variable (TARTSYS)
3. Build: Compile using your Makefile or CMake.
```
cd onlinemonitor
mkdir build
mkdir install
cd build
cmake -DCMAKE_INSTALL_PREFIX=../install ../
cd ../install/onlinemonitor/config/
```
4. copy json files for default configurations
```
cp config.json.example config.json
cp hist_ranges.json.example hist_ranges.json
```

## Quick Usage
```
tmux new -s onlinemonitor (or tmux a -t onlinemonitor)
source install/bin/setup_onlinemonitor.sh
onlinemonitor
```
To detach tmux session, Ctrl-b d

### online mode
* Main UI: http://server:10101/web/index.html
* simple UI is also available at: http://server:10101
replace "server" for your system.

Load specific configuration json file (online, canvas mode).
```
onlinemonitor online [config.json]
```
When you update the onlinemonitor and if it is not reflected on your display with browser, try Ctrl+F5.

### offline mode
histograms are drawn in TCanvas. 
```
onlinemonitor ridffile [config.json] (offline, canvas mode)
```
Click Canvas, then keyboard control will be available.


## Parameter Details of Config (config.json)
### Core Settings
| Key | Description |
| :--- | :--- |
| analyzer       | Analyzer settings, see below|
| display       | Display settings, see below|
| fig       | Auto figure save settings, see below|
| hist_range_file | Path to the JSON file defining histogram binning and ranges, should be located at the same dir as config.json|
| skip_histograms | String list of histogram names to be excluded in the display|
| http_port| TCP port number for the THttpServer (default: 10101)|
| auto_reset       | If true, all histogram statistics will be cleared automatically when the event count reaches the threshold|
| auto_reset_events| The threshold of event entries for the auto-reset|

### Analyzer Settings
| Key | Description |
| :--- | :--- |
| list       | String list of analyzer class names to be instantiated|
| dc_tdc_file       | rootfile name containing TDC distributions for DCs|
| z_bdc1 / z_bdc2 / z_target | Z positions of BDC1/BDC2/Target for tracking|

### Display Settings (display)
| Key | Description |
| :--- | :--- |
| update_interval_ms| Time interval of display refresh in milliseconds|
| default_rows / default_columns| Initial layout of the histogram grid|
| canvas_width / canvas_height| Canvas dimensions for canvas mode |

### Figure Auto Save (fig)
Auto save of figures is available. About 2GB disk space was used during s074/s055 campaign in 2026.
| Key | Description |
| :--- | :--- |
| auto_save            | if true, figures(png) are automatically saved periodically in background |
| save_interval_minutes| Time interval between each auto-save operation|

## For developers

#### User plots
If you want to include a specific plot for you experiment, modify UserAnalyzer.hh/.cc. If you want to combine several detectors, use HistogramManager::fUserVarMap.

Example of setting value (BDCAnalyzer.cc)
```
  HistogramManager::GetInstance()->SetUserVariable("TGTX",TGTX);	
```

Example of using value (TOGAXSIAnalyzer.cc)
```
  HistogramManager* hm = HistogramManager::GetInstance();
  double tgtx = hm->GetUserVariable("TGTX");
```

#### communication by DAQ
When a new run starts, call this. Then histograms are cleared.
```
curl "http://server:10101/ResetAll/cmd.json"
```

To save the figures use this.
```
curl "http://server:10101/SaveFigures/cmd.json"
```

## Known issues
- memory leak, browser spend a lot of memory in client
- Too many ANAROOT messages prevents to read onlinemonitor messages

## To do
- classify histograms by TFolder?
- clear fit line if histogram clear happens
- histogram fit should happen when integral is larger than xxx
- PPACAnalyzer

