#python

import testing
import k3d

document = k3d.new_document()
setup = testing.setup_mesh_reader_test("K3DMeshReader","mesh.modifier.NurbsExtrudeCurveReference.k3d")

modifier = setup.document.new_node("NurbsExtrudeCurve")
modifier.along = 'z'
modifier.distance = 3.0
modifier.segments = 2
modifier.delete_original = False
modifier.mesh_selection = k3d.mesh_selection.select_all()

document.set_dependency(modifier.get_property("input_mesh"), setup.reader.get_property("output_mesh"))

testing.mesh_comparison_to_reference(document, modifier.get_property("output_mesh"), "mesh.modifier.NurbsExtrudeCurve", 1)

