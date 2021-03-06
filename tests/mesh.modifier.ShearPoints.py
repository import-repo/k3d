#python

import k3d
import testing

setup = testing.setup_mesh_modifier_test("PolyCube", "ShearPoints")

selection = k3d.mesh_selection.deselect_all()
selection.points = k3d.mesh_selection.component_select_all()

setup.modifier.mesh_selection = selection
setup.modifier.shear_factor = 1.0

testing.mesh_comparison_to_reference(setup.document, setup.modifier.get_property("output_mesh"), "mesh.modifier.ShearPoints", 1)

