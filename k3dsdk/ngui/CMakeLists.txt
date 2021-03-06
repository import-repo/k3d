PROJECT(k3dsdk-ngui)

FILE(GLOB HEADERS *.h)
FILE(GLOB SOURCES *.cpp)

LIST(SORT HEADERS)
LIST(SORT SOURCES)

INCLUDE_DIRECTORIES(${k3d_SOURCE_DIR})
INCLUDE_DIRECTORIES(${k3dsdk_SOURCE_DIR}/gil)
INCLUDE_DIRECTORIES(${k3dsdk_BINARY_DIR})
INCLUDE_DIRECTORIES(${k3dsdk_BINARY_DIR})
INCLUDE_DIRECTORIES(${K3D_BOOST_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${K3D_GLIBMM_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${K3D_GTKGLEXT_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${K3D_GTKMM_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${K3D_SIGC_INCLUDE_DIRS})

LINK_DIRECTORIES(${K3D_BOOST_LIB_DIRS})
LINK_DIRECTORIES(${K3D_GTKGLEXT_LIB_DIRS})
LINK_DIRECTORIES(${K3D_GTKMM_LIB_DIRS})
LINK_DIRECTORIES(${K3D_SIGC_LIB_DIRS})

ADD_LIBRARY(k3dsdk-ngui SHARED ${HEADERS} ${SOURCES})
K3D_GENERATE_DEF_FILE(k3dsdk-ngui)

TARGET_LINK_LIBRARIES(k3dsdk-ngui k3dsdk)
TARGET_LINK_LIBRARIES(k3dsdk-ngui ${K3D_BOOST_PROGRAM_OPTIONS_LIBS})
TARGET_LINK_LIBRARIES(k3dsdk-ngui ${K3D_BOOST_REGEX_LIBS})
TARGET_LINK_LIBRARIES(k3dsdk-ngui ${K3D_GLIBMM_LIBS})
TARGET_LINK_LIBRARIES(k3dsdk-ngui ${K3D_GTKGLEXT_LIBS})
TARGET_LINK_LIBRARIES(k3dsdk-ngui ${K3D_GTKMM_LIBS})

IF(K3D_ENABLE_NLS)
	INCLUDE_DIRECTORIES(${K3D_INTL_INCLUDE_DIRS})
	TARGET_LINK_LIBRARIES(k3dsdk-ngui ${K3D_INTL_LIBS})
ENDIF(K3D_ENABLE_NLS)

INSTALL(TARGETS k3dsdk-ngui
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION ${K3D_LIBDIR}
	ARCHIVE DESTINATION ${K3D_LIBDIR})

