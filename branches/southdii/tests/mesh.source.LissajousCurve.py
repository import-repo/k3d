#python

import testing

setup = testing.setup_mesh_source_test("LissajousCurve")
testing.mesh_comparison(setup.document, setup.source.get_property("output_mesh"), "mesh.source.LissajousCurve", 2)

