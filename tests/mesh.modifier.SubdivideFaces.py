#python

import k3d
import testing

document = k3d.new_document()

reader = document.new_node("K3DMeshReader")
# load a mesh that has multiple polyhedra, triangles, quads, n-sided polygons and holes. (i.e. a mesh from hell)
reader.file = k3d.filesystem.generic_path(testing.source_path() + "/meshes/testmesh.polyhedra.hole.k3d")
reader.center = False
reader.scale_to_size = False

varying_colors = document.new_node("RandomColors")

document.set_dependency(varying_colors.get_property("input_mesh"), reader.get_property("output_mesh"))

modifier = document.new_node("SubdivideFaces")
# select some faces, distributed along polyhedra
selection = k3d.mesh_selection.deselect_all()
selection.faces = [(0, 6, 0), (6, 7, 1), (7, 8, 0), (8, 9, 1), (9, 13, 0), (13, 15, 1), (15, 16, 0), (16, 17, 1), (17, 19, 0), (19, 20, 1), (20, 23, 0)]
modifier.mesh_selection = selection

document.set_dependency(modifier.get_property("input_mesh"), varying_colors.get_property("output_mesh"))

testing.mesh_comparison_to_reference(document, modifier.get_property("output_mesh"), "mesh.modifier.SubdivideFaces.CenterMidpoints", 1)

modifier.subdivision_type = "midpoints"
testing.mesh_comparison_to_reference(document, modifier.get_property("output_mesh"), "mesh.modifier.SubdivideFaces.Midpoints", 1)

modifier.subdivision_type = "center"
testing.mesh_comparison_to_reference(document, modifier.get_property("output_mesh"), "mesh.modifier.SubdivideFaces.Center", 1)
