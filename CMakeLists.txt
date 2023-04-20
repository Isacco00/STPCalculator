cmake_minimum_required (VERSION 3.0.0 FATAL_ERROR)

# Project name
project (STPCalculator CXX)

# OpenCascade
find_package(OpenCASCADE)

# Configure C++ compiler's includes dir
include_directories ( SYSTEM ${OpenCASCADE_INCLUDE_DIR} )

find_package(nlohmann_json 3.7.0 REQUIRED)

# Add executable
add_executable (STPCalculator
  CommonImport.cpp
  CommonImport.h
  Component.h
  Component.cpp
  IShape.cpp
  IShape.h
  Mesh.cpp
  Mesh.h
  Model.cpp
  Model.h
  NumTool.h
  OCCLib.h
  OCCUtil.cpp
  OCCUtil.h
  InputOptions.cpp
  InputOptions.h
  StopWatch.cpp
  StopWatch.h
  StepCalculator.cpp
  StepReader.cpp
  StepReader.h
  StrTool.h
  Tessellator.cpp
  Tessellator.h
  X3D_Writer.cpp
  X3D_Writer.h
  JsonWriter.cpp
  JsonWriter.h
  Json.hpp
)

# Add linker options
foreach (LIB ${OpenCASCADE_LIBRARIES})
  target_link_libraries(STPCalculator debug ${OpenCASCADE_LIBRARY_DIR}d/${LIB}.lib)
  target_link_libraries(STPCalculator optimized ${OpenCASCADE_LIBRARY_DIR}/${LIB}.lib)
endforeach()

target_link_libraries(STPCalculator debug nlohmann_json::nlohmann_json)
target_link_libraries(STPCalculator optimized nlohmann_json::nlohmann_json)

set_property(TARGET STPCalculator PROPERTY VS_DEBUGGER_ENVIRONMENT "PATH=$<$<CONFIG:DEBUG>:${OpenCASCADE_BINARY_DIR}d>$<$<NOT:$<CONFIG:DEBUG>>:${OpenCASCADE_BINARY_DIR}>;%PATH%")

target_compile_features(STPCalculator PRIVATE cxx_std_17)

add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
add_compile_definitions(_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
