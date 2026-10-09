# ---------------------------------------------------------------------------
# Jazz++ main application sources
# ---------------------------------------------------------------------------

include(CheckCXXSourceCompiles)

check_cxx_source_compiles("
  #include <iostream>
  int main()
  {
    std::_Ios_Openmode mode = std::ios::in;
    (void)mode;
    return 0;
  }
" HAVE_IOS_OPENMODE)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/config.h"
  @ONLY
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/JazzPlusPlusVersion/JazzPlusPlusVersion.cpp
  COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/ShellScripts/CreateVersion.sh
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/JazzPlusPlusVersion/JazzPlusPlusVersion.tpl
    ${CMAKE_CURRENT_SOURCE_DIR}/JazzPlusPlusVersion/Versions.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/ShellScripts/CreateVersion.sh
  COMMENT "Creating JazzPlusPlusVersion.cpp from template"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/JazzPlusPlusVersion
  VERBATIM)

set_source_files_properties(
  ${CMAKE_CURRENT_SOURCE_DIR}/JazzPlusPlusVersion/JazzPlusPlusVersion.cpp
  PROPERTIES
    GENERATED TRUE)

set(
  JAZZ_SOURCES
    AboutDialog.cpp
    ArrayControl.cpp
    AsciiMidiFile.cpp
    Audio.cpp
    Command.cpp
    Configuration.cpp
    ControlEdit.cpp
    DeprecatedWx/prop.cpp
    DeprecatedWx/propform.cpp
    DeprecatedWx/proplist.cpp
    Dialogs/AudioSettingsDialog.cpp
    Dialogs/CleanupDialog.cpp
    Dialogs/ControllerDialog.cpp
    Dialogs/DeleteDialog.cpp
    Dialogs/EndOfTrackDialog.cpp
    Dialogs/FilterDialog.cpp
    Dialogs/IntegerEdit.cpp
    Dialogs/LengthDialog.cpp
    Dialogs/KeyOnDialog.cpp
    Dialogs/MeterChangeDialog.cpp
    Dialogs/MetronomeSettingsDialog.cpp
    Dialogs/MidiChannelDialog.cpp
    Dialogs/PitchWheelDialog.cpp
    Dialogs/ProgramChangeDialog.cpp
    Dialogs/QuantizeDialog.cpp
    Dialogs/SamplesDialog.cpp
    Dialogs/SearchAndReplaceDialog.cpp
    Dialogs/SetTempoDialog.cpp
    Dialogs/ShiftDialog.cpp
    Dialogs/SnapDialog.cpp
    Dialogs/SynthesizerSettingsDialog.cpp
    Dialogs/SysexDialog.cpp
    Dialogs/TextDialog.cpp
    Dialogs/TrackDialog.cpp
    Dialogs/TransposeDialog.cpp
    Dialogs/VelocityDialog.cpp
    Dialogs.cpp
    DrumUtilities.cpp
    ErrorMessage.cpp
    Events.cpp
    EventFrame.cpp
    EventWindow.cpp
    FileSelector.cpp
    Filter.cpp
    FindFile.cpp
    FrequencyTable.cpp
    GetOptionIndex.cpp
    Globals.cpp
    GuitarFrame.cpp
    GuitarSettings.cpp
    GuitarSettingsDialog.cpp
    GuitarWindow.cpp
    HarmonyBrowserAnalyzer.cpp
    Harmony.cpp
    HarmonyP.cpp
    Help.cpp
    JazzPlusPlusApplication.cpp
    JazzPlusPlusVersion/JazzPlusPlusVersion.cpp
    KeyStringConverters.cpp
    Knob.cpp
    Mapper.cpp
    MeasureChoice.cpp
    MidiDeviceDialog.cpp
    Metronome.cpp
    MouseAction.cpp
    NamedChoice.cpp
    NamedValue.cpp
    NamedValueChoice.cpp
    PianoFrame.cpp
    PianoWindow.cpp
    Player.cpp
    Project.cpp
    ProjectManager.cpp
    PropertyListDialog.cpp
    Random.cpp
    RecordingInfo.cpp
    Rectangle.cpp
    Rhythm.cpp
    RhythmArrayControl.cpp
    Sample.cpp
    SampleCommand.cpp
    SampleDialog.cpp
    SampleFrame.cpp
    SampleWindow.cpp
    SelectControllerDialog.cpp
    Signal2.cpp
    SliderWindow.cpp
    Song.cpp
    StandardFile.cpp
    StringReadWrite.cpp
    StringUtilities.cpp
    Synth.cpp
    SysexChannel.cpp
    ToolBar.cpp
    Track.cpp
    TrackFrame.cpp
    TrackWindow.cpp
)

# ALSA-specific sources (Linux)
if(ENABLE_ALSA)
  list(
    APPEND JAZZ_SOURCES
      AlsaDriver.cpp
      AlsaPlayer.cpp
      AlsaThru.cpp
  )
else()
  # Non-ALSA: use PortMidi player
  list(
    APPEND JAZZ_SOURCES
      PortMidiPlayer.cpp
  )
endif()

# ---------------------------------------------------------------------------
# Headers (for IDE support)
# ---------------------------------------------------------------------------
set(
  JAZZ_HEADERS
    AboutDialog.h
    AlsaDriver.h
    AlsaPlayer.h
    AlsaThru.h
    ArrayControl.h
    AsciiMidiFile.h
    Audio.h
    AudioDriver.h
    BitSet.h
    Command.h
    Configuration.h
    ControlEdit.h
    DeprecatedWx/prop.h
    DeprecatedWx/propform.h
    DeprecatedWx/proplist.h
    Dialogs/AudioSettingsDialog.h
    Dialogs/CleanupDialog.h
    Dialogs/ControllerDialog.h
    Dialogs/DeleteDialog.h
    Dialogs/EndOfTrackDialog.h
    Dialogs/FilterDialog.h
    Dialogs/IntegerEdit.h
    Dialogs/LengthDialog.h
    Dialogs/KeyOnDialog.h
    Dialogs/MeterChangeDialog.h
    Dialogs/MetronomeSettingsDialog.h
    Dialogs/MidiChannelDialog.h
    Dialogs/PitchWheelDialog.h
    Dialogs/ProgramChangeDialog.h
    Dialogs/QuantizeDialog.h
    Dialogs/SamplesDialog.h
    Dialogs/SearchAndReplaceDialog.h
    Dialogs/SetTempoDialog.h
    Dialogs/ShiftDialog.h
    Dialogs/SnapDialog.h
    Dialogs/SynthesizerSettingsDialog.h
    Dialogs/SysexDialog.h
    Dialogs/TextDialog.h
    Dialogs/TrackDialog.h
    Dialogs/TransposeDialog.h
    Dialogs/VelocityDialog.h
    Dialogs.h
    DrumEnums.h
    DrumUtilities.h
    DynamicArray.h
    ErrorMessage.h
    Events.h
    EventFrame.h
    EventWindow.h
    FileSelector.h
    Filter.h
    FindFile.h
    FrequencyTable.h
    GetOptionIndex.h
    Globals.h
    GuitarFrame.h
    GuitarSettingsDialog.h
    GuitarSettings.h
    GuitarWindow.h
    HarmonyBrowserAnalyzer.h
    Harmony.h
    HarmonyP.h
    Help.h
    JazzPlusPlusApplication.h
    JazzPlusPlusVersion/JazzPlusPlusVersion.h
    KeyStringConverters.h
    Mapper.h
    MeasureChoice.h
    Metronome.h
    MidiDeviceDialog.h
    midinet.h
    MouseAction.h
    NamedChoice.h
    NamedValue.h
    NamedValueChoice.h
    PianoFrame.h
    PianoWindow.h
    Player.h
    PortMidiPlayer.h
    Project.h
    ProjectManager.h
    PropertyListDialog.h
    Random.h
    RecordingInfo.h
    Rectangle.h
    Resources.h
    Rhythm.h
    RhythmArrayControl.h
    Sample.h
    SampleCommand.h
    SampleDialog.h
    SampleFrame.h
    SampleWindow.h
    SelectControllerDialog.h
    Signal2.h
    SignalInterface.h
    SliderWindow.h
    Song.h
    StandardFile.h
    StringReadWrite.h
    StringUtilities.h
    Synth.h
    SynthesizerTypeEnums.h
    SysexChannel.h
    ToolBar.h
    TrackFrame.h
    Track.h
    TrackWindow.h
)

# ---------------------------------------------------------------------------
# Include directories
# ---------------------------------------------------------------------------
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Bitmaps
  ${CMAKE_CURRENT_SOURCE_DIR}/Dialogs
  ${CMAKE_CURRENT_SOURCE_DIR}/DeprecatedWx
  ${wxWidgets_INCLUDE_DIRS}
)

if(ENABLE_ALSA)
  include_directories(${ALSA_INCLUDE_DIRS})
endif()

# ---------------------------------------------------------------------------
# Define JAZZ_DATADIR
# ---------------------------------------------------------------------------
add_definitions(-DJAZZ_DATADIR="${CMAKE_INSTALL_PREFIX}/share/Jazz++")

# ---------------------------------------------------------------------------
# Build the executable
# ---------------------------------------------------------------------------
add_executable(jazz ${JAZZ_SOURCES} ${JAZZ_HEADERS})

target_include_directories(
  jazz PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
)

# ---------------------------------------------------------------------------
# Link libraries
# ---------------------------------------------------------------------------
target_link_libraries(
  jazz
    ${wxWidgets_LIBRARIES}
    portmidi
)

if(ENABLE_ALSA)
  target_link_libraries(jazz ${ALSA_LIBRARIES})
endif()

if(APPLE)
  target_link_libraries(jazz ${COREMIDI_LIBRARY} ${COREAUDIO_LIBRARY})
endif()

# ---------------------------------------------------------------------------
# Help files
# ---------------------------------------------------------------------------
add_subdirectory(HelpFiles)

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------
install(TARGETS jazz DESTINATION bin)
