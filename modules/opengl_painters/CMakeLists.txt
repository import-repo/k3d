INCLUDE_DIRECTORIES(${ftgl_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${K3D_FREETYPE2_INCLUDE_DIRS})
LINK_DIRECTORIES(${K3D_FREETYPE2_LIB_DIRS})

K3D_BUILD_MODULE(k3d-opengl-painters)
K3D_CREATE_MODULE_PROXY(k3d-opengl-painters)

TARGET_LINK_LIBRARIES(k3d-opengl-painters k3dsdk-subdivision-surface)
TARGET_LINK_LIBRARIES(k3d-opengl-painters k3dsdk-ftgl)
TARGET_LINK_LIBRARIES(k3d-opengl-painters ${K3D_FREETYPE2_LIBS})

