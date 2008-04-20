#python

import testing

setup = testing.setup_mesh_source_test("PolyCube")

testing.assert_valid_mesh(setup.output_mesh)
testing.assert_solid_mesh(setup.output_mesh)
testing.mesh_comparison(setup.document, setup.source.get_property("output_mesh"), "mesh.source.PolyCube", 1)

