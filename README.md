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
|analyzers      | Array of Strings List of analyzer class names to be instantiated|
|hist_range_file| String Path to the JSON file defining histogram binning and ranges|
|skip_histograms| Array of Strings List of histogram names to be excluded from creation and display. Useful for debugging or reducing memory usage|

### Auto Reset (Statistics)
|auto_reset       | Boolean If true, all histogram statistics will be cleared automatically when the event count reaches the threshold.|
|auto_reset_events| Number The threshold of event entries for the auto-reset trigger.|

### Display Settings (display)
|update_interval_ms| Number Refresh rate for the JSROOT web interface in milliseconds.|
|default_rows / default_columns| Number Initial layout of the histogram grid on the web UI.|
|canvas_width / canvas_height| Number Internal canvas dimensions used for server-side rendering. This affects the relative positioning of the statistics box (TPaveStats) and font scaling.|

### Figure Export (fig)
|auto_save            | Boolean Enable/disable periodic background saving of histograms (e.g., as .png or .root files)|
|save_interval_minutes| Number Time interval between each auto-save operation|

### Network
|http_port| Number TCP port number for the THttpServer (default: 8080)|

## Config (config.json)
| Key | Description |
| :--- | :--- |
| http_port | Web server port (default: 8080). |
| auto_reset | Enable/disable automatic histogram reset. |
| auto_reset_events | Number of events before an automatic reset. |
| display | Set default_rows, default_columns, and update_interval_ms. |
| skip_histograms | List of histogram names to hide from the UI. |

## to do
- jsroot -> submodule
- core, analyzer -> source/core,analyzer
- PlasticAnalyzer, BDCAnalyzer, FDCAnalyzer, HODAnalyzer, NEBULAAnalyzer, PPACAnalyzer, BeamFocusAnalyzer

