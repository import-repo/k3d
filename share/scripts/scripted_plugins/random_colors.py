#python

# k3d:plugin-class="document"
# k3d:plugin-type="MeshModifierScript"
# k3d:plugin-name="RandomColors"
# k3d:plugin-description="Adds random colors to the attribute arrays in the polyhedra structure and to the vertices"

import k3d
k3d.check_node_environment(locals(), "MeshModifierScript")

from random import seed, randint

Output.copy(Input)

seed(123)

colors = [ k3d.color(1, 0, 0), k3d.color(1, 1, 0), k3d.color(0, 1, 0), k3d.color(0, 1, 1), k3d.color(0, 0, 1), k3d.color(1, 0, 1), k3d.color(1, 1, 1)]

# We iterate over input primitives, to avoid making a writable copy of every primitive
for prim_idx, const_primitive in enumerate(Input.primitives()):
	if const_primitive.type() == "polyhedron":
		polyhedron = k3d.polyhedron.validate(Output.primitives()[prim_idx])
		if polyhedron:
			Cs = polyhedron.face_varying_data().create("Cs", "k3d::color")
			for i in range(len(polyhedron.edge_points())):
				Cs.append(colors[i % len(colors)])
			Cs = polyhedron.uniform_data().create("Cs", "k3d::color")
			for i in range(len(polyhedron.face_first_loops())):
				Cs.append(colors[i % len(colors)])
			Cs = polyhedron.constant_data().create("Cs", "k3d::color")
			for i in range(len(polyhedron.shell_first_faces())):
				Cs.append(colors[i % len(colors)])

Cs = Output.vertex_data().create("Cs", "k3d::color")
for point in range(len(Input.points())):
	Cs.append(colors[point % len(colors)])