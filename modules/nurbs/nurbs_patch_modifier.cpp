#include "nurbs_patch_modifier.h"

namespace module
{
	namespace nurbs
	{
		nurbs_patch_modifier::nurbs_patch_modifier(k3d::mesh& input)
		{
			try{

                m_instance = &input;

                if (m_instance->nurbs_patches == NULL)
                {
                    m_instance->nurbs_patches = boost::shared_ptr<k3d::mesh::nurbs_patches_t>(new k3d::mesh::nurbs_patches_t());
                }

                m_nurbs_patches = k3d::make_unique(m_instance->nurbs_patches);
                m_patch_first_points = k3d::make_unique(m_nurbs_patches->patch_first_points);
                m_patch_u_point_counts = k3d::make_unique(m_nurbs_patches->patch_u_point_counts);
                m_patch_v_point_counts = k3d::make_unique(m_nurbs_patches->patch_v_point_counts);
                m_patch_u_orders = k3d::make_unique(m_nurbs_patches->patch_u_orders);
                m_patch_v_orders = k3d::make_unique(m_nurbs_patches->patch_v_orders);
                m_patch_u_first_knots = k3d::make_unique(m_nurbs_patches->patch_u_first_knots);
                m_patch_v_first_knots = k3d::make_unique(m_nurbs_patches->patch_v_first_knots);
                m_patch_selection = k3d::make_unique(m_nurbs_patches->patch_selection);
                m_patch_materials = k3d::make_unique(m_nurbs_patches->patch_materials);
                m_patch_points = k3d::make_unique(m_nurbs_patches->patch_points);
                m_patch_point_weights = k3d::make_unique(m_nurbs_patches->patch_point_weights);
                m_patch_u_knots = k3d::make_unique(m_nurbs_patches->patch_u_knots);
                m_patch_v_knots = k3d::make_unique(m_nurbs_patches->patch_v_knots);

                bool not_initialized = false;

                if (!m_nurbs_patches->patch_trim_curve_loop_counts)
                {
                    not_initialized = true;
                    m_nurbs_patches->patch_first_trim_curve_loops = boost::shared_ptr<k3d::mesh::indices_t>(new k3d::mesh::indices_t());
                    m_nurbs_patches->patch_trim_curve_loop_counts = boost::shared_ptr<k3d::mesh::counts_t>(new k3d::mesh::counts_t());
                    m_nurbs_patches->patch_first_trim_curve_loops = boost::shared_ptr<k3d::mesh::indices_t>(new k3d::mesh::indices_t());
                    m_nurbs_patches->trim_points = boost::shared_ptr<k3d::mesh::points_2d_t>(new k3d::mesh::points_2d_t());
                    m_nurbs_patches->trim_point_selection = boost::shared_ptr<k3d::mesh::selection_t>(new k3d::mesh::selection_t());
                    m_nurbs_patches->first_trim_curves = boost::shared_ptr<k3d::mesh::indices_t>(new k3d::mesh::indices_t());
                    m_nurbs_patches->trim_curve_counts = boost::shared_ptr<k3d::mesh::counts_t>(new k3d::mesh::counts_t());
                    m_nurbs_patches->trim_curve_loop_selection = boost::shared_ptr<k3d::mesh::selection_t>(new k3d::mesh::selection_t());
                    m_nurbs_patches->trim_curve_first_points = boost::shared_ptr<k3d::mesh::indices_t>(new k3d::mesh::indices_t());
                    m_nurbs_patches->trim_curve_point_counts = boost::shared_ptr<k3d::mesh::counts_t>(new k3d::mesh::counts_t());
                    m_nurbs_patches->trim_curve_orders = boost::shared_ptr<k3d::mesh::orders_t>(new k3d::mesh::orders_t());
                    m_nurbs_patches->trim_curve_first_knots = boost::shared_ptr<k3d::mesh::indices_t>(new k3d::mesh::indices_t());
                    m_nurbs_patches->trim_curve_selection = boost::shared_ptr<k3d::mesh::selection_t>(new k3d::mesh::selection_t());
                    m_nurbs_patches->trim_curve_points = boost::shared_ptr<k3d::mesh::indices_t>(new k3d::mesh::indices_t());
                    m_nurbs_patches->trim_curve_point_weights = boost::shared_ptr<k3d::mesh::weights_t>(new k3d::mesh::weights_t());
                    m_nurbs_patches->trim_curve_knots = boost::shared_ptr<k3d::mesh::knots_t>(new k3d::mesh::knots_t());
                }

                m_patch_trim_curve_loop_counts = k3d::make_unique(m_nurbs_patches->patch_trim_curve_loop_counts);
                m_patch_first_trim_curve_loops = k3d::make_unique(m_nurbs_patches->patch_first_trim_curve_loops);
                m_trim_points = k3d::make_unique(m_nurbs_patches->trim_points);
                m_trim_point_selection = k3d::make_unique(m_nurbs_patches->trim_point_selection);
                m_first_trim_curves = k3d::make_unique(m_nurbs_patches->first_trim_curves);
                m_trim_curve_counts = k3d::make_unique(m_nurbs_patches->trim_curve_counts);
                m_trim_curve_loop_selection = k3d::make_unique(m_nurbs_patches->trim_curve_loop_selection);
                m_trim_curve_first_points = k3d::make_unique(m_nurbs_patches->trim_curve_first_points);
                m_trim_curve_point_counts = k3d::make_unique(m_nurbs_patches->trim_curve_point_counts);
                m_trim_curve_orders = k3d::make_unique(m_nurbs_patches->trim_curve_orders);
                m_trim_curve_first_knots = k3d::make_unique(m_nurbs_patches->trim_curve_first_knots);
                m_trim_curve_selection = k3d::make_unique(m_nurbs_patches->trim_curve_selection);
                m_trim_curve_points = k3d::make_unique(m_nurbs_patches->trim_curve_points);
                m_trim_curve_point_weights = k3d::make_unique(m_nurbs_patches->trim_curve_point_weights);
                m_trim_curve_knots = k3d::make_unique(m_nurbs_patches->trim_curve_knots);

                if(not_initialized)
                {
                    for(int i = 0; i < m_patch_first_points->size(); i++)
                    {
                        m_patch_first_trim_curve_loops->push_back(0);
                        m_patch_trim_curve_loop_counts->push_back(0);
                    }
                }


                m_mesh_points = k3d::make_unique(m_instance->points);
                m_point_selections = k3d::make_unique(m_instance->point_selection);
			}
			catch(std::exception& e)
			{
			    k3d::log() << error << nurbs_debug << "Error when creating patch modifier: " << e.what() << std::endl;
			}
			catch(...)
			{
			    k3d::log() << error << nurbs_debug << "Error when creating patch modifier" << std::endl;
			}

		}

		nurbs_curve nurbs_patch_modifier::extract_v_curve(size_t patch, size_t u)
		{
			try
			{

				nurbs_curve result;

				if (u >= m_patch_u_point_counts->at(patch))
				{
					k3d::log() << error << nurbs_debug << "Cannot extract " << u << "th curve" << std::endl;
					return nurbs_curve();
				}

				const size_t points_begin = m_patch_first_points->at(patch) + u;
				const size_t point_step =  m_patch_u_point_counts->at(patch);
				const size_t points_end = points_begin + (m_patch_v_point_counts->at(patch) - 1) * point_step + 1;

				const size_t knot_count_v = m_patch_v_point_counts->at(patch) + m_patch_v_orders->at(patch);

				const size_t knots_begin = m_patch_v_first_knots->at(patch);

				for (int i = knots_begin; i < knots_begin + knot_count_v; i++)
				{
					result.curve_knots.push_back(m_patch_v_knots->at(i));
				}

				for (int i = points_begin; i < points_end; i += point_step)
				{
					result.control_points.push_back(m_mesh_points->at( m_patch_points->at(i) ));
					result.curve_point_weights.push_back( m_patch_point_weights->at(i) );
				}

				return result;
			}
			catch (std::exception& e)
			{
				k3d::log() << error << nurbs_debug << "Error in Extract V Curve: " << e.what() << std::endl;
				return nurbs_curve();
			}
			catch (...)
			{
				k3d::log() << error << nurbs_debug << "Error in Extract V Curve" << std::endl;
				return nurbs_curve();
			}
		}

		nurbs_curve nurbs_patch_modifier::extract_u_curve(size_t patch, size_t v)
		{
			try
			{
				nurbs_curve result;

				if (v >= m_patch_v_point_counts->at(patch))
				{
					k3d::log() << error << nurbs_debug << "Cannot extract " << v << "th curve" << std::endl;
					return nurbs_curve();
				}

				const size_t points_begin = m_patch_first_points->at(patch) + v * m_patch_u_point_counts->at(patch);
				const size_t point_step = 1;
				const size_t points_end = points_begin + m_patch_u_point_counts->at(patch);

				const size_t knot_count_u = m_patch_u_point_counts->at(patch) + m_patch_u_orders->at(patch);

				const size_t knots_begin = m_patch_u_first_knots->at(patch);

				for (int i = knots_begin; i < knots_begin + knot_count_u; i++)
				{
					MY_DEBUG << "adding knot " << i << std::endl;
					result.curve_knots.push_back(m_patch_u_knots->at(i));
				}

				for (int i = points_begin; i < points_end; i += point_step)
				{
					MY_DEBUG << "adding point " << i << " of " << m_patch_points->size() << std::endl;
					result.control_points.push_back(m_mesh_points->at( m_patch_points->at(i) ));
					result.curve_point_weights.push_back( m_patch_point_weights->at(i) );
				}

				return result;
			}
			catch (std::exception& e)
			{
				k3d::log() << error << nurbs_debug << "Error in Extract U Curve: " << e.what() << " with patch = " << patch << " v = " << v << std::endl;
				return nurbs_curve();
			}
			catch (...)
			{
				k3d::log() << error << nurbs_debug << "Error in Extract U Curve" << std::endl;
				return nurbs_curve();
			}
		}

		nurbs_patch nurbs_patch_modifier::extract_patch(size_t patch)
		{
			try
			{
				MY_DEBUG << "Extracting curves" << std::endl;
				nurbs_curve u = extract_u_curve(patch, 0);
				nurbs_curve v = extract_v_curve(patch, 0);

				nurbs_patch result;
				result.u_knots = u.curve_knots;
				result.v_knots = v.curve_knots;

				result.u_order = m_patch_u_orders->at(patch);
				result.v_order = m_patch_v_orders->at(patch);

				MY_DEBUG << "Copying points & weights" << std::endl;
				for (int i = 0; i < m_patch_v_point_counts->at(patch) * m_patch_u_point_counts->at(patch); i++)
				{
					result.control_points.push_back( m_mesh_points->at( m_patch_points->at(m_patch_first_points->at(patch) + i)) );
					result.point_weights.push_back( m_patch_point_weights->at(m_patch_first_points->at(patch) + i) );
				}

				return result;
			}
			catch (std::exception& e)
			{
				MY_DEBUG << "Error in ExtractPatch: " << e.what() << std::endl;
				return nurbs_patch();
			}
			catch (...)
			{
				MY_DEBUG << "Error in ExtractPatch" << std::endl;
				return nurbs_patch();
			}
		}

		int nurbs_patch_modifier::get_patch_count()
		{
			return m_patch_first_points->size();
		}

		void nurbs_patch_modifier::insert_patch(const nurbs_patch& patch, bool share_points)
		{
			MY_DEBUG << "Inserting patch" << std::endl;
			k3d::mesh::indices_t patch_points;

			for (int i = 0; i < patch.control_points.size(); i++)
			{
				patch_points.push_back(insert_point(patch.control_points.at(i), share_points));
			}

			m_patch_first_points->push_back(m_patch_points->size());
			m_patch_selection->push_back(0.0);
			m_patch_u_first_knots->push_back(m_patch_u_knots->size());
			m_patch_v_first_knots->push_back(m_patch_v_knots->size());
			m_patch_u_orders->push_back(patch.u_order);
			m_patch_v_orders->push_back(patch.v_order);
			m_patch_u_point_counts->push_back(patch.u_knots.size() - patch.u_order);
			m_patch_v_point_counts->push_back(patch.v_knots.size() - patch.v_order);
			m_patch_materials->push_back(0);
			if (m_patch_trim_curve_loop_counts)
			{
				m_patch_trim_curve_loop_counts->push_back(0);
				m_patch_first_trim_curve_loops->push_back(0);
			}
			m_patch_points->insert(m_patch_points->end(), patch_points.begin(), patch_points.end());
			m_patch_point_weights->insert(m_patch_point_weights->end(), patch.point_weights.begin(), patch.point_weights.end());
			m_patch_u_knots->insert(m_patch_u_knots->end(), patch.u_knots.begin(), patch.u_knots.end());
			m_patch_v_knots->insert(m_patch_v_knots->end(), patch.v_knots.begin(), patch.v_knots.end());
		}

		size_t nurbs_patch_modifier::insert_point(const k3d::point3& point, bool shared)
		{
			try
			{
				//MY_DEBUG << "Inserting points" << std::endl;
				int found = -1;

				for (int j = 0; j < m_mesh_points->size(); j++)
				{
					if (nurbs_curve_modifier::point3_float_equal(m_mesh_points->at(j), point, 0.000001))
					{
						//MY_DEBUG << "Found point" << std::endl;
						found = j;
					}
				}

				if (found < 0 || !shared)
				{
					//MY_DEBUG << "Need to add the point to mesh" << std::endl;
					//we need to insert the point
					m_mesh_points->push_back(point);
					m_point_selections->push_back(0.0);
					return m_mesh_points->size() - 1;
				}
				else return found;
			}
			catch (std::exception& e)
			{
				MY_DEBUG << "Error in InsertPoint: " << e.what() << std::endl;
				return 0;
			}
			catch (...)
			{
				MY_DEBUG << "Error in InsertPoint" << std::endl;
				return 0;
			}
		}

		void nurbs_patch_modifier::patch_u_knot_insertion(size_t patch, double u, size_t multiplicity)
		{
			try
			{
				std::vector<nurbs_curve> curves;
				k3d::mesh::indices_t new_points;
				k3d::mesh::weights_t new_weights;

				for (int i = 0; i < m_patch_v_point_counts->at(patch); i++)
				{
					nurbs_curve c = extract_u_curve(patch, i);
					k3d::mesh temp;
					nurbs_curve_modifier mod(temp);
					mod.add_curve(c, false);
					mod.normalize_knot_vector(0);
					mod.curve_knot_insertion(0, u, multiplicity);
					curves.push_back(mod.extract_curve(0));

					for (int j = 0; j < curves.back().control_points.size(); j++)
					{
						new_points.push_back(insert_point(curves.back().control_points.at(j), true));
						new_weights.push_back(curves.back().curve_point_weights.at(j));
					}
				}

				const int point_count = m_patch_u_point_counts->at(patch) * m_patch_v_point_counts->at(patch);
				const int point_offset = new_points.size() - point_count;
				//insert points
				k3d::mesh::indices_t::iterator points_begin = m_patch_points->begin() + m_patch_first_points->at(patch);
				k3d::mesh::indices_t::iterator points_end = m_patch_points->begin() + m_patch_first_points->at(patch) + point_count;
				m_patch_points->erase(points_begin, points_end);

				points_begin = m_patch_points->begin() + m_patch_first_points->at(patch);

				m_patch_points->insert(points_begin, new_points.begin(), new_points.end());

				//insert weights
				k3d::mesh::weights_t::iterator weights_begin = m_patch_point_weights->begin() + m_patch_first_points->at(patch);
				k3d::mesh::weights_t::iterator weights_end = m_patch_point_weights->begin() + m_patch_first_points->at(patch) + point_count;
				m_patch_point_weights->erase(weights_begin, weights_end);

				weights_begin = m_patch_point_weights->begin() + m_patch_first_points->at(patch);

				m_patch_point_weights->insert(weights_begin, new_weights.begin(), new_weights.end());

				//offset all first_points which start behind this one
				for (int i = 0; i < m_patch_first_points->size(); i++)
				{
					if (m_patch_first_points->at(i) > m_patch_first_points->at(patch))
						m_patch_first_points->at(i) += point_offset;
				}

				//add the new knot vector
				const int knot_count = m_patch_u_point_counts->at(patch) + m_patch_u_orders->at(patch);
				const int knot_offset = curves.back().curve_knots.size() - knot_count;

				k3d::mesh::knots_t::iterator knots_begin = m_patch_u_knots->begin() + m_patch_u_first_knots->at(patch);
				k3d::mesh::knots_t::iterator knots_end = m_patch_u_knots->begin() + m_patch_u_first_knots->at(patch) + knot_count;
				m_patch_u_knots->erase(knots_begin, knots_end);

				knots_begin = m_patch_u_knots->begin() + m_patch_u_first_knots->at(patch);
				m_patch_u_knots->insert(knots_begin, curves.back().curve_knots.begin(), curves.back().curve_knots.end());

				m_patch_u_point_counts->at(patch) += knot_offset;//we're inserting the same number of points and knots

				for (int i = 0; i < m_patch_u_first_knots->size(); i++)
				{
					if (m_patch_u_first_knots->at(i) > m_patch_u_first_knots->at(patch))
						m_patch_u_first_knots->at(i) += knot_offset;
				}
			}
			catch (...)
			{
				k3d::log() << error << nurbs_debug << "Error in Patch_u_knot_insertion" << std::endl;
			}
		}

		void nurbs_patch_modifier::patch_v_knot_insertion(size_t patch, double v, size_t multiplicity)
		{
			try
			{
				std::vector<nurbs_curve> curves;
				k3d::mesh::indices_t new_points;
				k3d::mesh::weights_t new_weights;

				for (int i = 0; i < m_patch_u_point_counts->at(patch); i++)
				{
					nurbs_curve c = extract_v_curve(patch, i);
					k3d::mesh temp;
					nurbs_curve_modifier mod(temp);
					mod.add_curve(c, false);
					mod.normalize_knot_vector(0);
					mod.curve_knot_insertion(0, v, multiplicity);
					curves.push_back(mod.extract_curve(0));


				}

				for (int j = 0; j < curves.back().control_points.size(); j++)
				{
					for (int i = 0; i < curves.size(); i++)
					{
						new_points.push_back(insert_point(curves.at(i).control_points.at(j), true));
						new_weights.push_back(curves.at(i).curve_point_weights.at(j));
					}
				}

				const int point_count = m_patch_u_point_counts->at(patch) * m_patch_v_point_counts->at(patch);
				const int point_offset = new_points.size() - point_count;
				//insert points
				k3d::mesh::indices_t::iterator points_begin = m_patch_points->begin() + m_patch_first_points->at(patch);
				k3d::mesh::indices_t::iterator points_end = m_patch_points->begin() + m_patch_first_points->at(patch) + point_count;
				m_patch_points->erase(points_begin, points_end);

				points_begin = m_patch_points->begin() + m_patch_first_points->at(patch);

				m_patch_points->insert(points_begin, new_points.begin(), new_points.end());
				//insert weights
				k3d::mesh::weights_t::iterator weights_begin = m_patch_point_weights->begin() + m_patch_first_points->at(patch);
				k3d::mesh::weights_t::iterator weights_end = m_patch_point_weights->begin() + m_patch_first_points->at(patch) + point_count;
				m_patch_point_weights->erase(weights_begin, weights_end);

				weights_begin = m_patch_point_weights->begin() + m_patch_first_points->at(patch);

				m_patch_point_weights->insert(weights_begin, new_weights.begin(), new_weights.end());

				//offset all first_points which start behind this one
				for (int i = 0; i < m_patch_first_points->size(); i++)
				{
					if (m_patch_first_points->at(i) > m_patch_first_points->at(patch))
						m_patch_first_points->at(i) += point_offset;
				}

				//add the new knot vector
				const int knot_count = m_patch_v_point_counts->at(patch) + m_patch_v_orders->at(patch);
				const int knot_offset = curves.back().curve_knots.size() - knot_count;

				k3d::mesh::knots_t::iterator knots_begin = m_patch_v_knots->begin() + m_patch_v_first_knots->at(patch);
				k3d::mesh::knots_t::iterator knots_end = m_patch_v_knots->begin() + m_patch_v_first_knots->at(patch) + knot_count;
				m_patch_v_knots->erase(knots_begin, knots_end);

				knots_begin = m_patch_v_knots->begin() + m_patch_v_first_knots->at(patch);
				m_patch_v_knots->insert(knots_begin, curves.back().curve_knots.begin(), curves.back().curve_knots.end());

				m_patch_v_point_counts->at(patch) += knot_offset;//we're inserting the same number of points and knots

				for (int i = 0; i < m_patch_v_first_knots->size(); i++)
				{
					if (m_patch_v_first_knots->at(i) > m_patch_v_first_knots->at(patch))
						m_patch_v_first_knots->at(i) += knot_offset;
				}
			}
			catch (...)
			{
				k3d::log() << error << nurbs_debug << "Error in Patch_v_knot_insertion" << std::endl;
			}
		}

		void nurbs_patch_modifier::patch_u_degree_elevation(size_t patch, size_t degree)
		{
			try
			{
				std::vector<nurbs_curve> curves;
				k3d::mesh::indices_t new_points;
				k3d::mesh::weights_t new_weights;

				for (int i = 0; i < m_patch_v_point_counts->at(patch); i++)
				{
					nurbs_curve c = extract_u_curve(patch, i);
					k3d::mesh temp;
					nurbs_curve_modifier mod(temp);
					mod.add_curve(c, false);
					mod.normalize_knot_vector(0);
					for (int i = 0; i < degree; i++)
					{
						mod.curve_degree_elevate(0);
					}
					curves.push_back(mod.extract_curve(0));

					for (int j = 0; j < curves.back().control_points.size(); j++)
					{
						new_points.push_back(insert_point(curves.back().control_points.at(j), true));
						new_weights.push_back(curves.back().curve_point_weights.at(j));
					}
				}

				MY_DEBUG << "Adding new curves to patch" << std::endl;

				const int point_count = m_patch_u_point_counts->at(patch) * m_patch_v_point_counts->at(patch);
				const int point_offset = new_points.size() - point_count;
				//insert points
				k3d::mesh::indices_t::iterator points_begin = m_patch_points->begin() + m_patch_first_points->at(patch);
				k3d::mesh::indices_t::iterator points_end = m_patch_points->begin() + m_patch_first_points->at(patch) + point_count;
				m_patch_points->erase(points_begin, points_end);

				points_begin = m_patch_points->begin() + m_patch_first_points->at(patch);

				m_patch_points->insert(points_begin, new_points.begin(), new_points.end());

				//insert weights
				k3d::mesh::weights_t::iterator weights_begin = m_patch_point_weights->begin() + m_patch_first_points->at(patch);
				k3d::mesh::weights_t::iterator weights_end = m_patch_point_weights->begin() + m_patch_first_points->at(patch) + point_count;
				m_patch_point_weights->erase(weights_begin, weights_end);

				weights_begin = m_patch_point_weights->begin() + m_patch_first_points->at(patch);

				m_patch_point_weights->insert(weights_begin, new_weights.begin(), new_weights.end());

				//offset all first_points which start behind this one
				for (int i = 0; i < m_patch_first_points->size(); i++)
				{
					if (m_patch_first_points->at(i) > m_patch_first_points->at(patch))
						m_patch_first_points->at(i) += point_offset;
				}

				//add the new knot vector
				const int knot_count = m_patch_u_point_counts->at(patch) + m_patch_u_orders->at(patch);
				const int knot_offset = curves.back().curve_knots.size() - knot_count;

				k3d::mesh::knots_t::iterator knots_begin = m_patch_u_knots->begin() + m_patch_u_first_knots->at(patch);
				k3d::mesh::knots_t::iterator knots_end = m_patch_u_knots->begin() + m_patch_u_first_knots->at(patch) + knot_count;
				m_patch_u_knots->erase(knots_begin, knots_end);

				knots_begin = m_patch_u_knots->begin() + m_patch_u_first_knots->at(patch);
				m_patch_u_knots->insert(knots_begin, curves.back().curve_knots.begin(), curves.back().curve_knots.end());

				m_patch_u_orders->at(patch) += degree;
				m_patch_u_point_counts->at(patch) = curves.back().curve_knots.size() - m_patch_u_orders->at(patch);

				for (int i = 0; i < m_patch_u_first_knots->size(); i++)
				{
					if (m_patch_u_first_knots->at(i) > m_patch_u_first_knots->at(patch))
						m_patch_u_first_knots->at(i) += knot_offset;
				}
				MY_DEBUG << "Finished modifying patch" << std::endl;
			}
			catch (...)
			{
				k3d::log() << error << nurbs_debug << "Error in Patch_u_degree_elevation" << std::endl;
			}
		}

		void nurbs_patch_modifier::patch_v_degree_elevation(size_t patch, size_t degree)
		{
			try
			{
				std::vector<nurbs_curve> curves;
				k3d::mesh::indices_t new_points;
				k3d::mesh::weights_t new_weights;

				for (int i = 0; i < m_patch_u_point_counts->at(patch); i++)
				{
					nurbs_curve c = extract_v_curve(patch, i);
					k3d::mesh temp;
					nurbs_curve_modifier mod(temp);
					mod.add_curve(c, false);
					mod.normalize_knot_vector(0);
					for (int i = 0; i < degree; i++)
					{
						mod.curve_degree_elevate(0);
					}
					curves.push_back(mod.extract_curve(0));
				}

				for (int j = 0; j < curves.back().control_points.size(); j++)
				{
					for (int i = 0; i < curves.size(); i++)
					{
						new_points.push_back(insert_point(curves.at(i).control_points.at(j), true));
						new_weights.push_back(curves.at(i).curve_point_weights.at(j));
					}
				}

				const int point_count = m_patch_u_point_counts->at(patch) * m_patch_v_point_counts->at(patch);
				const int point_offset = new_points.size() - point_count;
				//insert points
				k3d::mesh::indices_t::iterator points_begin = m_patch_points->begin() + m_patch_first_points->at(patch);
				k3d::mesh::indices_t::iterator points_end = m_patch_points->begin() + m_patch_first_points->at(patch) + point_count;
				m_patch_points->erase(points_begin, points_end);

				points_begin = m_patch_points->begin() + m_patch_first_points->at(patch);

				m_patch_points->insert(points_begin, new_points.begin(), new_points.end());
				//insert weights
				k3d::mesh::weights_t::iterator weights_begin = m_patch_point_weights->begin() + m_patch_first_points->at(patch);
				k3d::mesh::weights_t::iterator weights_end = m_patch_point_weights->begin() + m_patch_first_points->at(patch) + point_count;
				m_patch_point_weights->erase(weights_begin, weights_end);

				weights_begin = m_patch_point_weights->begin() + m_patch_first_points->at(patch);

				m_patch_point_weights->insert(weights_begin, new_weights.begin(), new_weights.end());

				//offset all first_points which start behind this one
				for (int i = 0; i < m_patch_first_points->size(); i++)
				{
					if (m_patch_first_points->at(i) > m_patch_first_points->at(patch))
						m_patch_first_points->at(i) += point_offset;
				}

				//add the new knot vector
				const int knot_count = m_patch_v_point_counts->at(patch) + m_patch_v_orders->at(patch);
				const int knot_offset = curves.back().curve_knots.size() - knot_count;

				k3d::mesh::knots_t::iterator knots_begin = m_patch_v_knots->begin() + m_patch_v_first_knots->at(patch);
				k3d::mesh::knots_t::iterator knots_end = m_patch_v_knots->begin() + m_patch_v_first_knots->at(patch) + knot_count;
				m_patch_v_knots->erase(knots_begin, knots_end);

				knots_begin = m_patch_v_knots->begin() + m_patch_v_first_knots->at(patch);
				m_patch_v_knots->insert(knots_begin, curves.back().curve_knots.begin(), curves.back().curve_knots.end());

				m_patch_v_orders->at(patch) += degree;
				m_patch_v_point_counts->at(patch) = curves.back().curve_knots.size() - m_patch_v_orders->at(patch);


				for (int i = 0; i < m_patch_v_first_knots->size(); i++)
				{
					if (m_patch_v_first_knots->at(i) > m_patch_v_first_knots->at(patch))
						m_patch_v_first_knots->at(i) += knot_offset;
				}
			}
			catch (...)
			{
				k3d::log() << error << nurbs_debug << "Error in Patch_v_degree_elevation" << std::endl;
			}
		}

		void nurbs_patch_modifier::split_patch_u(size_t patch, double u)
		{
			try
			{
				std::vector<nurbs_curve> curves;
				std::vector<nurbs_curve> split_curves;
				k3d::mesh::indices_t new_points;
				k3d::mesh::points_t split_points;
				k3d::mesh::weights_t new_weights;
				k3d::mesh::weights_t split_weights;

				for (int i = 0; i < m_patch_v_point_counts->at(patch); i++)
				{
					nurbs_curve c = extract_u_curve(patch, i);
					k3d::mesh temp;
					nurbs_curve_modifier mod(temp);
					mod.add_curve(c, false);
					mod.normalize_knot_vector(0);
					mod.split_curve_at(0, u);
					curves.push_back(mod.extract_curve(0));
					split_curves.push_back(mod.extract_curve(1));

					for (int j = 0; j < curves.back().control_points.size(); j++)
					{
						new_points.push_back(insert_point(curves.back().control_points.at(j), true));
						new_weights.push_back(curves.back().curve_point_weights.at(j));
					}

					for (int j = 0; j < split_curves.back().control_points.size(); j++)
					{
						split_points.push_back(split_curves.back().control_points.at(j));
						split_weights.push_back(split_curves.back().curve_point_weights.at(j));
					}
				}

				MY_DEBUG << "Adding new curves to patch" << std::endl;

				const int point_count = m_patch_u_point_counts->at(patch) * m_patch_v_point_counts->at(patch);
				const int point_offset = point_count - new_points.size();
				//insert points
				k3d::mesh::indices_t::iterator points_begin = m_patch_points->begin() + m_patch_first_points->at(patch);
				k3d::mesh::indices_t::iterator points_end = m_patch_points->begin() + m_patch_first_points->at(patch) + point_count;
				m_patch_points->erase(points_begin, points_end);

				points_begin = m_patch_points->begin() + m_patch_first_points->at(patch);

				m_patch_points->insert(points_begin, new_points.begin(), new_points.end());

				//insert weights
				k3d::mesh::weights_t::iterator weights_begin = m_patch_point_weights->begin() + m_patch_first_points->at(patch);
				k3d::mesh::weights_t::iterator weights_end = m_patch_point_weights->begin() + m_patch_first_points->at(patch) + point_count;
				m_patch_point_weights->erase(weights_begin, weights_end);

				weights_begin = m_patch_point_weights->begin() + m_patch_first_points->at(patch);

				m_patch_point_weights->insert(weights_begin, new_weights.begin(), new_weights.end());

				//offset all first_points which start behind this one
				for (int i = 0; i < m_patch_first_points->size(); i++)
				{
					if (m_patch_first_points->at(i) > m_patch_first_points->at(patch))
						m_patch_first_points->at(i) -= point_offset;
				}

				//add the new knot vector
				const int knot_count = m_patch_u_point_counts->at(patch) + m_patch_u_orders->at(patch);
				const int knot_offset = knot_count - curves.back().curve_knots.size();

				k3d::mesh::knots_t::iterator knots_begin = m_patch_u_knots->begin() + m_patch_u_first_knots->at(patch);
				k3d::mesh::knots_t::iterator knots_end = m_patch_u_knots->begin() + m_patch_u_first_knots->at(patch) + knot_count;
				m_patch_u_knots->erase(knots_begin, knots_end);

				knots_begin = m_patch_u_knots->begin() + m_patch_u_first_knots->at(patch);
				m_patch_u_knots->insert(knots_begin, curves.back().curve_knots.begin(), curves.back().curve_knots.end());

				m_patch_u_point_counts->at(patch) = curves.back().curve_knots.size() - m_patch_u_orders->at(patch);

				for (int i = 0; i < m_patch_u_first_knots->size(); i++)
				{
					if (m_patch_u_first_knots->at(i) > m_patch_u_first_knots->at(patch))
						m_patch_u_first_knots->at(i) -= knot_offset;
				}
				MY_DEBUG << "Finished modifying patch" << std::endl;

				//add new patch
				nurbs_patch split_patch;
				split_patch.control_points = split_points;
				split_patch.point_weights = split_weights;

				k3d::mesh::knots_t::iterator vknots_begin = m_patch_v_knots->begin() + m_patch_v_first_knots->at(patch);
				k3d::mesh::knots_t::iterator vknots_end = m_patch_v_knots->begin() + m_patch_v_first_knots->at(patch) + m_patch_v_point_counts->at(patch) + m_patch_v_orders->at(patch);

				split_patch.v_knots.insert(split_patch.v_knots.begin(), vknots_begin, vknots_end);
				split_patch.v_order = m_patch_v_orders->at(patch);
				split_patch.u_knots = split_curves.back().curve_knots;
				split_patch.u_order = m_patch_u_orders->at(patch);

				insert_patch(split_patch, false);
			}
			catch (...)
			{
				k3d::log() << error << nurbs_debug << "Error in split_patch_u" << std::endl;
			}
		}

		void nurbs_patch_modifier::split_patch_v(size_t patch, double v)
		{
			try
			{
				std::vector<nurbs_curve> curves;
				std::vector<nurbs_curve> split_curves;
				k3d::mesh::indices_t new_points;
				k3d::mesh::points_t split_points;
				k3d::mesh::weights_t new_weights;
				k3d::mesh::weights_t split_weights;

				for (int i = 0; i < m_patch_u_point_counts->at(patch); i++)
				{
					nurbs_curve c = extract_v_curve(patch, i);
					k3d::mesh temp;
					nurbs_curve_modifier mod(temp);
					mod.add_curve(c, false);
					mod.normalize_knot_vector(0);
					mod.split_curve_at(0, v);

					curves.push_back(mod.extract_curve(0));
					split_curves.push_back(mod.extract_curve(1));
				}

				for (int j = 0; j < curves.back().control_points.size(); j++)
				{
					for (int i = 0; i < curves.size(); i++)
					{
						new_points.push_back(insert_point(curves.at(i).control_points.at(j), true));
						new_weights.push_back(curves.at(i).curve_point_weights.at(j));
					}
				}

				for (int j = 0; j < split_curves.back().control_points.size(); j++)
				{
					for (int i = 0; i < split_curves.size(); i++)
					{
						split_points.push_back(split_curves.at(i).control_points.at(j));
						split_weights.push_back(split_curves.at(i).curve_point_weights.at(j));
					}
				}

				const int point_count = m_patch_u_point_counts->at(patch) * m_patch_v_point_counts->at(patch);
				const int point_offset = point_count - new_points.size();
				//insert points
				k3d::mesh::indices_t::iterator points_begin = m_patch_points->begin() + m_patch_first_points->at(patch);
				k3d::mesh::indices_t::iterator points_end = m_patch_points->begin() + m_patch_first_points->at(patch) + point_count;
				m_patch_points->erase(points_begin, points_end);

				points_begin = m_patch_points->begin() + m_patch_first_points->at(patch);

				m_patch_points->insert(points_begin, new_points.begin(), new_points.end());
				//insert weights
				k3d::mesh::weights_t::iterator weights_begin = m_patch_point_weights->begin() + m_patch_first_points->at(patch);
				k3d::mesh::weights_t::iterator weights_end = m_patch_point_weights->begin() + m_patch_first_points->at(patch) + point_count;
				m_patch_point_weights->erase(weights_begin, weights_end);

				weights_begin = m_patch_point_weights->begin() + m_patch_first_points->at(patch);

				m_patch_point_weights->insert(weights_begin, new_weights.begin(), new_weights.end());

				//offset all first_points which start behind this one
				for (int i = 0; i < m_patch_first_points->size(); i++)
				{
					if (m_patch_first_points->at(i) > m_patch_first_points->at(patch))
						m_patch_first_points->at(i) -= point_offset;
				}

				//add the new knot vector
				const int knot_count = m_patch_v_point_counts->at(patch) + m_patch_v_orders->at(patch);
				const int knot_offset = knot_count - curves.back().curve_knots.size();

				k3d::mesh::knots_t::iterator knots_begin = m_patch_v_knots->begin() + m_patch_v_first_knots->at(patch);
				k3d::mesh::knots_t::iterator knots_end = m_patch_v_knots->begin() + m_patch_v_first_knots->at(patch) + knot_count;
				m_patch_v_knots->erase(knots_begin, knots_end);

				knots_begin = m_patch_v_knots->begin() + m_patch_v_first_knots->at(patch);
				m_patch_v_knots->insert(knots_begin, curves.back().curve_knots.begin(), curves.back().curve_knots.end());

				m_patch_v_point_counts->at(patch) = curves.back().curve_knots.size() - m_patch_v_orders->at(patch);


				for (int i = 0; i < m_patch_v_first_knots->size(); i++)
				{
					if (m_patch_v_first_knots->at(i) > m_patch_v_first_knots->at(patch))
						m_patch_v_first_knots->at(i) -= knot_offset;
				}

				//add new patch
				nurbs_patch split_patch;
				split_patch.control_points = split_points;
				split_patch.point_weights = split_weights;

				k3d::mesh::knots_t::iterator uknots_begin = m_patch_u_knots->begin() + m_patch_u_first_knots->at(patch);
				k3d::mesh::knots_t::iterator uknots_end = m_patch_u_knots->begin() + m_patch_u_first_knots->at(patch) + m_patch_u_point_counts->at(patch) + m_patch_u_orders->at(patch);

				split_patch.u_knots.insert(split_patch.u_knots.begin(), uknots_begin, uknots_end);
				split_patch.u_order = m_patch_u_orders->at(patch);
				split_patch.v_knots = split_curves.back().curve_knots;
				split_patch.v_order = m_patch_v_orders->at(patch);

				insert_patch(split_patch, false);
			}
			catch (...)
			{
				k3d::log() << error << nurbs_debug << "Error in split_patch_v" << std::endl;
			}
		}

		int nurbs_patch_modifier::get_selected_patch()
		{
			int selection = -1;
			for (int i = 0; i < m_patch_selection->size(); i++)
			{
				if (m_patch_selection->at(i) > 0.0)
				{
					if (selection < 0)
					{
						selection = i;
					}
					else
					{
						return -1;
					}
				}
			}

			return selection;
		}

		std::vector<size_t> nurbs_patch_modifier::get_selected_patches()
		{
			std::vector<size_t> selection;
			for (int i = 0; i < m_patch_selection->size(); i++)
			{
				if (m_patch_selection->at(i) > 0.0)
				{
					selection.push_back(i);
				}
			}

			return selection;
		}

		void nurbs_patch_modifier::add_trim_curve(size_t patch, size_t curve, double scale, double offset_u, double offset_v)
		{
			try{
			    MY_DEBUG << "Adding trim curve: patch=" << patch << " curve=" << curve << std::endl;
				nurbs_curve_modifier mod(*m_instance);
				nurbs_trim_curve tc = mod.create_trim_curve(curve);

                if(m_patch_first_trim_curve_loops == NULL)
                {
                    k3d::log() << error << nurbs_debug << "Patch first trim curve loops was NULL" << std::endl;
                    return;
                }

				if (m_patch_trim_curve_loop_counts->at(patch) == 0)
					m_patch_first_trim_curve_loops->at(patch) = m_first_trim_curves->size();
				m_patch_trim_curve_loop_counts->at(patch)++;
				while(m_trim_curve_loop_selection->size() < patch + 1)
                    m_trim_curve_loop_selection->push_back(0.0);

                MY_DEBUG << "Copying data" << std::endl;
				m_first_trim_curves->push_back(m_trim_curve_first_points->size());
                m_trim_curve_counts->push_back(1);
				m_trim_curve_first_knots->push_back(m_trim_curve_knots->size());
				m_trim_curve_knots->insert(m_trim_curve_knots->end(), tc.curve_knots.begin(), tc.curve_knots.end());
				m_trim_curve_first_points->push_back(m_trim_curve_points->size());
				m_trim_curve_point_weights->insert(m_trim_curve_point_weights->end(), tc.curve_point_weights.begin(), tc.curve_point_weights.end());
				m_trim_curve_orders->push_back(tc.curve_knots.size() - tc.control_points.size());
				m_trim_curve_point_counts->push_back(tc.control_points.size());
				m_trim_curve_selection->push_back(0.0);

                //map control points to u x v = [0,1] x [0,1]??
                MY_DEBUG << "Copying points" << std::endl;

				for (int i = 0; i < tc.control_points.size(); i++)
				{
					m_trim_curve_points->push_back(m_trim_points->size());
					m_trim_point_selection->push_back(0.0);

					k3d::point2 p(tc.control_points.at(i)[0] * scale + offset_u, tc.control_points.at(i)[1] * scale + offset_v);

					m_trim_points->push_back(p);
				}
			}
			catch (...)
			{
				k3d::log() << error << nurbs_debug << "Error in add_trim_curve" << std::endl;
			}
		}

		void nurbs_patch_modifier::select_patch(size_t patch)
		{
		    for(int i = 0; i < get_patch_count(); i++)
		    {
		        if(i!= patch)
                    m_patch_selection->at(i) = 0.0;
                else
                    m_patch_selection->at(i) = 1.0;
		    }
		}
	}
}
