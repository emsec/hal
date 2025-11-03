# enable all QT-Stuff we need

SET(qgvlib_TS)

SET(qgvlib_UI)

SET(TARGET_VERSION_MAJOR 0 )
SET(TARGET_VERSION_MINOR 1 )

# all files which may be mocced
qt5_wrap_cpp(MOC_SCENE ${CMAKE_CURRENT_SOURCE_DIR}/QGVScene.h)

SET(QGVLIB_CPP
	private/QGVCore.cpp
  private/QGVGraphPrivate.cpp
  private/QGVEdgePrivate.cpp
  private/QGVGvcPrivate.cpp
  private/QGVNodePrivate.cpp
  QGVEdge.cpp
  QGVNode.cpp
  QGVScene.cpp
  QGVSubGraph.cpp
  ${MOC_SCENE}
)

SET(TARGET_NAME "QGVCore")

# include with QT_USE selected library parts
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

INCLUDE_DIRECTORIES(
	${GRAPHVIZ_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/private
  ${CMAKE_CURRENT_BINARY_DIR}/private
  
)
# Activate Export macro for building the library itself
# if QGVCORE_LIB is not set or equal 0 (null), import macro is used
ADD_DEFINITIONS(-DQGVCORE_LIB -D_PACKAGE_ast -D_dll_import -D_BLD_cdt -D_DLL_BLD)

ADD_DEFINITIONS(-DQVGCORE_LIB)

        QT5_WRAP_UI(QGVLIB_UI_H ${qgvlib_UI})
	QT5_ADD_RESOURCES(qgvlib_RCCS ${qgvlib_QRC})

ADD_LIBRARY(qgvcore SHARED
  ${QGVLIB_CPP}
  ${QGVLIB_UI_H}
  ${qgvlib_TRANS}
)

TARGET_LINK_LIBRARIES(qgvcore
  ${QT_LIBRARIES}
	${GRAPHVIZ_GVC_LIBRARY}
  ${GRAPHVIZ_CGRAPH_LIBRARY}
  ${GRAPHVIZ_CDT_LIBRARY}
  Qt5::Core 
  Qt5::Widgets
)

string(TOUPPER ${CMAKE_BUILD_TYPE} CFG)
SET_TARGET_PROPERTIES(qgvcore PROPERTIES
	OUTPUT_NAME "QGVCore"
	VERSION ${TARGET_VERSION_MAJOR}.${TARGET_VERSION_MINOR}.0
        SOVERSION ${TARGET_VERSION_MAJOR}
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY_${CFG} "${CMAKE_BINARY_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY_${CFG} "${CMAKE_BINARY_DIR}/lib"
)

# TODO : verify install directory
INSTALL(TARGETS qgvcore
                ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIRECTORY}
                LIBRARY DESTINATION ${LIBRARY_INSTALL_DIRECTORY}
		COMPONENT library
)

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_NAME}.pc.in
                ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.pc @ONLY)
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_NAME}-uninstalled.pc.in
                ${CMAKE_BINARY_DIR}/${TARGET_NAME}-uninstalled.pc @ONLY)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.pc
                DESTINATION lib/pkgconfig)


IF(BUILD_DEV)
	FILE(GLOB header "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
	INSTALL(
		FILES ${header}
		DESTINATION ${include_DIR}
		COMPONENT headerfiles
	)
ENDIF(BUILD_DEV)
