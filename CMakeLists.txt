cmake_minimum_required(VERSION 3.16)
project(triage_gui CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOUIC OFF)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(triage_gui
    main.cpp
    Patient.hpp
    PriorityQueue.hpp
    TriageAlgorithm.hpp
)

target_link_libraries(triage_gui Qt6::Widgets)

set(QT_BIN "C:/Qt/6.10.0/msvc2022_64/bin")

if (MSVC AND WIN32)
  add_custom_command(TARGET triage_gui POST_BUILD
    COMMAND "${QT_BIN}/windeployqt.exe" --release --compiler-runtime "$<TARGET_FILE:triage_gui>"
    COMMENT "Running windeployqt to bundle Qt and MSVC runtime")
endif()
