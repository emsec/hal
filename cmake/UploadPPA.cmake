## -*- mode:cmake; coding:utf-8; -*-
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
# Changes Copyright (c) 2011 2012 Rüdiger Sonderfeld <ruediger@c-plusplus.de>
#
# UploadPPA.cmake is free software. It comes without any warranty,
# to the extent permitted by applicable law. You can redistribute it
# and/or modify it under the terms of the Do What The Fuck You Want
# To Public License, Version 2, as published by Sam Hocevar. See
# http://sam.zoy.org/wtfpl/COPYING for more details.
#
##
# Documentation
#
# This CMake module uploads a project to a PPA.  It creates all the files
# necessary (similar to CPack) and uses debuild(1) and dput(1) to create the
# package and upload it to a PPA.  A PPA is a Personal Package Archive and can
# be used by Debian/Ubuntu or other apt/deb based distributions to install and
# update packages from a remote repository.
# Canonicals Launchpad (http://launchpad.net) is usually used to host PPAs.
# See https://help.launchpad.net/Packaging/PPA for further information
# about PPAs.
#
# UploadPPA.cmake uses similar settings to CPack and the CPack DEB Generator.
# Additionally the following variables are used
#
# CPACK_DEBIAN_PACKAGE_BUILD_DEPENDS to specify build dependencies
# (cmake is added as default)
# CPACK_DEBIAN_RESOURCE_FILE_CHANGELOG should point to a file containing the
# changelog in debian format.  If not set it checks whether a file
# debian/changelog exists in the source directory or creates a simply changelog
# file.
# CPACK_DEBIAN_UPDATE_CHANGELOG if set to True then UploadPPA.cmake adds a new
# entry to the changelog with the current version number and distribution name
# (lsb_release -c is used).  This can be useful because debuild uses the latest
# version number from the changelog and the version number set in
# CPACK_PACKAGE_VERSION.  If they mismatch the creation of the package fails.
#
## A.Hoarau : CHANGELOG_MESSAGE can be used to pass a custom changelog message
# Check packages
#
# ./configure -DENABLE_PPA=On
# make dput
# cd build/Debian/${DISTRI}
# dpkg-source -x vobsub2srt_1.0pre4-ppa1.dsc
# cd vobsub2srt-1.0pre4/
# debuild -i -us -uc -sa -b
#
# Check the lintian warnings!
#
# From the resulting dsc file in the Debian directory also a Ubuntu/Debian package can be build:
#
# dpkg-source -x foo.dsc
# cd <CPACK_DEBIAN_PACKAGE_NAME>
# fakeroot debian/rules binary
#
# When uploading it to Launchpad with dput you must be registered there and you must have a .dput.cf in place. Follow the instructions on https://help.launchpad.net/Packaging/PPA/Uploading
#
##
# TODO
# I plan to add support for git dch (from git-buildpackage) to auto generate
# the changelog.
##

find_program(DEBUILD_EXECUTABLE debuild)
find_program(DPUT_EXECUTABLE dput)
find_program(DEBSIGN_EXECUTABLE debsign)

if(NOT DEBUILD_EXECUTABLE OR NOT DPUT_EXECUTABLE)
    message(WARNING "Debuild or dput not installed, please run sudo apt-get install devscripts")
    return()
endif(NOT DEBUILD_EXECUTABLE OR NOT DPUT_EXECUTABLE)


if(NOT PROJECT_PPA_DISTRIB_TARGET)
    execute_process(
            COMMAND lsb_release -cs
            OUTPUT_VARIABLE DISTRI
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(PROJECT_PPA_DISTRIB_TARGET ${DISTRI})
    message(STATUS "PROJECT_PPA_DISTRIB_TARGET NOT provided, so using system settings : ${DISTRI}")
endif()

foreach(DISTRI ${PROJECT_PPA_DISTRIB_TARGET})
    message(STATUS "Building for ${DISTRI}")

    # Strip "-dirty" flag from package version.
    # It can be added by, e.g., git describe but it causes trouble with debuild etc.
    message(STATUS "version-abc: ${CPACK_PACKAGE_VERSION}")
    string(REPLACE "-dirty" "" CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
    message(STATUS "version: ${CPACK_PACKAGE_VERSION}")

    # DEBIAN/control
    # debian policy enforce lower case for package name
    # Package: (mandatory)
    if(NOT CPACK_DEBIAN_PACKAGE_NAME)
        string(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_DEBIAN_PACKAGE_NAME)
    endif(NOT CPACK_DEBIAN_PACKAGE_NAME)

    # Section: (recommended)
    if(NOT CPACK_DEBIAN_PACKAGE_SECTION)
        set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
    endif(NOT CPACK_DEBIAN_PACKAGE_SECTION)

    # Priority: (recommended)
    if(NOT CPACK_DEBIAN_PACKAGE_PRIORITY)
        set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    endif(NOT CPACK_DEBIAN_PACKAGE_PRIORITY)

    if(NOT CPACK_DEBIAN_PACKAGE_MAINTAINER)
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_CONTACT})
    endif()

    if(NOT CPACK_PACKAGE_DESCRIPTION AND EXISTS ${CPACK_PACKAGE_DESCRIPTION_FILE})
        file(STRINGS ${CPACK_PACKAGE_DESCRIPTION_FILE} DESC_LINES)
        foreach(LINE ${DESC_LINES})
            set(deb_long_description "${deb_long_description} ${LINE}\n")
        endforeach(LINE ${DESC_LINES})
    else()
        # add space before each line
        string(REPLACE "\n" "\n " deb_long_description " ${CPACK_PACKAGE_DESCRIPTION}")
    endif()

    if(PPA_DEBIAN_VERSION)
        set(DEBIAN_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}-${PPA_DEBIAN_VERSION}~${DISTRI}1")
    elseif(NOT PPA_DEBIAN_VERSION AND NOT PROJECT_PPA_DISTRIB_TARGET)
        message(WARNING "Variable PPA_DEBIAN_VERSION not set! Building 'native' package!")
        set(DEBIAN_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}")
    else()
        set(DEBIAN_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}~${DISTRI}1")
    endif()

    message(STATUS "Debian version: ${DEBIAN_PACKAGE_VERSION}")

    set(DEBIAN_SOURCE_DIR ${CMAKE_BINARY_DIR}/Debian/${DISTRI}/${CPACK_DEBIAN_PACKAGE_NAME}_${DEBIAN_PACKAGE_VERSION})

    ##############################################################################
    # debian/control

    set(debian_control ${DEBIAN_SOURCE_DIR}/debian/control)
    list(APPEND CPACK_DEBIAN_PACKAGE_BUILD_DEPENDS "cmake" "debhelper (>= 7.0.50)")
    list(REMOVE_DUPLICATES CPACK_DEBIAN_PACKAGE_BUILD_DEPENDS)
    list(SORT CPACK_DEBIAN_PACKAGE_BUILD_DEPENDS)
    string(REPLACE ";" ", " build_depends "${CPACK_DEBIAN_PACKAGE_BUILD_DEPENDS}")
    string(REPLACE ";" ", " bin_depends "${CPACK_DEBIAN_PACKAGE_DEPENDS}")
    file(WRITE ${debian_control}
         "Source: ${CPACK_DEBIAN_PACKAGE_NAME}\n"
         "Section: ${CPACK_DEBIAN_PACKAGE_SECTION}\n"
         "Priority: ${CPACK_DEBIAN_PACKAGE_PRIORITY}\n"
         "Maintainer: ${CPACK_DEBIAN_PACKAGE_MAINTAINER}\n"
         "Build-Depends: ${build_depends}\n"
         "Standards-Version: 3.9.7\n"
         "Homepage: ${CPACK_DEBIAN_PACKAGE_HOMEPAGE}\n"
         "\n"
         "Package: ${CPACK_DEBIAN_PACKAGE_NAME}\n"
         "Architecture: ${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}\n"
         "Depends: ${bin_depends}, \${shlibs:Depends}, \${misc:Depends}\n"
         "Description: ${CPACK_PACKAGE_DESCRIPTION_SUMMARY}\n"
         "${deb_long_description}"
         )

    file(APPEND ${debian_control}
         "\n\n"
         "Package: ${CPACK_DEBIAN_PACKAGE_NAME}-dbg\n"
         "Priority: extra\n"
         "Section: debug\n"
         "Architecture: ${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}\n"
         "Depends: ${CPACK_DEBIAN_PACKAGE_NAME} (= \${binary:Version}), \${shlibs:Depends}, \${misc:Depends}\n"
         "Description: ${CPACK_PACKAGE_DESCRIPTION_SUMMARY}\n"
         "${deb_long_description}"
         "\n .\n"
         " This is the debugging symbols for the ${CPACK_DEBIAN_PACKAGE_NAME} library"
         )

    foreach(COMPONENT ${CPACK_COMPONENTS_ALL})
        string(TOUPPER ${COMPONENT} UPPER_COMPONENT)
        set(DEPENDS "${CPACK_DEBIAN_PACKAGE_NAME}")
        foreach(DEP ${CPACK_COMPONENT_${UPPER_COMPONENT}_DEPENDS})
            set(DEPENDS "${DEPENDS}, ${CPACK_DEBIAN_PACKAGE_NAME}-${DEP}")
        endforeach(DEP ${CPACK_COMPONENT_${UPPER_COMPONENT}_DEPENDS})
        file(APPEND ${debian_control} "\n"
             "Package: ${CPACK_DEBIAN_PACKAGE_NAME}-${COMPONENT}\n"
             "Architecture: ${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}\n"
             "Depends: ${DEPENDS}\n"
             "Description: ${CPACK_PACKAGE_DESCRIPTION_SUMMARY}"
             ": ${CPACK_COMPONENT_${UPPER_COMPONENT}_DISPLAY_NAME}\n"
             "${deb_long_description}"
             "\n .\n"
             " ${CPACK_COMPONENT_${UPPER_COMPONENT}_DESCRIPTION}\n"
             )
    endforeach(COMPONENT ${CPACK_COMPONENTS_ALL})


    ##############################################################################
    # debian/copyright
    set(debian_copyright ${DEBIAN_SOURCE_DIR}/debian/copyright)
    configure_file(${CPACK_RESOURCE_FILE_LICENSE} ${debian_copyright} COPYONLY)

    ##############################################################################
    # debian/rules
    set(debian_rules ${DEBIAN_SOURCE_DIR}/debian/rules)

    file(WRITE ${debian_rules}
         "#!/usr/bin/make -f\n"
         "\nexport DH_VERBOSE=1"
         "\n\n%:\n"
         "\tdh  $@ --buildsystem=cmake --parallel\n"
         "\noverride_dh_auto_configure:\n"
         "\tls -lah\n"
         "\tDESTDIR=\"$(CURDIR)/debian/${CPACK_DEBIAN_PACKAGE_NAME}\" dh_auto_configure -- -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_INSTALL_LDCONFIG=OFF -DBUILD_ALL_PLUGINS=ON -DBUILD_TESTS=OFF -DWITH_GUI=ON -DUSE_VERSION_FROM_OPTION=ON -DDHAL_VERSION_RETURN=\"${DHAL_VERSION_RETURN}\" -DHAL_VERSION_MAJOR=\"${HAL_VERSION_MAJOR}\" -DHAL_VERSION_MINOR=\"${HAL_VERSION_MINOR}\" -DHAL_VERSION_PATCH=\"${HAL_VERSION_PATCH}\" -DHAL_VERSION_TWEAK=\"${HAL_VERSION_TWEAK}\" -DHAL_VERSION_ADDITIONAL_COMMITS=\"${HAL_VERSION_ADDITIONAL_COMMITS}\" -DHAL_VERSION_HASH=\"${HAL_VERSION_HASH}\" -DHAL_VERSION_DIRTY=\"${HAL_VERSION_DIRTY}\" -DHAL_VERSION_BROKEN=\"${HAL_VERSION_BROKEN}\"\n"
         "\n\noverride_dh_auto_install:\n"
         "\tdh_auto_install --destdir=\"$(CURDIR)/debian/${CPACK_DEBIAN_PACKAGE_NAME}\" --buildsystem=cmake"
         "\n\noverride_dh_strip:\n"
         "\tdh_strip --dbg-package=${CPACK_DEBIAN_PACKAGE_NAME}-dbg"
         )

    execute_process(COMMAND chmod +x ${debian_rules})

    ##############################################################################
    # debian/compat
    file(WRITE ${DEBIAN_SOURCE_DIR}/debian/compat "9")

    ##############################################################################
    # debian/source/format
    file(WRITE ${DEBIAN_SOURCE_DIR}/debian/source/format "3.0 (native)")

    ##############################################################################

    # debian/changelog
    set(debian_changelog ${DEBIAN_SOURCE_DIR}/debian/changelog)

    execute_process(
        COMMAND ${CMAKE_SOURCE_DIR}/tools/changelog_converter.py -i CHANGELOG.md --to debian -o ${debian_changelog} --for-ppa-debian-dir --ppa-version ${PPA_DEBIAN_VERSION} -r ${DISTRI}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULTS_VARIABLE OUTPUT_CHANGELOG
    )


    ##########################################################################
    # Templates

    if(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA)
        foreach(CF ${CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA})
            get_filename_component(CF_NAME ${CF} NAME)
            message("Writing debian/${CF_NAME}")
            configure_file(${CF} ${DEBIAN_SOURCE_DIR}/debian/${CF_NAME} @ONLY)
        endforeach()
    endif(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA)


    ##########################################################################
    # .orig.tar.gz
    #execute_process(COMMAND date +%y%m%d
    #  OUTPUT_VARIABLE day_suffix
    #  OUTPUT_STRIP_TRAILING_WHITESPACE
    #  )

    message(STATUS "CPACK_SOURCE_IGNORE_FILES: ${CPACK_SOURCE_IGNORE_FILES}")
    set(CPACK_SOURCE_IGNORE_FILES
        ${CPACK_SOURCE_IGNORE_FILES}
        "/.*/.*build.*/"
        "/Testing/"
        "/tmp/"
        "/debian/"
        "/\\\\.git.*"
        "/\\\\.idea/"
        "/\\\\.codelite/"
        "*~$")

    #set(package_file_name "${CPACK_DEBIAN_PACKAGE_NAME}_${DEBIAN_PACKAGE_VERSION}")
    set(package_file_name "${CPACK_DEBIAN_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}")

    file(WRITE "${CMAKE_BINARY_DIR}/Debian/${DISTRI}/cpack.cmake"
         "set(CPACK_GENERATOR TGZ)\n"
         "set(CPACK_PACKAGE_NAME \"${CPACK_DEBIAN_PACKAGE_NAME}\")\n"
         "set(CPACK_PACKAGE_VERSION \"${CPACK_PACKAGE_VERSION}\")\n"
         "set(CPACK_PACKAGE_FILE_NAME \"${package_file_name}.orig\")\n"
         "set(CPACK_PACKAGE_DESCRIPTION \"${CPACK_PACKAGE_NAME} Source\")\n"
         "set(CPACK_IGNORE_FILES \"${CPACK_SOURCE_IGNORE_FILES}\")\n"
         "set(CPACK_INSTALLED_DIRECTORIES \"${CPACK_SOURCE_INSTALLED_DIRECTORIES}\")\n"
         "set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)\n"
         )

    set(orig_file "${CMAKE_BINARY_DIR}/Debian/${DISTRI}/${package_file_name}.orig.tar.gz")

    add_custom_command(OUTPUT ${orig_file}
                       COMMAND cpack --config ${CMAKE_BINARY_DIR}/Debian/${DISTRI}/cpack.cmake
                       WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Debian/${DISTRI}
                       )

    add_custom_command(OUTPUT ${DEBIAN_SOURCE_DIR}/CMakeLists.txt
                       COMMAND tar zxf ${orig_file}
                       WORKING_DIRECTORY ${DEBIAN_SOURCE_DIR}
                       DEPENDS ${orig_file}
                       )

    ##############################################################################
    # debuild -S -sa -us -uc
    set(DEB_SOURCE_CHANGES
        ${CPACK_DEBIAN_PACKAGE_NAME}_${DEBIAN_PACKAGE_VERSION}_source.changes
        )

    option(NO_SIGN_PPA "NO_SIGN_PPA" OFF)
    if(NO_SIGN_PPA)
        add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/Debian/${DISTRI}/${DEB_SOURCE_CHANGES}
                           COMMAND ${DEBUILD_EXECUTABLE} --no-tgz-check --preserve-env -S -sa -us -uc
                           WORKING_DIRECTORY ${DEBIAN_SOURCE_DIR}
                           DEPENDS ${DEBIAN_SOURCE_DIR}/CMakeLists.txt
                           )

    else()
        add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/Debian/${DISTRI}/${DEB_SOURCE_CHANGES}
                           COMMAND ${DEBUILD_EXECUTABLE} --no-tgz-check -S -sa -us -uc && ${DEBSIGN_EXECUTABLE} -p'gpg2 --pinentry-mode loopback --passphrase-file=${PPA_GPG_PASSPHRASE_FILE} --no-tty --yes' -S -k${PPA_GPG_KEY_ID} ${CMAKE_BINARY_DIR}/Debian/${DISTRI}/${DEB_SOURCE_CHANGES}
                           WORKING_DIRECTORY ${DEBIAN_SOURCE_DIR}
                           DEPENDS ${DEBIAN_SOURCE_DIR}/CMakeLists.txt
                           )
    endif()
    add_custom_target(debuild_${DISTRI} ALL
                      DEPENDS ${DEBIAN_SOURCE_DIR}/CMakeLists.txt
                      ${CMAKE_BINARY_DIR}/Debian/${DISTRI}/${DEB_SOURCE_CHANGES}
                      )
    ##############################################################################
    # dput ppa:your-lp-id/ppa <source.changes>
    message(STATUS "Upload PPA is ${UPLOAD_PPA}")
    option(SIMULATE_DPUT "SIMULATE_DPUT" ON)
    set(SIMULATE_DPUT_STR "-s")
    if(NOT SIMULATE_DPUT)
        set(SIMULATE_DPUT_STR "")
    endif(NOT SIMULATE_DPUT)
    message(STATUS "SIMULATE_DPUT_STR is ${SIMULATE_DPUT_STR}")
    if(UPLOAD_PPA)

        if(EXISTS ${DPUT_CONFIG_IN})
            set(DPUT_DIST ${DISTRI})
            configure_file(
                    ${DPUT_CONFIG_IN}
                    ${CMAKE_BINARY_DIR}/Debian/${DISTRI}/dput.cf
                    @ONLY
            )
            add_custom_target(dput_${DISTRI} ALL
                              COMMAND ${DPUT_EXECUTABLE} ${SIMULATE_DPUT_STR} -c ${CMAKE_BINARY_DIR}/Debian/${DISTRI}/dput.cf ${DPUT_HOST} ${DEB_SOURCE_CHANGES}
                              DEPENDS debuild_${DISTRI}
                              WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Debian/${DISTRI}
                              )
        else()
            add_custom_target(dput_${DISTRI} ALL
                              COMMAND ${DPUT_EXECUTABLE} ${SIMULATE_DPUT_STR} ${DPUT_HOST} ${DEB_SOURCE_CHANGES}
                              DEPENDS debuild_${DISTRI}
                              WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Debian/${DISTRI}
                              )
        endif()
    endif()
endforeach(DISTRI)
