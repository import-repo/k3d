#include <k3dsdk/legacy_mesh.h>
#include <k3dsdk/mesh.h>

#include <iostream>

int main(int argc, char* argv[])
{
	std::cout << "sizeof(k3d::legacy::mesh): " << sizeof(k3d::legacy::mesh) << std::endl;

	std::cout << "sizeof(k3d::mesh::points_t): " << sizeof(k3d::mesh::points_t) << std::endl;
	std::cout << "sizeof(k3d::mesh): " << sizeof(k3d::mesh) << std::endl;

	return 0;
}

