INCLUDE_DIRECTORIES(${k3d_SOURCE_DIR})
INCLUDE_DIRECTORIES(${k3d_BINARY_DIR}/k3dsdk)
INCLUDE_DIRECTORIES(${K3D_BOOST_INCLUDE_DIRS})

ADD_EXECUTABLE(test-types main.cpp)
ADD_TEST(k3dsdk.types ${EXECUTABLE_OUTPUT_PATH}/test-types)

