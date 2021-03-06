#python

import k3d
import testing

document = k3d.new_document()

source1 = document.new_node("BlobbyEllipsoid")
source1.color = k3d.color(1, 0, 0)

source2 = document.new_node("BlobbySegment")
source2.color = k3d.color(1, 1, 0)

modifier = document.new_node("BlobbyMultiply")
modifier.create_property("k3d::mesh*", "input_mesh1", "Input Mesh 1", "")
modifier.create_property("k3d::mesh*", "input_mesh2", "Input Mesh 2", "")

document.set_dependency(modifier.get_property("input_mesh1"), source1.get_property("output_mesh"))
document.set_dependency(modifier.get_property("input_mesh2"), source2.get_property("output_mesh"))

testing.mesh_comparison_to_reference(document, modifier.get_property("output_mesh"), "mesh.modifier.BlobbyMultiply", 5)

