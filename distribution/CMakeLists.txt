# Setup some odds-and-ends used for distribution
SET(K3D_SVN_ROOT "https://k3d.svn.sourceforge.net:/svnroot/k3d")
SET(K3D_SVN_TRUNK "${K3D_SVN_ROOT}/trunk")
SET(K3D_SVN_TAGS "${K3D_SVN_ROOT}/tags")
SET(K3D_SVN_BRANCHES "${K3D_SVN_ROOT}/branches")
SET(K3D_BRANCH_TAG "k3d-release-${K3D_MAJOR_VERSION}.${K3D_MINOR_VERSION}")
SET(K3D_VERSION_TAG "k3d-${K3D_MAJOR_VERSION}.${K3D_MINOR_VERSION}.${K3D_RELEASE_VERSION}.${K3D_BUILD_VERSION}")

# Get a SourceForge username so we can upload distributables to the FRS
IF(WIN32)
	SET(K3D_SOURCEFORGE_USERNAME "$ENV{USERNAME}" CACHE STRING "SourceForge user account name, for uploading distribution files.")
ELSE(WIN32)
	SET(K3D_SOURCEFORGE_USERNAME "$ENV{USER}" CACHE STRING "SourceForge user account name, for uploading distribution files.")
ENDIF(WIN32)
MARK_AS_ADVANCED(K3D_SOURCEFORGE_USERNAME)

ADD_CUSTOM_TARGET(changelog
	COMMAND ${SVNCOMMAND} log > ${k3d_BINARY_DIR}/ChangeLog
	WORKING_DIRECTORY ${k3d_SOURCE_DIR}
	)

ADD_CUSTOM_TARGET(tag-trunk
	COMMAND ${SVNCOMMAND} cp ${K3D_SVN_TRUNK}
	${K3D_SVN_TAGS}/${K3D_VERSION_TAG} -m "COMP: Tagged K-3D Version ${K3D_VERSION}"
	WORKING_DIRECTORY ${k3d_SOURCE_DIR}
	)

ADD_CUSTOM_TARGET(tag-branch
	COMMAND ${SVNCOMMAND} cp ${K3D_SVN_TRUNK}
	${K3D_SVN_BRANCHES}/${K3D_BRANCH_TAG} -m "COMP: Branched K-3D Version ${K3D_MAJOR_VERSION}.${K3D_MINOR_VERSION}"
	WORKING_DIRECTORY ${k3d_SOURCE_DIR}
	)

# Setup default packaging parameters (deprecated) ...
SET(CPACK_PACKAGE_DESCRIPTION "K-3D free-as-in-freedom modeling, animation and rendering system")
SET(CPACK_PACKAGE_FILE_NAME "k3d-setup-${K3D_VERSION}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "K-3D ${K3D_VERSION}")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "K-3D ${K3D_VERSION}")
SET(CPACK_PACKAGE_NAME "k3d")
SET(CPACK_PACKAGE_VENDOR "www.k-3d.org")
SET(CPACK_PACKAGE_VERSION ${K3D_VERSION})
SET(CPACK_PACKAGE_VERSION_MAJOR ${K3D_MAJOR_VERSION})
SET(CPACK_PACKAGE_VERSION_MINOR ${K3D_MINOR_VERSION})
SET(CPACK_PACKAGE_VERSION_PATCH ${K3D_RELEASE_VERSION})
SET(CPACK_RESOURCE_FILE_LICENSE "${k3d_SOURCE_DIR}/COPYING")

# Setup global packaging parameters ...
SET(K3D_CPACK_INSTALL_CMAKE_PROJECTS "${k3d_BINARY_DIR};k3d;ALL;/")
SET(K3D_CPACK_PACKAGE_DESCRIPTION "K-3D free-as-in-freedom modeling, animation, and rendering system.")
SET(K3D_CPACK_PACKAGE_FILE_NAME "k3d-setup-${K3D_VERSION}")
SET(K3D_CPACK_PACKAGE_NAME "k3d")
SET(K3D_CPACK_PACKAGE_VERSION "${K3D_VERSION}")
SET(K3D_CPACK_RESOURCE_FILE_LICENSE "${k3d_SOURCE_DIR}/COPYING")
SET(K3D_CPACK_SOURCE_IGNORE_FILES "/CVS/;/.svn/")
SET(K3D_CPACK_SOURCE_INSTALLED_DIRECTORIES "${k3d_SOURCE_DIR};/")
SET(K3D_CPACK_SOURCE_PACKAGE_FILE_NAME "k3d-source-${K3D_VERSION}")

# Setup some macros that can be used by the platform-specific distribution logic
MACRO(K3D_CHECK_INSTALLED_VERSION FOUND PACKAGE EXPECTED)
	IF(NOT ${FOUND} STREQUAL ${EXPECTED})
		MESSAGE(SEND_ERROR "Found ${PACKAGE} version ${FOUND} - for installation, version ${EXPECTED} is required.")
	ENDIF(NOT ${FOUND} STREQUAL ${EXPECTED})
ENDMACRO(K3D_CHECK_INSTALLED_VERSION)

MACRO(K3D_CHECK_INSTALLED_PYTHON_DEPENDENCY CONFIG)
	IF(${CONFIG} AND NOT K3D_INSTALL_PYTHON)
		MESSAGE(SEND_ERROR "K3D_INSTALL_PYTHON is required by ${CONFIG}.")
	ENDIF(${CONFIG} AND NOT K3D_INSTALL_PYTHON)
ENDMACRO(K3D_CHECK_INSTALLED_PYTHON_DEPENDENCY)

# Generate source tarballs
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/package-source-tbz2.cpack.in ${CMAKE_CURRENT_BINARY_DIR}/package-source-tbz2.cpack)
ADD_CUSTOM_TARGET(package-source-tbz2
	${CMAKE_CPACK_COMMAND}
	--config ${CMAKE_CURRENT_BINARY_DIR}/package-source-tbz2.cpack
	)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/package-source-tgz.cpack.in ${CMAKE_CURRENT_BINARY_DIR}/package-source-tgz.cpack)
ADD_CUSTOM_TARGET(package-source-tgz
	${CMAKE_CPACK_COMMAND}
	--config ${CMAKE_CURRENT_BINARY_DIR}/package-source-tgz.cpack
	)

# Generate source zip files
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/package-source-zip.cpack.in ${CMAKE_CURRENT_BINARY_DIR}/package-source-zip.cpack)
ADD_CUSTOM_TARGET(package-source-zip
	${CMAKE_CPACK_COMMAND}
	--config ${CMAKE_CURRENT_BINARY_DIR}/package-source-zip.cpack
	)

# Upload sources to SourceForge for use with the FRS
ADD_CUSTOM_TARGET(upload-package-source-tgz
	rsync -avP -e ssh ${CMAKE_CURRENT_BINARY_DIR}/${K3D_CPACK_SOURCE_PACKAGE_FILE_NAME}.tgz ${K3D_SOURCEFORGE_USERNAME}@frs.sourceforge.net:uploads/
	)

# Generate platform-specific binary packages
IF(APPLE)
	ADD_SUBDIRECTORY(osx)
ENDIF(APPLE)

IF(UNIX AND NOT APPLE)
	ADD_SUBDIRECTORY(posix)
ENDIF(UNIX AND NOT APPLE)

IF(WIN32)
	ADD_SUBDIRECTORY(win32)
ENDIF(WIN32)

