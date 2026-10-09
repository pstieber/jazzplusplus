# ---------------------------------------------------------------------------
# PortMidi library (bundled)
# ---------------------------------------------------------------------------

set(
  PORTMIDI_SOURCES
    pm_common/pmutil.c
    pm_common/portmidi.c
)

set(
  PORTTIME_SOURCES
    porttime/porttime.c
)

# Platform-specific PortMidi sources
if(WIN32)
  list(
    APPEND PORTMIDI_SOURCES
      pm_win/pmwin.c
      pm_win/pmwinmm.c
  )
  list(
    APPEND PORTTIME_SOURCES
      porttime/ptwinmm.c
  )
elseif(APPLE)
  list(
    APPEND PORTMIDI_SOURCES
      pm_mac/pmmac.c
      pm_mac/finddefault.c
  )
  list(
    APPEND PORTTIME_SOURCES
      porttime/ptmacosx_cf.c
  )
elseif(UNIX)
  list(
    APPEND PORTMIDI_SOURCES
      pm_linux/pmlinux.c
      pm_linux/pmlinuxalsa.c
  )
  list(
    APPEND PORTTIME_SOURCES
      porttime/ptlinux.c
  )
endif()

# ---------------------------------------------------------------------------
# Build PortMidi as a static library
# ---------------------------------------------------------------------------
add_library(
  portmidi STATIC
    ${PORTMIDI_SOURCES}
    ${PORTTIME_SOURCES}
)

target_compile_definitions(portmidi PRIVATE NEWBUFFER)

target_include_directories(
  portmidi
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/pm_common
    ${CMAKE_CURRENT_SOURCE_DIR}/porttime
)

if(UNIX AND NOT APPLE)
  if(ALSA_FOUND)
    target_link_libraries(portmidi PRIVATE ${ALSA_LIBRARIES})
    target_include_directories(portmidi PRIVATE ${ALSA_INCLUDE_DIRS})
  endif()
endif()
