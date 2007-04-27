FIND_PATH(K3D_INTL_INCLUDE_DIR libintl.h
	/usr/include
	c:/gtk/include
	DOC "Directory where the libintl header files are located"
	)
MARK_AS_ADVANCED(K3D_INTL_INCLUDE_DIR)

SET(K3D_INTL_LIB intl CACHE STRING "")
MARK_AS_ADVANCED(K3D_INTL_LIB)

SET(K3D_INTL_INCLUDE_DIRS
	${K3D_LIBINTL_INCLUDE_DIR}
	)

SET(K3D_INTL_LIBS
	${K3D_LIBINTL_LIB}
	)

SET(K3D_INTL_FOUND 1)

