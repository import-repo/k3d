#python

import k3d
k3d.check_node_environment(locals(), "MeshSourceScript")

positions = [
	(-5, -5, 0), (-2, -5, 2), (2, -5, -2), (5, -5, 0),
	(-5, -2, 2), (-2, -2, 5), (2, -2, -5), (5, -2, -2),
	(-5, 2, 2), (-2, 2, 5), (2, 2, -5), (5, 2, -2),
	(-5, 5, 0), (-2, 5, 2), (2, 5, -2), (5, 5, 0)]

points = Output.create_points()
point_selection = Output.create_point_selection()

for position in positions:
	points.append(k3d.point3(position[0], position[2], -position[1]))
	point_selection.append(0.0)

bicubic_patches = Output.create_bicubic_patches()

patch_selection = bicubic_patches.create_patch_selection()
patch_selection.assign([0])

patch_materials = bicubic_patches.create_patch_materials()
patch_materials.assign([None])

patch_points = bicubic_patches.create_patch_points()
for i in range(16):
	patch_points.append(i)

Cs = bicubic_patches.writable_varying_data().create_array("Cs", "k3d::color")
Cs.assign([k3d.color(1, 0, 0), k3d.color(0, 1, 0), k3d.color(0, 0, 1), k3d.color(1, 1, 1)])

