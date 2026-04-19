cmake_minimum_required(VERSION 3.15)
project(onlinemonitor LANGUAGES CXX)

find_package(ROOT REQUIRED COMPONENTS Net Core RIO Hist Tree Gui RHTTP Thread)
include(${ROOT_USE_FILE})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -pthread)

file(GLOB_RECURSE CORE_SOURCES "core/src/*.cc")
file(GLOB_RECURSE DETECTOR_SOURCES "detectors/src/*.cc")
file(GLOB_RECURSE HEADERS "core/include/*.hh" "detectors/include/*.hh")

set(MAIN_SOURCE "onlinemonitor.cc")

add_executable(onlinemonitor ${MAIN_SOURCE} ${CORE_SOURCES} ${DETECTOR_SOURCES})

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

target_link_libraries(onlinemonitor PRIVATE ${ANAROOT_LIBRARIES} ${ROOT_LIBRARIES})
#include_directories($ENV{TARTSYS}/include)
### ANAROOT ---------------------------


target_include_directories(onlinemonitor PRIVATE $ENV{TARTSYS}/include)
target_include_directories(onlinemonitor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/core/include)
target_include_directories(onlinemonitor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/detectors/include)

target_compile_definitions(onlinemonitor PRIVATE "USE_THREADS")

install(FILES ${HEADERS} DESTINATION include)
install(TARGETS onlinemonitor DESTINATION ${CMAKE_INSTALL_BINDIR})
