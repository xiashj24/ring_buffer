# ── CPM bootstrap ────────────────────────────────────────────────────────────
set(CPM_VERSION 0.40.2)
set(CPM_FILE "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_VERSION}.cmake")

if(NOT EXISTS "${CPM_FILE}")
  message(STATUS "Downloading CPM.cmake v${CPM_VERSION}...")
  file(DOWNLOAD
    "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_VERSION}/CPM.cmake"
    "${CPM_FILE}"
  )
endif()

include("${CPM_FILE}")

# ── Dependencies ──────────────────────────────────────────────────────────────
CPMAddPackage("gh:catchorg/Catch2@3.7.1")

include(CTest)
list(APPEND CMAKE_MODULE_PATH "${Catch2_SOURCE_DIR}/extras")
include(Catch)

# ── Test discovery ────────────────────────────────────────────────────────────
# Any file matching *.test.cpp is picked up automatically.
# To add a new test unit, just drop a new *.test.cpp file in this directory.
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.test.cpp")

foreach(TEST_SOURCE IN LISTS TEST_SOURCES)
  get_filename_component(TEST_TARGET "${TEST_SOURCE}" NAME_WE)

  add_executable("${TEST_TARGET}" "${TEST_SOURCE}")

  target_link_libraries("${TEST_TARGET}" PRIVATE Catch2::Catch2WithMain)

  # Headers from src/ are available to all test targets
  target_include_directories("${TEST_TARGET}" PRIVATE "${CMAKE_SOURCE_DIR}/src")

  catch_discover_tests("${TEST_TARGET}")
endforeach()
