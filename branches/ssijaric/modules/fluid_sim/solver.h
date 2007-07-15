#ifndef FLUID_SIM_SOLVER_H
#define FLUID_SIM_SOLVER_H

#include <k3dsdk/document_plugin_factory.h>
#include <k3dsdk/imaterial.h> 
#include <k3dsdk/node.h>
#include <k3dsdk/persistent.h> 
#include <k3dsdk/material.h>
#include <k3dsdk/material_client.h>
#include <k3dsdk/measurement.h>
#include <k3dsdk/mesh_source.h>
#include <k3dsdk/mesh_modifier.h>
#include <k3dsdk/mesh.h>
#include <k3dsdk/module.h>
#include <k3dsdk/property.h>


#include "voxel_grid.h"

namespace fluid_sim
{

class solver : public k3d::node
{
public:
	solver(k3d::iplugin_factory& Factory, k3d::idocument& Document);
	static k3d::iplugin_factory& get_factory();

protected:
	 void step(float* u1, float* u0); 	// vstep and sstep both operate on an array of reals
	 void transport(float *u1, float *u0);

	 k3d_data(voxel_grid*, immutable_name, change_signal, no_undo, node_storage, no_constraint, node_property, no_serialization) m_voxel_grid;
	 k3d_data(float, immutable_name, change_signal, with_undo, local_storage, no_constraint, read_only_property, no_serialization) m_timestep;
	 k3d_data(int, immutable_name, change_signal, with_undo, local_storage, no_constraint, read_only_property, no_serialization) m_steps;
	 k3d_data(float, immutable_name, change_signal, with_undo, local_storage, no_constraint, read_only_property, no_serialization) m_viscosity;


	 sigc::slot<void, iunknown*> start_solver_slot();
	 void start_solver(iunknown* const Hint);
	 void simulate();
	 void vstep(float* u1, float* u0, float viscosity, float* forces);
	 void transport(float* u1, float* u0, float* ufield);
	 void diffuse(float* u1, float* u0);
	 void project(float* u1, float* u0);
	 void swap(float *x, float* y) 
	 {
		float* temp = x;
		x = y;
		y = temp;
	 }
	 void trace_particle(const k3d::vector3& x, float* ufield, float timestep, float* result);





};

k3d::iplugin_factory& solver_factory();

}

#endif
