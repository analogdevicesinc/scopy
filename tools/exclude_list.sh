#!/bin/bash

# NOTE: paths containing * are treated as regex => escape them with \ (ex: \*)

OMIT_DIRS_LIST=(
    .\*
    .git\*
    .vscode
    android
    apple
    build
    ci/\*
    docs
    tmp
    tools
    windows
    # Directories specific to your local environment that are not part of the repository
    venv
)
OMIT_DIRS_LIST=("${OMIT_DIRS_LIST[@]/#/*\/}") # */ prefix to each element
GITIGNORE_DIRS_LIST=(
    # .gitinore
    CMakeFiles
    build\*
    deps
    iio-emu
    html/\*
    build_arm64-v8a/\*
    .cache/\*
    build/\*
    .vscode/\*
    windows/\*
    bin/\*
    lib/\*
    docs/_build/\*
    share/\*
    # ci/flatpak/.gitinore
    ci/flatpak/build
    ci/flatpak/repo
)
GITIGNORE_DIRS_LIST=("${GITIGNORE_DIRS_LIST[@]/#/*\/}") # */ prefix to each element

OMIT_FILES_LIST=(
    .clang-format
    .clangformatignore
    .cmake-format
    .gitignore
    .gitmodules
    LICENSE
    \*.md
    \*.png
    \*.rst
    azure-pipelines.yml
    requirements.txt
    \*.html
    \*.svg
    \*.icns
    \*.ico
    \*.qmodel
    \*.ui
    \*.json
    \*.qrc
    \*.ts
    \*.gif
    \*.theme
    \*.ttf
    \*.zip
    \*.csv
    \*.bin
    \*.xml
    \*.cmakein
    # Files specific to your local environment that are not part of the repository
    \*.build
    \*.git
    \*.gitrepo
    \*.mat
    \*.user
)
OMIT_FILES_LIST=("${OMIT_FILES_LIST[@]/#/*\/}")
GITINORE_FILES_LIST=(
    # .gitinore
    CMakeLists.txt.user
    CMakeCache.txt
    cmake_install.cmake
    Makefile
    moc_\*.cpp
    \*_automoc.cpp
    ui_\*.h
    resources/scopy_osp.html
    resources/stylesheets/default.qss
    resources/stylesheets/light.qss
    resources/credits.html
    resources/scopy_home.html
    resources/about.html
    \*qt.conf
    \*.swp
    \*.DS_Store
    android-build
    android\*.sh
    \*.apk
    \*.aab
    ci/general/gh-actions.envs
    core/include/scopy-core_config.h
    core/include/scopy-core_export.h
    core/include/core/scopy-core_config.h
    core/include/core/scopy-core_export.h
    common/include/common/scopy-common_config.h
    common/include/common/scopy-common_export.h
    gr-util/include/gr-util/scopy-gr-util_export.h
    gui/include/gui/scopy-gui_export.h
    gui/include/gui/scopy-gui_config.h
    iio-widgets/include/iio-widgets/scopy-iio-widgets_export.h
    iioutil/include/iioutil/scopy-iioutil_export.h
    pluginbase/include/pluginbase/scopy-pluginbase_config.h
    pluginbase/include/pluginbase/scopy-pluginbase_export.h
    # gui/res/.gitinore
    gui/res/about.html
    gui/res/scopy_osp.html
    gui/res/buildinfo.html
    gui/res/credits.html
    # ci/flatpak/.gitinore
    ci/flatpak/Scopy.flatpak
    ci/flatpak/.flatpak-builder
    ci/flatpak/org.adi.Scopy.json
    ci/flatpak/tmp.json
    # plugins/adc/.gitinore
    plugins/adc/include/adc/scopy-adc_export.h
    plugins/adc/include/adc/scopy-adc_config.h
    # plugins/dac/.gitinore
    plugins/dac/include/dac/scopy-dac_export.h
    plugins/dac/include/dac/scopy-dac_config.h
    # plugins/m2k/.gitinore
    plugins/m2k/include/m2k/scopy-m2k_export.h
    plugins/m2k/include/m2k/scopy-m2k_config.h
    # plugins/pqm/.gitinore
    plugins/pqm/include/pqm/scopy-pqm_export.h
    plugins/pqm/include/pqm/scopy-pqm_config.h
    # plugins/test/.gitinore
    plugins/test/include/test/scopy-test_export.h
    # plugins/swiot/.gitinore
    plugins/swiot/include/swiot/scopy-swiot_export.h
    plugins/swiot/include/swiot/scopy-swiot_config.h
    # plugins/test2/.gitinore
    plugins/test2/include/test2/scopy-test2_export.h
    # plugins/guitest/.gitinore
    plugins/guitest/include/guitest/scopy-guitest_export.h
    # plugins/debugger/.gitinore
    plugins/debugger/include/debugger/scopy-debugger_export.h
    plugins/debugger/include/debugger/scopy-debugger_config.h
    # plugins/datalogger/.gitinore
    plugins/datalogger/include/datalogger/scopy-datalogger_export.h
    plugins/datalogger/include/datalogger/scopy-datalogger_config.h
    # plugins/bareminimum/.gitinore
    plugins/bareminimum/include/bareminimum/scopy-bareminimum_export.h
    # plugins/m2k/m2k-gui/.gitinore
    plugins/m2k/m2k-gui/include/m2k-gui/scopy-m2k-gui_export.h
    plugins/m2k/m2k-gui/include/m2k-gui/scopy-m2k-gui_config.h
    plugins/m2k/m2k-gui/gr-gui/include/gr-gui/scopy-gr-gui_export.h
    plugins/m2k/m2k-gui/gr-gui/include/gr-gui/scopy-gr-gui_config.h
    plugins/m2k/m2k-gui/sigrok-gui/include/sigrok-gui/scopy-sigrok-gui_export.h
    plugins/m2k/m2k-gui/sigrok-gui/include/sigrok-gui/scopy-sigrok-gui_config.h
    # plugins/regmap/.gitinore
    plugins/regmap/include/regmap/scopy-regmap_export.h
    plugins/regmap/include/regmap/scopy-regmap_config.h
)
GITINORE_FILES_LIST=("${GITINORE_FILES_LIST[@]/#/*\/}")
