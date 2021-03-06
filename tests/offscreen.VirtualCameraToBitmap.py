#python

import k3d
import testing

doc = k3d.new_document()

axes = doc.new_node("Axes")
axes.xyplane = False

material = doc.new_node("RenderManMaterial")

torus = doc.new_node("Torus")
torus.material = material

mesh_instance = doc.new_node("MeshInstance")
mesh_instance.gl_painter = doc.new_node("OpenGLTorusPainter")
doc.set_dependency(mesh_instance.get_property("input_mesh"), torus.get_property("output_mesh"))

camera = testing.create_camera(doc)
render_engine = testing.create_opengl_engine(doc)

camera_to_bitmap = doc.new_node("VirtualCameraToBitmap")
camera_to_bitmap.camera = camera
camera_to_bitmap.render_engine = render_engine

testing.image_comparison(doc, camera_to_bitmap.get_property("output_bitmap"), "offscreen.VirtualCameraToBitmap", 0.009)

