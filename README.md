# Online Monitor

A real-time visualization tool using ANAROOT, ROOT-THttpServer and JSROOT.

## Requirements
* ANAROOT
* ROOT 6 (with THttpServer enabled)
* nlohmann/json (C++ JSON library)
* C++17 compatible compiler
* JSROOT (compatible with ver 7.10.3)

## Installation
```
git clone --recursive https://github.com/kondo2024/onlinemonitor.git
```
Set ANAROOT Environment Variable (TARTSYS)
Build: Compile using your Makefile or CMake.
```
cd onlinemonitor
mkdir build
mkdir install
cd build
cmake -DCMAKE_INSTALL_PREFIX=../install ../
```


## Quick Usage
```
source install/bin/setup_onlinemonitor.sh
```
### online mode, access to http://server:8080/web/index.html
Access the monitoring dashboard via your browser:
* Main UI: http://server:8080/web/index.html
* simple UI is also available at: http://server:8080
```
online_monitor
```
### load specific configuration json file
```
online_monitor online [config.json] (online, canvas mode)
```
### offline mode, histograms are drawn in TCanvas
Click Canvas, then keyboard control will be available.
```
online_monitor ridffile [config.json] (offline, canvas mode)
```

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
