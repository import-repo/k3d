#python

import k3d
import testing

doc = k3d.new_document()

# We will be writing a temporary file ...
file = k3d.filesystem.generic_path(testing.binary_path() + "/mesh.sink.OBJMeshWriter.obj")

# Create a simple polyhedron source ...
source = doc.new_node("PolyCube")

# Write the geometry to a temporary file ...
writer = doc.new_node("OBJMeshWriter")
writer.file = file
doc.set_dependency(writer.get_property("input_mesh"), source.get_property("output_mesh"))

# Read the geometry back in ...
reader = doc.new_node("OBJMeshReader")
reader.file = file
reader.center = False
reader.scale_to_size = False

# Compare the original to the imported data ...

diff = doc.new_node("MeshDiff")
diff.create_property("k3d::mesh*", "input_a", "InputA", "First input mesh")
diff.create_property("k3d::mesh*", "input_b", "InputB", "Second input mesh")

doc.set_dependency(diff.get_property("input_a"), source.get_property("output_mesh"))
doc.set_dependency(diff.get_property("input_b"), reader.get_property("output_mesh"))

if not diff.get_property("input_a").pipeline_value() or not diff.get_property("input_b").pipeline_value():
	raise Exception("missing mesh comparison input")

if not diff.equal:
	print "source " + repr(source.output_mesh)
	print "reader " + repr(reader.output_mesh)
	raise Exception("Exported mesh differs")

