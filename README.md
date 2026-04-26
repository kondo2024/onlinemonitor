# Online Monitor

A real-time visualization tool using ANAROOT, ROOT-THttpServer and JSROOT.

## Requirements
* ANAROOT
* ROOT 6 (with THttpServer enabled)
* nlohmann/json (C++ JSON library)
* C++17 compatible compiler
* JSROOT (compatible with ver 7.10.3)

## Installation
1. 
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
source install/bin/setup_onlinemonitor.sh
online_monitor
```
### online mode
* Main UI: http://server:8080/web/index.html
* simple UI is also available at: http://server:8080

Load specific configuration json file (online, canvas mode).
```
online_monitor online [config.json]
```
### offline mode
histograms are drawn in TCanvas. 
```
online_monitor ridffile [config.json] (offline, canvas mode)
```
Click Canvas, then keyboard control will be available.


## Parameter Details of Config (config.json)
### Core Settings
| Key | Description |
| :--- | :--- |
| analyzers       | Array of Strings List of analyzer class names to be instantiated|
| hist_range_file | String Path to the JSON file defining histogram binning and ranges|
| skip_histograms | Array of Strings List of histogram names to be excluded in the display|
| http_port| TCP port number for the THttpServer (default: 8080)|

### Auto Reset (Statistics)
| Key | Description |
| :--- | :--- |
| auto_reset       | If true, all histogram statistics will be cleared automatically when the event count reaches the threshold|
| auto_reset_events| The threshold of event entries for the auto-reset|

### Display Settings (display)
| Key | Description |
| :--- | :--- |
| update_interval_ms| Time interval of display refresh in milliseconds|
| default_rows / default_columns| Initial layout of the histogram grid|
| canvas_width / canvas_height| Canvas dimensions for canvas mode |

### Figure Export (fig)
| Key | Description |
| :--- | :--- |
| auto_save            | if true, figures(png) are automatically saved periodically in background |
| save_interval_minutes| Time interval between each auto-save operation|

## To do
- classified histograms by TFolder
- PlasticAnalyzer, BDCAnalyzer, FDCAnalyzer, HODAnalyzer, NEBULAAnalyzer, PPACAnalyzer
- BeamFocusAnalyzer and its parameters (relative positions) in json

