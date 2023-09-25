if(DEFINED __INCLUDED_SCOPY_MACOS_CMAKE)
	return()
endif()
set(__INCLUDED_SCOPY_MACOS_CMAKE TRUE)

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

install(
	CODE "
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

set(EXTRA_BUNDLE_FILES
    ${EXTRA_BUNDLE_FILES}
    ${QT_PLUGINS}
    ${ICON_FILE}
    ${PKGINFO}
    ${QT_CONF}
)
