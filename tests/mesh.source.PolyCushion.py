#python

import testing

setup = testing.setup_mesh_source_test("PolyCushion")
testing.mesh_comparison_to_reference(setup.document, setup.source.get_property("output_mesh"), "mesh.source.PolyCushion", 1)

