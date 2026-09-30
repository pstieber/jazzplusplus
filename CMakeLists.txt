cmake_minimum_required(VERSION 3.10)

project(JazzPlusPlus VERSION 5.0.0 LANGUAGES C CXX)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(ENABLE_ALSA "Enable ALSA audio support (Linux)" ON)
option(ENABLE_SEQUENCER2 "Enable /dev/sequencer2 support" OFF)
option(ENABLE_MPU401 "Enable MPU-401 support" OFF)
option(ENABLE_DEBUG "Enable debugging code" OFF)
option(ENABLE_OPTIMIZE "Enable optimization" OFF)
option(ENABLE_PROFILE "Enable gprof profiling" OFF)

# ---------------------------------------------------------------------------
# C++ Standard
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)

# ---------------------------------------------------------------------------
# Compiler flags
# ---------------------------------------------------------------------------
if(ENABLE_DEBUG)
  add_compile_options(-g -fno-inline)
else()
  add_compile_options(-DNDEBUG)
endif()

if(ENABLE_PROFILE)
  add_compile_options(-pg)
  add_link_options(-pg)
endif()

if(ENABLE_OPTIMIZE)
  add_compile_options(-O3)
endif()

add_compile_options(-Wall -Wno-unknown-pragmas)

# ---------------------------------------------------------------------------
# wxWidgets
# ---------------------------------------------------------------------------
find_package(wxWidgets 2.9.4 REQUIRED COMPONENTS core base adv)
if(NOT wxWidgets_FOUND)
  message(FATAL_ERROR "wxWidgets not found. Please install wxWidgets >= 2.9.4")
endif()
include(${wxWidgets_USE_FILE})

# ---------------------------------------------------------------------------
# ALSA (Linux)
# ---------------------------------------------------------------------------
if(ENABLE_ALSA)
  find_package(ALSA)
  if(ALSA_FOUND)
    add_definitions(-DDEV_ALSA=1)
    include_directories(${ALSA_INCLUDE_DIRS})
  else()
    message(WARNING "ALSA not found. ALSA support will be disabled.")
    set(ENABLE_ALSA OFF)
  endif()
endif()

# ---------------------------------------------------------------------------
# Platform-specific settings
# ---------------------------------------------------------------------------
if(WIN32)
  # Windows-specific settings
  add_definitions(-D__WINDOWS__)
elseif(APPLE)
  # macOS-specific settings
  find_library(COREMIDI_LIBRARY CoreMIDI)
  find_library(COREAUDIO_LIBRARY CoreAudio)
  set(PLATFORM_LIBS ${COREMIDI_LIBRARY} ${COREAUDIO_LIBRARY})
elseif(UNIX)
  # Linux/Unix-specific settings
  if(ENABLE_SEQUENCER2)
    add_definitions(-DDEV_SEQUENCER2=1)
  endif()
  if(ENABLE_MPU401)
    add_definitions(-DDEV_MPU401=1)
  endif()
endif()

# ---------------------------------------------------------------------------
# PortMidi
# ---------------------------------------------------------------------------
# PortMidi is bundled in the portmidi/ subdirectory
add_subdirectory(portmidi)

# ---------------------------------------------------------------------------
# Subdirectories
# ---------------------------------------------------------------------------
add_subdirectory(src)
add_subdirectory(conf)

# ---------------------------------------------------------------------------
# Install data files
# ---------------------------------------------------------------------------
install(DIRECTORY song/ DESTINATION share/Jazz++/song)
install(DIRECTORY conf/ DESTINATION share/Jazz++/conf)
