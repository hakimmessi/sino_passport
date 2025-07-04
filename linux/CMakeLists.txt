# Project-level configuration.
cmake_minimum_required(VERSION 3.13)
project(runner LANGUAGES CXX)

# Set standard globally
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The name of the executable created for the application. Change this to change
# the on-disk name of your application.
set(BINARY_NAME "sino_passport")
# The unique GTK application identifier for this application. See:
# https://wiki.gnome.org/HowDoI/ChooseApplicationID
set(APPLICATION_ID "com.example.sino_passport")

# Explicitly opt in to modern CMake behaviors to avoid warnings with recent
# versions of CMake.
cmake_policy(SET CMP0063 NEW)

# Load bundled libraries from the lib/ directory relative to the binary.
set(CMAKE_INSTALL_RPATH "$ORIGIN/lib")

# Root filesystem for cross-building.
if(FLUTTER_TARGET_PLATFORM_SYSROOT)
  set(CMAKE_SYSROOT ${FLUTTER_TARGET_PLATFORM_SYSROOT})
  set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})
  set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
  set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif()

# Define build configuration options.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Debug" CACHE
    STRING "Flutter build mode" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Profile" "Release")
endif()

function(APPLY_STANDARD_SETTINGS TARGET)
  target_compile_features(${TARGET} PUBLIC cxx_std_14)
  target_compile_options(${TARGET} PRIVATE -Wall -Werror)
  target_compile_options(${TARGET} PRIVATE "$<$<NOT:$<CONFIG:Debug>>:-O3>")
  target_compile_definitions(${TARGET} PRIVATE "$<$<NOT:$<CONFIG:Debug>>:NDEBUG>")
endfunction()

# Flutter library and tool build rules.
set(FLUTTER_MANAGED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/flutter")
add_subdirectory(${FLUTTER_MANAGED_DIR})

# System-level dependencies.
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED IMPORTED_TARGET gtk+-3.0)
pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)

# Application build; see runner/CMakeLists.txt.
add_subdirectory("runner")

# Run the Flutter tool portions of the build. This must not be removed.
add_dependencies(${BINARY_NAME} flutter_assemble)

# Only the install-generated bundle's copy of the executable will launch
# correctly, since the resources must in the right relative locations. To avoid
# people trying to run the unbundled copy, put it in a subdirectory instead of
# the default top-level location.
set_target_properties(${BINARY_NAME}
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/intermediates_do_not_run"
)


# Generated plugin build rules, which manage building the plugins and adding
# them to the application.
include(flutter/generated_plugins.cmake)


# === Installation ===
# By default, "installing" just makes a relocatable bundle in the build
# directory.
set(BUILD_BUNDLE_DIR "${PROJECT_BINARY_DIR}/bundle")
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${BUILD_BUNDLE_DIR}" CACHE PATH "..." FORCE)
endif()

# Start with a clean build bundle directory every time.
install(CODE "
  file(REMOVE_RECURSE \"${BUILD_BUNDLE_DIR}/\")
  " COMPONENT Runtime)

set(INSTALL_BUNDLE_DATA_DIR "${CMAKE_INSTALL_PREFIX}/data")
set(INSTALL_BUNDLE_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib")

install(TARGETS ${BINARY_NAME} RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}"
  COMPONENT Runtime)

install(FILES "${FLUTTER_ICU_DATA_FILE}" DESTINATION "${INSTALL_BUNDLE_DATA_DIR}"
  COMPONENT Runtime)

install(FILES "${FLUTTER_LIBRARY}" DESTINATION "${INSTALL_BUNDLE_LIB_DIR}"
  COMPONENT Runtime)

foreach(bundled_library ${PLUGIN_BUNDLED_LIBRARIES})
  install(FILES "${bundled_library}"
    DESTINATION "${INSTALL_BUNDLE_LIB_DIR}"
    COMPONENT Runtime)
endforeach(bundled_library)

# Copy the native assets provided by the build.dart from all packages.
set(NATIVE_ASSETS_DIR "${PROJECT_BUILD_DIR}native_assets/linux/")
install(DIRECTORY "${NATIVE_ASSETS_DIR}"
   DESTINATION "${INSTALL_BUNDLE_LIB_DIR}"
   COMPONENT Runtime)

# ARM-specific definitions
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
  add_compile_definitions(ARM_BUILD)
  message(STATUS "ARM build detected - CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
else()
  message(STATUS "Non-ARM build - CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Fully re-copy the assets directory on each build to avoid having stale files
# from a previous install.
set(FLUTTER_ASSET_DIR_NAME "flutter_assets")
install(CODE "
  file(REMOVE_RECURSE \"${INSTALL_BUNDLE_DATA_DIR}/${FLUTTER_ASSET_DIR_NAME}\")
  " COMPONENT Runtime)
install(DIRECTORY "${PROJECT_BUILD_DIR}/${FLUTTER_ASSET_DIR_NAME}"
  DESTINATION "${INSTALL_BUNDLE_DATA_DIR}" COMPONENT Runtime)

# Install the AOT library on non-Debug builds only.
if(NOT CMAKE_BUILD_TYPE MATCHES "Debug")
  install(FILES "${AOT_LIBRARY}" DESTINATION "${INSTALL_BUNDLE_LIB_DIR}"
    COMPONENT Runtime)
endif()

# === PNG Wrapper Configuration ===
message(STATUS "Configuring PNG Wrapper for libIDCard.so compatibility...")

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(PNG REQUIRED libpng)

# Add PNG wrapper sources to the binary
target_sources(${BINARY_NAME}
        PRIVATE
        src/sinosecu.cpp
        src/png_wrapper.cpp  # Add PNG wrapper
)

# Add PNG wrapper include directories
target_include_directories(${BINARY_NAME} PRIVATE
        ${PNG_INCLUDE_DIRS}
        src/sinosecu.cpp
        src/png_wrapper.cpp
)

# Link PNG wrapper dependencies
target_link_libraries(${BINARY_NAME} PRIVATE
        ${PNG_LIBRARIES}
        dl  # Required for dlopen/dlsym
)

# Add PNG wrapper compile definitions
target_compile_definitions(${BINARY_NAME} PRIVATE
        ${PNG_CFLAGS_OTHER}
        PNG_WRAPPER_ENABLED=1
)

# Ensure PNG wrapper has higher priority than static PNG in libIDCard.so
set_target_properties(${BINARY_NAME} PROPERTIES
        LINK_FLAGS "-Wl,--export-dynamic -Wl,--as-needed"
)

# === Sinosecu SDK Integration ===
set(SINOSEC_SDK_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")

# Check if the SDK directory exists
if(NOT EXISTS "${SINOSEC_SDK_LIB_DIR}")
  message(WARNING "Sinosecu SDK directory not found: ${SINOSEC_SDK_LIB_DIR}")
else()
  message(STATUS "Found Sinosecu SDK at: ${SINOSEC_SDK_LIB_DIR}")

  # List the SDK libraries
  set(SINOSEC_LIBS
          "${SINOSEC_SDK_LIB_DIR}/libIDCard.so"
          # Add other libraries as needed:
          "${SINOSEC_SDK_LIB_DIR}/libIDCardKernal.so"
          "${SINOSEC_SDK_LIB_DIR}/libCamDll.so"
  )

  # CRITICAL: Link SDK libraries AFTER PNG wrapper
  # This ensures PNG wrapper symbols override libIDCard.so symbols
  foreach(LIB ${SINOSEC_LIBS})
    if(EXISTS "${LIB}")
      target_link_libraries(${BINARY_NAME} PRIVATE "${LIB}")
      message(STATUS "Linking SDK library: ${LIB}")
    else()
      message(WARNING "Library not found: ${LIB}")
    endif()
  endforeach()

  # Install SDK libraries to bundle lib directory
  install(DIRECTORY "${SINOSEC_SDK_LIB_DIR}/"
          DESTINATION "${INSTALL_BUNDLE_LIB_DIR}"
          FILES_MATCHING
          PATTERN "*.so*"
          PATTERN "*.dat"
          PATTERN "*.xml"
          PATTERN "*.ini"
          PATTERN "*.txt"
  )
endif()
