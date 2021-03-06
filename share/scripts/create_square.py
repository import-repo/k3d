#python

import k3d

doc = Document
doc.start_change_set()
try:
	frozen_mesh = doc.new_node("FrozenMesh")
	frozen_mesh.name = "Square"

	mesh = k3d.dynamic_cast(frozen_mesh, "imesh_storage").reset_mesh()

	points = mesh.create_points()
	point_selection = mesh.create_point_selection()
	polyhedra = mesh.create_polyhedra()
	first_faces = polyhedra.create_first_faces()
	face_counts = polyhedra.create_face_counts()
	types = polyhedra.create_types()
	face_first_loops = polyhedra.create_face_first_loops()
	face_loop_counts = polyhedra.create_face_loop_counts()
	face_materials = polyhedra.create_face_materials()
	face_selection = polyhedra.create_face_selection()
	loop_first_edges = polyhedra.create_loop_first_edges()
	edge_points = polyhedra.create_edge_points()
	clockwise_edges = polyhedra.create_clockwise_edges()
	edge_selection = polyhedra.create_edge_selection()
	Cs = polyhedra.writable_face_varying_data().create_array("Cs", "k3d::color")

	positions = [(-5, 0, 5), (5, 0, 5), (5, 0, -5), (-5, 0, -5)]
	for position in positions:
		points.append(k3d.point3(position[0], position[1], position[2]))
		point_selection.append(0.0)

	first_faces.append(len(face_first_loops))
	face_counts.append(1)
	types.append(k3d.polyhedron_type.polygons)
	face_first_loops.append(len(loop_first_edges))
	face_loop_counts.append(1)
	face_materials.append(None)
	face_selection.append(0.0)
	loop_first_edges.append(len(edge_points))

	edge_points.append(0)
	edge_points.append(1)
	edge_points.append(2)
	edge_points.append(3)

	clockwise_edges.append(1)
	clockwise_edges.append(2)
	clockwise_edges.append(3)
	clockwise_edges.append(0)

	edge_selection.append(0.0)
	edge_selection.append(0.0)
	edge_selection.append(0.0)
	edge_selection.append(0.0)

	Cs.append(k3d.color(1, 0, 0))
	Cs.append(k3d.color(0, 1, 0))
	Cs.append(k3d.color(0, 0, 1))
	Cs.append(k3d.color(1, 1, 1))

	mesh_instance = doc.new_node("MeshInstance")
	mesh_instance.name = "Square Instance"
	mesh_instance.gl_painter = doc.get_node("GL Default Painter")
	mesh_instance.ri_painter = doc.get_node("RenderMan Default Painter")
	doc.set_dependency(mesh_instance.get_property("input_mesh"), frozen_mesh.get_property("output_mesh"))

	doc.finish_change_set("Create Square")

except:
	doc.cancel_change_set()
	raise

