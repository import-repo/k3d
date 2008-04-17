#python

import pygtk
pygtk.require('2.0')

import gtk
import k3d
import math
import sys

def mix(a, b, amount):
	return (a * (1 - amount)) + (b * amount)

x1 = -5
x2 = 5
xcount = 29

y1 = -5
y2 = 5
ycount = 29

function = "-2 * math.sqrt(math.fabs(x * y))"

doc = Document

x1_entry = gtk.Entry()
x1_entry.set_text(str(x1))

x2_entry = gtk.Entry()
x2_entry.set_text(str(x2))

xcount_entry = gtk.Entry()
xcount_entry.set_text(str(xcount))

y1_entry = gtk.Entry()
y1_entry.set_text(str(y1))

y2_entry = gtk.Entry()
y2_entry.set_text(str(y2))

ycount_entry = gtk.Entry()
ycount_entry.set_text(str(ycount))

function_entry = gtk.Entry()
function_entry.set_text(str(function))

table = gtk.Table(7, 2)

table.attach(gtk.Label("x1"), 0, 1, 0, 1)
table.attach(x1_entry, 1, 2, 0, 1)

table.attach(gtk.Label("x2"), 0, 1, 1, 2)
table.attach(x2_entry, 1, 2, 1, 2)

table.attach(gtk.Label("xcount"), 0, 1, 2, 3)
table.attach(xcount_entry, 1, 2, 2, 3)

table.attach(gtk.Label("y1"), 0, 1, 3, 4)
table.attach(y1_entry, 1, 2, 3, 4)

table.attach(gtk.Label("y2"), 0, 1, 4, 5)
table.attach(y2_entry, 1, 2, 4, 5)

table.attach(gtk.Label("ycount"), 0, 1, 5, 6)
table.attach(ycount_entry, 1, 2, 5, 6)

table.attach(gtk.Label("function"), 0, 1, 6, 7)
table.attach(function_entry, 1, 2, 6, 7)

dialog = gtk.Dialog("Plot Function", None, gtk.DIALOG_MODAL | gtk.DIALOG_NO_SEPARATOR, (gtk.STOCK_OK, gtk.RESPONSE_ACCEPT, gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT))
dialog.vbox.pack_start(table)
dialog.show_all()
result = dialog.run()
dialog.hide()

if result == gtk.RESPONSE_ACCEPT:
	x1 = float(x1_entry.get_text())
	x2 = float(x2_entry.get_text())
	xcount = int(xcount_entry.get_text())
	y1 = float(y1_entry.get_text())
	y2 = float(y2_entry.get_text())
	ycount = int(ycount_entry.get_text())
	function = function_entry.get_text()

	doc.start_change_set()
	try:
		frozen_mesh = doc.new_node("FrozenMesh")
		frozen_mesh.name = function

		mesh = frozen_mesh.new_mesh()
		for xi in range(0, xcount):
			for yi in range(0, ycount):
				x = mix(x1, x2, float(xi) / (xcount - 1))
				y = mix(y1, y2, float(yi) / (ycount - 1))
				z = eval(function)
				mesh.new_point([x, y, z])

		mesh_instance = doc.new_node("MeshInstance")
		mesh_instance.name = function + " Instance"
		doc.set_dependency(mesh_instance.get_property("input_mesh"), frozen_mesh.get_property("output_mesh"))

		doc.finish_change_set("Plot " + function)
	except:
		doc.cancel_change_set()
		raise

sys.stdout.flush()
sys.stderr.flush()