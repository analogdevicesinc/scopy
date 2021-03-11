# * try to find Qwt libraries and include files QWT_INCLUDE_DIR where to find
#   qwt_global.h, etc. QWT_LIBRARIES libraries to link against QWT_FOUND If
#   false, do not try to use Qwt qwt_global.h holds a string with the QWT
#   version; test to make sure it's at least 5.2

find_path(
    QWT_INCLUDE_DIRS
    NAMES qwt_global.h
    HINTS ${CMAKE_INSTALL_PREFIX}/include/qwt
    PATHS /usr/local/include/qwt-qt5
          /usr/local/include/qwt
          /usr/include/qwt6
          /usr/include/qwt-qt5
          /usr/include/qwt
          /usr/include/qwt5
          /opt/local/include/qwt
          /sw/include/qwt
          /usr/local/lib/qwt.framework/Headers
)

find_library(
    QWT_LIBRARIES
    NAMES qwt6 qwt6-qt5 qwt qwt-qt5
    HINTS ${CMAKE_INSTALL_PREFIX}/lib ${CMAKE_INSTALL_PREFIX}/lib64
    PATHS /usr/local/lib
          /usr/lib
          /opt/local/lib
          /sw/lib
          /usr/local/lib/qwt.framework
)

set(QWT_FOUND FALSE)
if(QWT_INCLUDE_DIRS)
    file(STRINGS "${QWT_INCLUDE_DIRS}/qwt_global.h" QWT_STRING_VERSION
         REGEX "QWT_VERSION_STR"
    )
    set(QWT_WRONG_VERSION True)
    set(QWT_VERSION "No Version")
    string(REGEX MATCH "[0-9]+.[0-9]+.[0-9]+" QWT_VERSION ${QWT_STRING_VERSION})
    string(COMPARE LESS ${QWT_VERSION} "5.2.0" QWT_WRONG_VERSION)

    message(STATUS "QWT Version: ${QWT_VERSION}")
    if(NOT QWT_WRONG_VERSION)
        set(QWT_FOUND TRUE)
    else()
        message(STATUS "QWT Version must be >= 5.2, Found ${QWT_VERSION}")
    endif()

endif()

find_library(QWTPOLAR_LIBRARIES qwtpolar)
find_path(QWTPOLAR_INCLUDE_DIRS qwt_polar_plot.h PATH_SUFFIXES qwt)

add_library(qwt INTERFACE)
target_link_libraries(qwt INTERFACE ${QWT_LIBRARIES})
target_include_directories(qwt INTERFACE ${QWT_INCLUDE_DIRS})

if(QWTPOLAR_LIBRARIES)
    target_link_libraries(qwt INTERFACE ${QWTPOLAR_LIBRARIES})
else()
    message(
        STATUS "QwtPolar libraries not found - assuming they are built in Qwt"
    )
endif()

if(NOT QWTPOLAR_INCLUDE_DIRS)
    message(SEND_ERROR "QwtPolar includes not found")
else()
    target_include_directories(qwt INTERFACE ${QWTPOLAR_INCLUDE_DIRS})
endif()

add_library(qwt::qwt ALIAS qwt)
