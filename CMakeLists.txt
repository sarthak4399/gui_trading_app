cmake_minimum_required(VERSION 3.16)
project(HFTTradingApp VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)

# Try SFML 3.x first, then fall back to manual linking
find_package(SFML 3 QUIET COMPONENTS System Window Graphics Network Audio)
if(NOT SFML_FOUND)
    find_package(SFML 2.5 QUIET COMPONENTS system window graphics network audio)
endif()

if(NOT SFML_FOUND)
    message(STATUS "SFML not found via find_package, trying manual detection")
    find_library(SFML_SYSTEM_LIB sfml-system PATHS /opt/homebrew/lib)
    find_library(SFML_WINDOW_LIB sfml-window PATHS /opt/homebrew/lib)
    find_library(SFML_GRAPHICS_LIB sfml-graphics PATHS /opt/homebrew/lib)
    find_library(SFML_NETWORK_LIB sfml-network PATHS /opt/homebrew/lib)
    
    if(SFML_SYSTEM_LIB AND SFML_WINDOW_LIB AND SFML_GRAPHICS_LIB AND SFML_NETWORK_LIB)
        set(SFML_LIBRARIES ${SFML_SYSTEM_LIB} ${SFML_WINDOW_LIB} ${SFML_GRAPHICS_LIB} ${SFML_NETWORK_LIB})
        set(SFML_FOUND TRUE)
        message(STATUS "Found SFML libraries manually")
    else()
        message(FATAL_ERROR "SFML libraries not found")
    endif()
endif()

# Try to find TGUI
find_package(TGUI QUIET)
if(NOT TGUI_FOUND)
    message(STATUS "TGUI not found via find_package, trying pkg-config")
    pkg_check_modules(TGUI QUIET tgui)
    if(NOT TGUI_FOUND)
        message(STATUS "TGUI not found, will use manual linking")
    endif()
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/gui_app.cpp
    src/data_fetcher.cpp
    src/strategy_engine.cpp
    src/history_manager.cpp
    src/market_data.cpp
    src/trading_signal.cpp
    src/yahoo_finance_api.cpp
    src/technical_indicators.cpp
)

# Header files
set(HEADERS
    include/gui_app.h
    include/data_fetcher.h
    include/strategy_engine.h
    include/history_manager.h
    include/market_data.h
    include/trading_signal.h
    include/yahoo_finance_api.h
    include/technical_indicators.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
if(TARGET sfml-system)
    # SFML 3.x style with targets
    target_link_libraries(${PROJECT_NAME} 
        sfml-system 
        sfml-window 
        sfml-graphics 
        sfml-network
    )
elseif(SFML_LIBRARIES)
    # Manual SFML libraries
    target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE /opt/homebrew/include)
else()
    # Fallback to individual library names
    target_link_libraries(${PROJECT_NAME} 
        sfml-system 
        sfml-window 
        sfml-graphics 
        sfml-network
    )
endif()

# Try to link TGUI
if(TGUI_FOUND)
    if(TARGET tgui)
        target_link_libraries(${PROJECT_NAME} tgui)
    else()
        target_link_libraries(${PROJECT_NAME} ${TGUI_LIBRARIES})
        target_include_directories(${PROJECT_NAME} PRIVATE ${TGUI_INCLUDE_DIRS})
    endif()
else()
    # Manual TGUI linking (adjust path as needed)
    find_library(TGUI_LIB tgui PATHS /opt/homebrew/lib)
    if(TGUI_LIB)
        target_link_libraries(${PROJECT_NAME} ${TGUI_LIB})
        target_include_directories(${PROJECT_NAME} PRIVATE /opt/homebrew/include)
    else()
        message(WARNING "TGUI library not found, build may fail")
    endif()
endif()

# For HTTP requests - try different options
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(CURL QUIET libcurl)
    if(CURL_FOUND)
        target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES})
        target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIRS})
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_CURL)
    endif()
endif()

# Fallback to system curl
if(NOT CURL_FOUND)
    find_library(CURL_LIB curl)
    if(CURL_LIB)
        target_link_libraries(${PROJECT_NAME} ${CURL_LIB})
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_CURL)
    endif()
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -O2)
endif()

# Copy resources to build directory
file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})

# Install target
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY resources DESTINATION share/${PROJECT_NAME})
install(DIRECTORY data DESTINATION share/${PROJECT_NAME})
