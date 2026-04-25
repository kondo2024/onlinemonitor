# Online Monitor

A real-time visualization tool using ANAROOT, ROOT-THttpServer and JSROOT.

## Requirements
* ANAROOT
* ROOT 6 (with THttpServer enabled)
* nlohmann/json (C++ JSON library)
* C++17 compatible compiler

## Installation
1. Set ANAROOT Environment Variable (TARTSYS)
2. Build: Compile using your Makefile or CMake.
   cd onlinemonitor
   mkdir build
   mkdir install
   cd build
   cmake -DCMAKE_INSTALL_PREFIX=../install ../
3. Run:
   source install/bin/setup_onlinemonitor.sh
   online_monitor (online mode, )
   online_monitor online [config.json] (online, canvas mode)
   online_monitor ridffile [config.json] (offline, canvas mode)

## Quick Usage
Access the monitoring dashboard via your browser:
* Main UI: http://localhost:8080/web/index.html
 * simple UI is also available at: http://localhost:8080
* Local Controls: click Canvas and press a key, then keyboard control is active

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

## Third-party Software
This software incorporates the following third-party library:
* [JSROOT](https://root.cern/js/) - Licensed under the GNU Lesser General Public License (LGPL), version 2.1.

The original license file is included in the `web/jsroot/` directory.
