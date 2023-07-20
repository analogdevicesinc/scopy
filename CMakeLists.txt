cmake_minimum_required(VERSION 3.9)
enable_testing()

project(scopy VERSION 2.0.0 LANGUAGES CXX)
set(SCOPY_VERSION ${PROJECT_VERSION})

# Make sure our local CMake Modules path comes first
list(INSERT CMAKE_MODULE_PATH 0 ${PROJECT_SOURCE_DIR}/cmake/Modules)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

if (Qt5Widgets_VERSION VERSION_LESS 5.15.2)
	message(FATAL_ERROR "Minimum supported Qt 5.15.2")
	return()
else()
	message(STATUS "Using Qt version: " ${Qt5Widgets_VERSION})
endif()

#message(QtVersion: ${QT_VERSION_MAJOR}:${QT_VERSION_MINOR})


FILE(GLOB SRC_LIST  *.cpp *.cc)
FILE(GLOB HEADER_LIST *.h *.hpp)
FILE(GLOB UI_LIST *.ui)

set(PROJECT_SOURCES
        ${SRC_LIST}
        ${HEADER_LIST}
        ${UI_LIST}
)

include(ScopyAbout)
configure_about(./resources/about)
FILE(GLOB SCOPY_RESOURCE_FILES gui/res/resources.qrc resources/aboutpage.qrc)
message(STATUS ${SCOPY_RESOURCE_FILES})
qt_add_resources(SCOPY_RESOURCES ${SCOPY_RESOURCE_FILES})

find_path(
	IIO_INCLUDE_DIRS
	NAMES iio.h
	HINTS  ${CMAKE_INSTALL_PREFIX}/include /include /usr/include /usr/local/include /opt/local/include
	REQUIRED)

find_library(
	IIO_LIBRARIES
	NAMES iio libiio
	HINTS  ${CMAKE_INSTALL_PREFIX}/lib /usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64 /opt/local/lib /opt/local/lib64
	REQUIRED)
message("IIO LIBRARIES: " ${IIO_LIBRARIES})

option(ENABLE_TESTING "Enable unit tests" ON)
if(ENABLE_TESTING)
    message(STATUS "Unit tests enabled")
    add_subdirectory(tests)
endif()

option(ENABLE_APPLICATION_BUNDLE "Enable application bundle for OSX" OFF)
if (APPLE)
	set(ENABLE_APPLICATION_BUNDLE ON)
endif(APPLE)

set(SCOPY_DLL_PATH ${CMAKE_INSTALL_LIBDIR})
set(SCOPY_PLUGIN_BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/plugins/plugins)
set(SCOPY_PLUGIN_INSTALL_PATH ${CMAKE_INSTALL_FULL_DATADIR}/scopy-plugins)

add_subdirectory(common)
add_subdirectory(gui)
add_subdirectory(iioutil)
add_subdirectory(pluginbase)
add_subdirectory(plugins)
add_subdirectory(core)

set(SCOPY_DEPENDENCIES ${IIO_LIBRARIES})

if (ENABLE_APPLICATION_BUNDLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

        set(PKGINFO ${CMAKE_BINARY_DIR}/PkgInfo)
        file(WRITE ${PKGINFO} "APPLScopy")
        set_source_files_properties(${PKGINFO} PROPERTIES MACOSX_PACKAGE_LOCATION .)

        set(QT_CONF ${CMAKE_BINARY_DIR}/qt.conf)
        file(APPEND ${QT_CONF} "")
        set_source_files_properties(${QT_CONF} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

        set(ICON_FILE ${CMAKE_SOURCE_DIR}/gui/res/Scopy.icns)
        set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

        set(CMAKE_EXE_LINKER_FLAGS "-Wl,-headerpad_max_install_names -Wl,-search_paths_first ${CMAKE_EXE_LINKER_FLAGS}")

        foreach(plugin ${Qt5Gui_PLUGINS} ${Qt5Svg_PLUGINS})
                get_target_property(_loc ${plugin} LOCATION)
                get_filename_component(_name ${_loc} NAME)
                get_filename_component(_dir ${_loc} DIRECTORY)
                get_filename_component(_dir ${_dir} NAME)

                set_source_files_properties(${_loc} PROPERTIES MACOSX_PACKAGE_LOCATION plugins/${_dir})
                set(QT_PLUGINS ${QT_PLUGINS} ${_loc})
                set(BUNDLED_QT_PLUGINS ${BUNDLED_QT_PLUGINS} ${CMAKE_BINARY_DIR}/Scopy.app/Contents/plugins/${_dir}/${_name})
        endforeach()

        install(CODE "
                set(BU_CHMOD_BUNDLE_ITEMS ON)
                include(BundleUtilities)
                fixup_bundle(\"${CMAKE_BINARY_DIR}/Scopy.app\" \"${BUNDLED_QT_PLUGINS}\" \"${CMAKE_SOURCE_DIR}\")"
        )

        set(OSX_BUNDLE MACOSX_BUNDLE)

        find_package(PkgConfig)
        set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
        pkg_check_modules(GLIB REQUIRED glib-2.0)
        pkg_check_modules(LIBSIGROK_DECODE REQUIRED libsigrokdecode)
        pkg_get_variable(LIBSIGROK_DECODERS_DIR libsigrokdecode decodersdir)
        file(GLOB_RECURSE DECODERS ${LIBSIGROK_DECODERS_DIR}/*.py)
        foreach(_decoder ${DECODERS})
                file(RELATIVE_PATH _file ${LIBSIGROK_DECODERS_DIR} ${_decoder})
                get_filename_component(_path ${_file} DIRECTORY)
                set_property(SOURCE ${_decoder} PROPERTY MACOSX_PACKAGE_LOCATION MacOS/decoders/${_path})
                set(EXTRA_BUNDLE_FILES ${EXTRA_BUNDLE_FILES} ${_decoder})
        endforeach()

        set(EXTRA_BUNDLE_FILES ${EXTRA_BUNDLE_FILES} ${QT_PLUGINS} ${ICON_FILE} ${PKGINFO} ${QT_CONF})
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
	${PROJECT_SOURCES}
	${SCOPY_RESOURCES}
	${SCOPY_DEPENDENCIES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET tool_launcher APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
	add_library(${PROJECT_NAME} SHARED
            ${PROJECT_SOURCES}
            ${SCOPY_RESOURCES}
            ${SCOPY_DEPENDENCIES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(${PROJECT_NAME} WIN32 ${OSX_BUNDLE}
            ${PROJECT_SOURCES}
            ${SCOPY_RESOURCES}
            ${SCOPY_DEPENDENCIES}
            ${EXTRA_BUNDLE_FILES}
        )
    endif()
endif()

list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_constexpr OUT_CONSTEXPR)
add_definitions(-DQT_NO_KEYWORDS)

set(CMAKE_VERBOSE_MAKEFILE ON)
target_include_directories(${PROJECT_NAME} PUBLIC scopy-gui scopy-core)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${SCOPY_DEPENDENCIES} scopy-core scopy-gui)

# Compiler options
target_compile_options(${PROJECT_NAME} PUBLIC -Wall)

#List of warnings to be treated as errors
target_compile_options(${PROJECT_NAME} PUBLIC
        -Werror=return-type
        -Werror=uninitialized
        -Werror=init-self
        -Werror=switch
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/apple/Info.plist.cmakein ${CMAKE_CURRENT_BINARY_DIR}/Info.plist COPYONLY)
set_target_properties(${PROJECT_NAME}  PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
    WIN32_EXECUTABLE TRUE
)

if (ENABLE_APPLICATION_BUNDLE)
	set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME Scopy)
endif()

configure_file(windows/scopy.iss.cmakein ${CMAKE_CURRENT_BINARY_DIR}/windows/scopy.iss @ONLY)
configure_file(windows/scopy-32.iss.cmakein ${CMAKE_CURRENT_BINARY_DIR}/windows/scopy-32.iss @ONLY)
configure_file(windows/scopy-64.iss.cmakein ${CMAKE_CURRENT_BINARY_DIR}/windows/scopy-64.iss @ONLY)
configure_file(windows/qt.conf.cmakein ${CMAKE_CURRENT_BINARY_DIR}/qt.conf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/scopy.desktop.cmakein ${CMAKE_CURRENT_BINARY_DIR}/scopy.desktop @ONLY)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/scopy.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gui/res/icon_small.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps RENAME scopy.svg)
endif()
if (NOT ENABLE_APPLICATION_BUNDLE)
    install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION .)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()
