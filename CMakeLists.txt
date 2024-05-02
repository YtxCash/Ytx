cmake_minimum_required(VERSION 3.19)
project(YTX VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS LinguistTools Widgets Sql Gui)
qt_standard_project_setup(REQUIRES 6.5)

add_definitions(-DQT_DISABLE_DEPRECATED_UP_TO=0x060500)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fsanitize=address,undefined -fsanitize-recover=all")
elseif(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
elseif(LINUX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fsanitize=address,undefined,thread,memory -fsanitize-recover=all")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE PROJECT_SOURCES
    "*.h"
    "*.cc"
    "*.ui"
  )

set(brc "${CMAKE_CURRENT_SOURCE_DIR}/resource/resource.brc")

if(APPLE)
  set(icon_mac "${CMAKE_CURRENT_SOURCE_DIR}/resource/logo/logo.icns")
  set_source_files_properties(${icon_mac} ${brc} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources")

  qt6_add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${PROJECT_SOURCES} ${icon_mac} ${brc})

  set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
  )
elseif(WIN32)
  set(INFO_RC "${CMAKE_CURRENT_SOURCE_DIR}/resource/logo/Info.rc")
  qt6_add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES} ${INFO_RC} ${brc})
else()
  qt6_add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${brc})
endif()

# Projects mode -> Build & Run -> Build -> Build Steps -> Details -> Targets, check "update_translations"
qt6_add_translations(${PROJECT_NAME}
  TS_FILES resource/I18N/ytx_zh_CN.ts)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Sql Qt6::Gui)
