cmake_minimum_required(VERSION 3.15)
project(smonlinemonitor LANGUAGES CXX)

# ROOT を探す
find_package(ROOT REQUIRED COMPONENTS RIO Hist Tree Gui)
include(${ROOT_USE_FILE})

# C++17 を使用
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# コンパイルオプション（TThread セーフやデバッグ向け）
add_compile_options(-Wall -Wextra -pthread)

# ソースファイルをまとめる
file(GLOB_RECURSE CORE_SOURCES "core/src/*.cc")
file(GLOB_RECURSE DETECTOR_SOURCES "detectors/src/*.cc")
# include ディレクトリ内のすべての .hh ファイルを取得（IDEでの表示用）
file(GLOB_RECURSE HEADERS "core/include/*.hh" "detectors/include/*.hh")

set(MAIN_SOURCE "smonlinemonitor.cc")

# 実行形式を生成
add_executable(smonlinemonitor ${SOURCES} ${HEADERS})

# ROOT ライブラリをリンク
target_link_libraries(smonlinemonitor ${ROOT_LIBRARIES})

### ANAROOT ---------------------------
if(DEFINED ENV{TARTSYS})
  message(STATUS "Found ANAROOT, TARTSYS = " $ENV{TARTSYS})
else()
  message(FATAL_ERROR "\$TARTSYS is not defined. please define the environment for ANAROOT.")
endif()

set(ANAROOT_LIBRARIES)
foreach(_cpt anaroot anacore anabrips anasamurai XMLParser)
  find_library(ANAROOT_${_cpt}_LIBRARY ${_cpt} $ENV{TARTSYS}/lib)
  if(ANAROOT_${_cpt}_LIBRARY)
    list(APPEND ANAROOT_LIBRARIES ${ANAROOT_${_cpt}_LIBRARY})
  endif()
endforeach()

target_link_libraries(smonlinemonitor ${ANAROOT_LIBRARIES})
#include_directories($ENV{TARTSYS}/include)
target_include_directories(smonlinemonitor PRIVATE $ENV{TARTSYS}/include)

# include ディレクトリ
target_include_directories(smonlinemonitor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/core/include)
target_include_directories(smonlinemonitor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/detectors/include)

# コンパイル時の定義
target_compile_definitions(smonlinemonitor PRIVATE "USE_THREADS")

install(FILES ${INC_FILES} DESTINATION include)
install(TARGETS smonlinemonitor DESTINATION ${CMAKE_INSTALL_BINDIR})
