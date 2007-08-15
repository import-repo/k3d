#include "solver.h"
#include <map>

// semi-largangian method, as it applies to water simulation
//
// non-periodic boundary conditions

namespace fluid_sim
{

	solver::solver(k3d::iplugin_factory& Factory, k3d::idocument& Document) :
		k3d::node(Factory, Document),
		m_voxel_grid(init_owner(*this) + init_name("voxel_grid") + init_label(_("Voxel Grid")) + init_description(_("Voxel Grid")) + init_value<voxel_grid*>(0)),
		m_timestep(init_owner(*this) + init_name("timestep") + init_label(_("Timestep")) + init_description(_("Timestep")) + init_value(0.1) + init_step_increment(0.1) + init_units(typeid(k3d::measurement::scalar))),
		m_steps(init_owner(*this) + init_name("steps") + init_label(_("Steps")) + init_description(_("Steps")) + init_value(2) + init_step_increment(1) + init_units(typeid(k3d::measurement::scalar))),
		m_viscosity(init_owner(*this) + init_name("timestep") + init_label(_("Timestep")) + init_description(_("Timestep")) + init_value(0.1) + init_step_increment(0.1) + init_units(typeid(k3d::measurement::scalar)))


		
	{
		m_voxel_grid.changed_signal().connect(start_solver_slot());	
	}


	sigc::slot<void, k3d::iunknown*> solver::start_solver_slot()		
	{
		
		//return sigc::mem_fun(*this, &solver::start_solver);
	}



	k3d::iplugin_factory& solver::get_factory()
 	{
		static k3d::document_plugin_factory<solver, k3d::interface_list<k3d::inode> > factory(
				k3d::uuid(0x1ac5578c, 0xa944a032, 0x30384fbd, 0x3aab8ab4),
				"FluidSolverPlugin",
				"Fluid Solver",
				"Fluid",
				k3d::iplugin_factory::EXPERIMENTAL);
		return factory;
	}

	k3d::iplugin_factory& solver_factory() {
		return solver::get_factory();
	}


	void setup_diffusion_velocity_bc(array3d_i& faces) {
		const boost::multi_array_types::size_type* st = faces.shape();	
		int xfaces = st[0];
		int yfaces = st[1];
		int zfaces = st[2];

		
		for (int i = 0; i < xfaces; ++i) {
			for (int j = 0; j < yfaces; ++j) {
				faces[i][j][zfaces-1] = -1;		
			}
		}

		for (int i = 0; i < xfaces; ++i) {
			for (int k = 0; k < zfaces; ++k) {
				faces[i][yfaces-1][k] = -1;
			}
		}

		for (int j = 0; j < yfaces; ++j) {
			for (int k = 0; k < zfaces; ++k) {
				faces[xfaces-1][j][k] = -1;
			}
		}
		
	}


	// Projection step to make the field divergence free
	//
	// We solve Ax = b, where A =  -dx^2*Laplacian
	// The Laplacian is negated in order to make A positive definite, and dx^2 = voxel_width^2
	// b = dx^2*rho*(div u)/dt
	//
	// Some notes: For all solid cells (ie. obstacles), the pressure values have the Neumann BC of 0 (ie. no change in pressure)
	// For this to work, we also set the velocities at cell faces of solid objects to 0
	// Why foes this work? ---> the projection step involves the subtraction of the pressure gradient from the velocity,
	// which gives the new velocity
	// In the case of the solid cell, the velocity at the face is 0, and the pressure gradient is 0 -> so the new velocity is
	// also 0
	void solver::project(voxel_grid& new_grid, const voxel_grid& old_grid)
	{
		// easier to use the u, w notation
		voxel_grid& u = new_grid;
		const voxel_grid& w = old_grid;

		int xvoxels = w.xvoxels();
		int yvoxels = w.yvoxels();
		int zvoxels = w.zvoxels();
		float vox_width = w.voxel_width();

		//std::list<idx> fluid_voxels;
		std::list<idx> fluid_voxels;
		array3d_i fluid_voxels_r(boost::extents[xvoxels][yvoxels][zvoxels]); // -1 if (i,j,k) is in not in A, the row index otherwise

		
		// first visit all the voxels and determine the corresponance (i,j,k) -> i
		int row = 0;
		for (int i = 0; i < xvoxels; ++i) {
			for (int j = 0; j < yvoxels; ++j) {
				for (int k = 0; k < zvoxels; ++k) {
					if (w.is_fluid(i,j,k)) {
						fluid_voxels.push_back(idx(i,j,k));
						fluid_voxels_r[i][j][k] = row;
						++row;
					}
					else fluid_voxels_r[i][j][k] = -1;
				}
			}
		}

		int size = fluid_voxels.size();
		gmm::row_matrix< gmm::wsvector<float> > A(size, size);
		std::vector<float> b(size);
		row = 0;
		for (std::list<idx>::iterator it = fluid_voxels.begin(); it != fluid_voxels.end(); ++it) {
			int i = (*it).i;
			int j = (*it).j;
			int k = (*it).k;

			b[row] = -(vox_width*w.density(i,j,k)/m_timestep.value())*(w.vx(i+1,j,k) - w.vx(i,j,k) +
				   w.vy(i,j+1,k) - w.vy(i,j,k) +
				   w.vz(i,j,k+1) - w.vz(i,j,k));

			// the following accounts for both Neumann boundary conditions and
			// Dirichlet boundary conditions
			// (ie. Nuemann BCs are needed at the obstacle cells, so that difference in pressure is zero,
			//  and Dirichlet BCs are needed for emtpy (air) cells, where the pressure is 0
			//  We start off assuming that all six surrounding cells are fluid cells, hence A(row,row) = 6,
			//  and for each cell that is an air cell or an obstacle we subtract 1 from 6
			//  This results in the correct matrix representation that accounts for BCs
			A(row,row) = 6;

			if (fluid_voxels_r[i-1][j][k] == -1) {
				A(row,row) -= 1; 
			}
			else {
				A(row,fluid_voxels_r[i-1][j][k]) = -1;
			}

			if (fluid_voxels_r[i][j-1][k] == -1) {
				A(row,row) -= 1; 
			}
			else {
				A(row,fluid_voxels_r[i][j-1][k]) = -1;
			}

			if (fluid_voxels_r[i][j][k-1] == -1) {
				A(row,row) -= 1; 
			}
			else {
				A(row,fluid_voxels_r[i][j][k-1]) = -1;
			}

			if (fluid_voxels_r[i+1][j][k] == -1) {
				A(row,row) -= 1; 
			}
			else {
				A(row,fluid_voxels_r[i+1][j][k]) = -1;
			}

			if (fluid_voxels_r[i][j+1][k] == -1) {
				A(row,row) -= 1; 
			}
			else {
				A(row,fluid_voxels_r[i][j+1][k]) = -1;
			}

			if (fluid_voxels_r[i][j][k+1] == -1) {
				A(row,row) -= 1; 
			}
			else {
				A(row,fluid_voxels_r[i][j][k+1]) = -1;
			}
			
			++row;

		}


		// now we have Ax = b, which is sparse, symmetric and positive definite...solve using CG

	}

	// - difuse the velocities - Ax = b, where A has a row for each velocity component cell face that has fluid on both sides of it
	// - diffuse each x, y, and z component seperately
	// - after the advection step, (i.e. right before diffusion), the boundary conditions must be updated
	void solver::diffuse_velocities(voxel_grid& new_grid, const voxel_grid& old_grid)
	{
		voxel_grid& u = new_grid;
		const voxel_grid& w = old_grid;

		int xfaces = w.xfaces();
		int yfaces = w.yfaces();
		int zfaces = w.zfaces();
	
		std::list<idx> fluid_faces;
		array3d_i fluid_faces_r(boost::extents[xfaces][yfaces][zfaces]);

		setup_diffusion_velocity_bc(fluid_faces_r);

		int row = 0;
		for (int i = 1; i < xfaces-1; ++i) {
			for (int j = 1; j < yfaces-1; ++j) {
				for (int k = 1; k < zfaces-1; ++k) {
					if (w.is_fluid(i,j,k) && w.is_fluid(i-1,j,k)) {
						fluid_faces.push_back(idx(i,j,k));
						fluid_faces_r[i][j][k] = row;
						++row;
					}	
					else {
						fluid_faces_r[i][j][k] = -1;
					}
				}
			}
		}
		setup_and_solve_diffusion(fluid_faces, fluid_faces_r, u, w, voxel_grid::VX);


		fluid_faces.clear();
		row = 0;
		for (int i = 1; i < xfaces-1; ++i) {
			for (int j = 1; j < yfaces-1; ++j) {
				for (int k = 1; k < zfaces-1; ++k) {
					if (w.is_fluid(i,j,k) && w.is_fluid(i,j-1,k)) {
						fluid_faces.push_back(idx(i,j,k));
						fluid_faces_r[i][j][k] = row;
						++row;
					}
					else {
						fluid_faces_r[i][j][k] = -1;
					}
				}
			}
		}
		setup_and_solve_diffusion(fluid_faces, fluid_faces_r, u, w, voxel_grid::VY);


		fluid_faces.clear();
		row = 0;
		for (int i = 1; i < xfaces-1; ++i) {
			for (int j = 1; j < yfaces-1; ++j) {
				for (int k = 1; k < zfaces-1; ++k) {
					if (w.is_fluid(i,j,k) && w.is_fluid(i,j,k-1)) {
						fluid_faces.push_back(idx(i,j,k));
						fluid_faces_r[i][j][k] = row;
						++row;
					}
					else {
						fluid_faces_r[i][j][k] = -1 ;
					
					}
				}
			}
		}
		setup_and_solve_diffusion(fluid_faces, fluid_faces_r, u, w, voxel_grid::VZ);


	}

	void solver::setup_and_solve_diffusion(const std::list<idx>& faces, array3d_i& faces_r, voxel_grid& u, const voxel_grid& w, voxel_grid::velocity_type vtype)
	{
		int size = faces.size();
		gmm::row_matrix< gmm::wsvector<float> > A(size, size);
		std::vector<float> b(size);
		float beta = m_viscosity.value()*m_timestep.value()/(m_vox_width*m_vox_width);

		
		int row = 0;
		if (vtype == voxel_grid::VX) {
			for (std::list<idx>::const_iterator it = faces.begin(); it != faces.end(); ++it) {
				int i = (*it).i;
				int j = (*it).j;
				int k = (*it).k;
	
				b[row] = w.vx(i,j,k);
				++row;
			}
		}
		else if (vtype == voxel_grid::VX) {
			for (std::list<idx>::const_iterator it = faces.begin(); it != faces.end(); ++it) {
				int i = (*it).i;
				int j = (*it).j;
				int k = (*it).k;
	
				b[row] = w.vy(i,j,k);
				++row;
			}
		}
		else if (vtype == voxel_grid::VY) {
			for (std::list<idx>::const_iterator it = faces.begin(); it != faces.end(); ++it) {
				int i = (*it).i;
				int j = (*it).j;
				int k = (*it).k;
	
				b[row] = w.vy(i,j,k);
				++row;
			}

		}

		row = 0;
		for (std::list<idx>::const_iterator it = faces.begin(); it != faces.end(); ++it) {
			int i = (*it).i;
			int j = (*it).j;
			int k = (*it).k;
			
			A(row,row) = 1 + 6*beta;

			if (faces_r[i-1][j][k] == -1) {
				A(row,row) -= 1; 
			}
			else {
				A(row,faces_r[i-1][j][k]) = -beta;
			}

			if (faces_r[i][j-1][k] == -1) {
				A(row,row) -= 1; 
			}
			else {
				A(row,faces_r[i][j-1][k]) = -beta;
			}

			if (faces_r[i][j][k-1] == -1) {
				A(row,row) -= 1; 
			}
			else {
				A(row,faces_r[i][j][k-1]) = -beta;
			}

			if (faces_r[i+1][j][k] == -1) {
				A(row,row) -= 1; 
			}
			else {
				A(row,faces_r[i+1][j][k]) = -beta;
			}

			if (faces_r[i][j+1][k] == -1) {
				A(row,row) -= 1; 
			}
			else {
				A(row,faces_r[i][j+1][k]) = -beta;
			}

			if (faces_r[i][j][k+1] == -1) {
				A(row,row) -= 1; 
			}
			else {
				A(row,faces_r[i][j][k+1]) = -beta;
			}
			
			++row;

		}

	}

	
	void solver::add_force(voxel_grid& u, const array3d_i& forcesx, const array3d_i& forcesy, const array3d_i& forcesz)
	{
		float timestep = m_timestep.value();
		for (int i = 0; i < u.xvoxels(); ++i) {
			for (int j = 0; j < u.yvoxels(); ++j) {
				for (int k = 0; k < u.zvoxels(); ++k) {
					u.vx(i,j,k) += timestep*forcesx[i][j][k];
					u.vy(i,j,k) += timestep*forcesy[i][j][k];
					u.vz(i,j,k) += timestep*forcesz[i][j][k];
				}
			}
		}
	}

	k3d::point3 solver::trace_particle(const k3d::point3& p, float dt) {
		k3d::point3 v, v2;

		v[0] = m_voxel_grid.value()->interpolate_vx(p);
		v[1] = m_voxel_grid.value()->interpolate_vy(p);
		v[2] = m_voxel_grid.value()->interpolate_vz(p);

		v[0] = p[0] + 0.5*dt*v[0];
		v[1] = p[1] + 0.5*dt*v[1];
		v[2] = p[2] + 0.5*dt*v[2];

		v2[0] = m_voxel_grid.value()->interpolate_vx(v);
		v2[1] = m_voxel_grid.value()->interpolate_vy(v);
		v2[2] = m_voxel_grid.value()->interpolate_vz(v);

		return p + dt*v;

		
	}

	// This routine must be called before the diffusion step to make sure that surface cells are
	// divergence-free
	void solver::update_surface_face_boundaries(voxel_grid& u)
	{
		std::list<idx> xfaces;
		std::list<idx> yfaces;
		std::list<idx> zfaces;

		// faces opposite those in lists above
		std::list<idx> xfaces_op;
		std::list<idx> yfaces_op;
		std::list<idx> zfaces_op;

		int num_xfaces, num_yfaces, num_zfaces;
		int surface_faces;

		for (int i = 1; i < u.xvoxels(); ++i) {
			for (int j = 1; j < u.yvoxels(); ++j) {
				for (int k = 1; k < u.zvoxels(); ++k) {
					if (u.is_fluid(i,j,k)) {
						if (u.is_air(i-1,j,k)) {
							xfaces.push_back(idx(i,j,k));
							xfaces_op.push_back(idx(i+1,j,k));
						}
						if (u.is_air(i+1,j,k)) {
							xfaces.push_back(idx(i+1,j,k));
							xfaces_op.push_back(idx(i,j,k));
						}
						if (u.is_air(i,j-1,k)) {
							yfaces.push_back(idx(i,j,k));
							yfaces_op.push_back(idx(i,j+1,k));
						}
						if (u.is_air(i,j+1,k)) {
							yfaces.push_back(idx(i,j+1,k));
							yfaces_op.push_back(idx(i,j,k));
						}
						if (u.is_air(i,j,k-1)) {
							zfaces.push_back(idx(i,j,k));
							zfaces_op.push_back(idx(i,j,k+1));
						}
						if (u.is_air(i,j,k+1)) {
							zfaces.push_back(idx(i,j,k+1));
							zfaces_op.push_back(idx(i,j,k));
						}
					}
				
					num_xfaces = xfaces.size();
					num_yfaces = yfaces.size();
					num_zfaces = zfaces.size();
					surface_faces = num_xfaces + num_yfaces + num_zfaces;


					if (surface_faces == 1) {
						if (num_xfaces == 1) {
							if ((xfaces.front()).i > i) {
								u.vx(i+1,j,k) = u.vx(i,j,k) - u.vy(i,j+1,k) + u.vy(i,j,k) 
									        - u.vz(i,j,k+1) + u.vz(i,j,k);
							}
							else {
								u.vx(i,j,k) = u.vx(i+1,j,k) + u.vy(i,j+1,k) - u.vy(i,j,k)
									    + u.vz(i,j,k+1) - u.vz(i,j,k);
							}
							//xfaces.pop_front();
						}
						else if (num_yfaces == 1) {
							if ((yfaces.front()).j > j) {
								u.vy(i,j+1,k) = u.vx(i,j,k) - u.vx(i+1,j,k) + u.vy(i,j,k)
									      - u.vz(i,j,k+1) + u.vz(i,j,k);
							}
							else {
								u.vy(i,j,k) = u.vx(i+1,j,k) - u.vx(i,j,k) + u.vy(i,j+1,k)
									    - u.vz(i,j,k) + u.vz(i,j,k+1);
							}
							//yfaces.pop_front();
						}
						else if (num_zfaces == 1) {
							if ((zfaces.front()).k > k) {
								u.vz(i,j,k+1) = u.vx(i,j,k) - u.vx(i+1,j,k) + u.vy(i,j,k)
									      - u.vy(i,j+1,k) + u.vz(i,j,k);
							}
							else {
								u.vz(i,j,k) = u.vx(i+1,j,k) - u.vx(i,j,k) + u.vy(i,j+1,k) 
									    - u.vy(i,j,k) + u.vz(i,j,k+1);
							}
							//zfaces.pop_front();
						}
					}
					else if (surface_faces == 2)
					{
						// when the air is on two opposite sides of the cell, we do nothing (Carlson's PhD thesis for 
						// reference)
						// otherwise "copy velocities to surface-faces from the faces on the opposite side, and add
						// half the difference of the remaining two faces to each surface face" (p. 26)

						float hdiff;
						if (num_xfaces == 1 && num_yfaces == 1) {
							hdiff = 0.5*(u.vz(i,j,k+1) - u.vz(i,j,k));

							u.vx(xfaces.front()) = u.vx(xfaces_op.front());
							u.vy(yfaces.front()) = u.vy(yfaces_op.front());
							u.vx(xfaces.front()) += hdiff;
							u.vy(yfaces.front()) += hdiff;

						}
						else if (num_xfaces == 1 && num_zfaces == 1) {
							hdiff = 0.5*(u.vy(i,j+1,k) - u.vy(i,j,k));
							
							u.vx(xfaces.front()) = u.vx(xfaces_op.front());
							u.vz(zfaces.front()) = u.vz(zfaces_op.front());
							u.vx(xfaces.front()) += hdiff;
							u.vz(zfaces.front()) += hdiff;

						}
						else if (num_yfaces == 1 && num_zfaces == 1) {
							hdiff = 0.5*(u.vx(i+1,j,k) - u.vy(i,j,k));
					
							u.vy(yfaces.front()) = u.vy(yfaces_op.front());
							u.vz(zfaces.front()) = u.vz(zfaces_op.front());
							u.vy(yfaces.front()) += hdiff;
							u.vz(zfaces.front()) += hdiff;
						}
					}
					else if (surface_faces == 3) {
						// non-surface faces are across from surfaces faces ---> copy velocities from non-surface
						// to surface faces
						if (num_xfaces == 1 && num_yfaces == 1 && num_zfaces == 1) {
							u.vx(xfaces.front()) = u.vx(xfaces_op.front());
							u.vy(yfaces.front()) = u.vy(yfaces_op.front());
							u.vz(zfaces.front()) = u.vz(zfaces_op.front());
						}
						// otherwise, just solve for the three surface faces
						else if (num_yfaces == 1) {
							if ((yfaces.front()).j > j) {
								// solve for u.vy(i,j+1,k)
								u.vy(i,j+1,k) = u.vx(i,j,k) - u.vx(i+1,j,k) + u.vy(i,j,k)
									      - u.vz(i,j,k+1) + u.vz(i,j,k);
							}
							else {
								u.vy(i,j,k) = u.vx(i+1,j,k) - u.vx(i,j,k) + u.vy(i,j+1,k)
								            - u.vz(i,j,k) + u.vz(i,j,k+1);
							}
						}
						else if (num_zfaces == 1) {
							if ((zfaces.front()).k > k) {
								u.vz(i,j,k+1) = u.vx(i,j,k) - u.vx(i+1,j,k) + u.vy(i,j,k)
									      - u.vy(i,j+1,k) + u.vz(i,j,k);
							}
							else {
								u.vz(i,j,k) = u.vx(i+1,j,k) - u.vx(i,j,k) + u.vy(i,j+1,k) 
									    - u.vy(i,j,k) + u.vz(i,j,k+1);
							}

						}
						else { // num_xfaces == 1
							if ((xfaces.front()).i > i) {
								u.vx(i+1,j,k) = u.vx(i,j,k) - u.vy(i,j+1,k) + u.vy(i,j,k) 
									        - u.vz(i,j,k+1) + u.vz(i,j,k);
							}
							else {
								u.vx(i,j,k) = u.vx(i+1,j,k) + u.vy(i,j+1,k) - u.vy(i,j,k)
									    + u.vz(i,j,k+1) - u.vz(i,j,k);
							}

						}

					}
					else if (surface_faces == 4) {
						float diff;
						if (num_yfaces == 1 && num_zfaces == 1) { // num_xfaces == 2
							u.vy(yfaces.front()) = u.vy(yfaces_op.front());
							u.vz(zfaces.front()) = u.vz(zfaces_op.front());
							diff = 0.25*(u.vx(i,j,k) - u.vx(i+1,j,k));
							u.vy(yfaces.front()) += diff;
							u.vy(yfaces_op.front()) += diff;
							u.vz(zfaces.front()) += diff;
							u.vz(zfaces_op.front()) += diff;
						}
						else if (num_xfaces == 1 && num_zfaces == 1) {
							u.vx(xfaces.front()) = u.vx(xfaces_op.front());
							u.vz(zfaces.front()) = u.vz(zfaces_op.front());
							diff = 0.25*(u.vy(i,j,k) - u.vy(i,j+1,k));
							u.vx(xfaces.front()) += diff; 
							u.vx(xfaces_op.front()) += diff;
							u.vz(zfaces.front()) += diff;
							u.vz(zfaces_op.front()) += diff;

						}
						else if (num_xfaces == 1 && num_yfaces == 1) {
							u.vx(xfaces.front()) = u.vx(xfaces_op.front());
							u.vy(yfaces.front()) = u.vy(yfaces_op.front());
							diff = 0.25*(u.vz(i,j,k) - u.vz(i,j,k+1));
							u.vx(xfaces.front()) += diff;
							u.vx(xfaces_op.front()) += diff;
							u.vy(yfaces.front()) += diff;
							u.vy(yfaces_op.front()) += diff;
						}
						else if (num_xfaces == 2 && num_zfaces == 2) {

						}
						else if (num_xfaces == 2 && num_yfaces == 2) {

						}
						else { // num_yfaces == 2 && num_zfaces == 2

						}

					}


				}
			}
		}

	}
}
