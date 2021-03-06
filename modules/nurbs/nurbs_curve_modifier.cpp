#include "nurbs_curve_modifier.h"

#include <k3dsdk/linear_curve.h>
#include <k3dsdk/metadata.h>
#include <k3dsdk/nurbs_curve.h>

#include <boost/scoped_ptr.hpp>

namespace module
{

namespace nurbs
{

k3d::nurbs_curve::primitive* get_first_nurbs_curve(k3d::mesh& Mesh)
{
	for(k3d::mesh::primitives_t::iterator primitive = Mesh.primitives.begin(); primitive != Mesh.primitives.end(); ++primitive)
	{
		if(primitive->get() && primitive->get()->type == "nurbs_curve")
		{
			return k3d::nurbs_curve::validate(primitive->writable());
		}
	}
}

k3d::nurbs_curve::const_primitive* get_first_nurbs_curve(const k3d::mesh& Mesh)
{
	for(k3d::mesh::primitives_t::const_iterator primitive = Mesh.primitives.begin(); primitive != Mesh.primitives.end(); ++primitive)
	{
		if(primitive->get() && primitive->get()->type == "nurbs_curve")
		{
			return k3d::nurbs_curve::validate(**primitive);
		}
	}
}

nurbs_curve_modifier::nurbs_curve_modifier(k3d::mesh& input, k3d::nurbs_curve::primitive& Curve) : m_instance(input), m_nurbs_curve(Curve)
{
	if (input.points.get())
	{
		point_selection = &input.point_selection.writable();
		mesh_points = &input.points.writable();
	}
	else
	{
		point_selection = &input.point_selection.create();
		mesh_points = &input.points.create();
	}
	
	if(m_nurbs_curve.material.empty())
		m_nurbs_curve.material.push_back(0);

}

nurbs_curve_modifier::~nurbs_curve_modifier()
{
	k3d::mesh::delete_unused_points(m_instance);
}

int nurbs_curve_modifier::add_curve(nurbs_curve& curve, bool shared)
{
	try
	{
		MY_DEBUG << "ADD_CURVE" << std::endl;
		
		MY_DEBUG << "curve count: " << m_nurbs_curve.curve_first_points.size() << std::endl;
		if(!m_nurbs_curve.curve_point_counts.empty()) k3d::log() << debug << "point count: " << m_nurbs_curve.curve_point_counts.front() << std::endl; 

		m_nurbs_curve.curve_first_points.push_back(m_nurbs_curve.curve_points.size());
		k3d::log() << debug << "added first points" << std::endl;
		m_nurbs_curve.curve_first_knots.push_back(m_nurbs_curve.curve_knots.size());
		k3d::log() << debug << "added first knots" << std::endl;
		m_nurbs_curve.curve_orders.push_back(curve.curve_knots.size() - curve.control_points.size());
		k3d::log() << debug << "added curve orders" << std::endl;
		m_nurbs_curve.curve_point_counts.push_back(curve.control_points.size());
		k3d::log() << debug << "added point couts" << std::endl;
		m_nurbs_curve.curve_knots.insert(m_nurbs_curve.curve_knots.end(), curve.curve_knots.begin(), curve.curve_knots.end());
		k3d::log() << debug << "added curve knots" << std::endl;
		m_nurbs_curve.curve_point_weights.insert(m_nurbs_curve.curve_point_weights.end(), curve.curve_point_weights.begin(), curve.curve_point_weights.end());
		k3d::log() << debug << "added point weights" << std::endl;
		m_nurbs_curve.curve_selections.push_back(0.0);
		
		MY_DEBUG << "add curve: done pushing arrays" << std::endl;

		k3d::mesh::indices_t points;

		for (int i = 0; i < curve.control_points.size(); i++)
		{
			points.push_back(insert_point(curve.control_points.at(i), shared));
		}

		m_nurbs_curve.curve_points.insert(m_nurbs_curve.curve_points.end(), points.begin(), points.end());
		
		MY_DEBUG << "done inserting points" << std::endl;

		return count_all_curves_in_groups() - 1;
	}
	catch (...)
	{
		k3d::log() << error << nurbs_debug << "Error in addCurve" << std::endl;
		return -1;
	}
}

k3d::uint_t nurbs_curve_modifier::insert_point(k3d::point3& point, bool shared)
{
	try
	{
		int found = -1;

		for (int j = 0; j < mesh_points->size(); j++)
		{
			if (point3_float_equal(mesh_points->at(j), point, 0.000001))
			{
				found = j;
			}
		}

		if (found < 0 || !shared)
		{
			//we need to insert the point
			mesh_points->push_back(point);
			point_selection->push_back(0.0);
			return mesh_points->size() - 1;
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

int nurbs_curve_modifier::selected_curve()
{
	int my_curve = -1;

	const k3d::uint_t curve_begin = 0;
	const k3d::uint_t curve_end = m_nurbs_curve.curve_first_points.size();
	for (k3d::uint_t curve = curve_begin; curve != curve_end; ++curve)
	{
		if (m_nurbs_curve.curve_selections.at(curve) > 0.0)
			if (my_curve >= 0)
			{
				return -1;
			}
			else
			{
				my_curve = curve;
			}
	}
	return my_curve;
}

std::vector<k3d::uint_t> nurbs_curve_modifier::selected_curves()
{
	std::vector<k3d::uint_t> my_curves;

	const k3d::uint_t curve_begin = 0;
	const k3d::uint_t curve_end = m_nurbs_curve.curve_first_points.size();
	for (k3d::uint_t curve = curve_begin; curve != curve_end; ++curve)
	{
		if (m_nurbs_curve.curve_selections.at(curve) > 0.0)
			my_curves.push_back(curve);
	}
	return my_curves;
}

void nurbs_curve_modifier::print_knot_vector(k3d::uint_t curve)
{
	const k3d::uint_t curve_knots_begin = m_nurbs_curve.curve_first_knots.at(curve);
	const k3d::uint_t curve_knots_end = curve_knots_begin + m_nurbs_curve.curve_point_counts.at(curve) + m_nurbs_curve.curve_orders.at(curve);

	std::stringstream str;
	str << "Knot vector:";
	for (k3d::uint_t knot = curve_knots_begin; knot < curve_knots_end; knot++)
	{
		str << " " << m_nurbs_curve.curve_knots.at(knot);
	}
	MY_DEBUG << str.str() << std::endl;
}

void nurbs_curve_modifier::replace_point(k3d::uint_t newIndex, k3d::uint_t curve, k3d::uint_t point, bool continuous)
{
	if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Replace Point started on curve " << curve << " and point " << point << std::endl;
	const k3d::uint_t curve_point_begin = m_nurbs_curve.curve_first_points.at(curve);
	const k3d::uint_t curve_point_end = curve_point_begin + m_nurbs_curve.curve_point_counts.at(curve);

	for (k3d::uint_t points = curve_point_begin; points < curve_point_end; ++points)
	{
		if (m_nurbs_curve.curve_points.at(points) == point)
		{
			if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Found point, replacing by" << newIndex << std::endl;
			//we found the index pointing to point1
			m_nurbs_curve.curve_points.at(points) = newIndex;
			if (continuous)
			{

				const k3d::uint_t curve_knots_begin = m_nurbs_curve.curve_first_knots.at(curve);
				const k3d::uint_t curve_knots_end = curve_knots_begin + curve_point_end - curve_point_begin + m_nurbs_curve.curve_orders.at(curve);

				const k3d::uint_t order = m_nurbs_curve.curve_orders.at(curve);
				const k3d::uint_t half_order = static_cast<k3d::uint_t>(floor(0.5 * order));
				const k3d::uint_t pos = half_order + (points - curve_point_begin) + curve_knots_begin;
				float knot_at_pos = m_nurbs_curve.curve_knots.at(points - curve_point_begin + curve_knots_begin + half_order);

				if (pos - curve_knots_begin < order)
				{
					for (k3d::uint_t x = curve_knots_begin; x < order + curve_knots_begin; ++x)
						m_nurbs_curve.curve_knots.at(x) = knot_at_pos;
					k3d::uint_t knot_pos = order + curve_knots_begin;
					while ((m_nurbs_curve.curve_knots.at(knot_pos + 1) - m_nurbs_curve.curve_knots.at(knot_pos) > 2) && (knot_pos < curve_knots_end - 1))
					{
						m_nurbs_curve.curve_knots.at(knot_pos + 1) = m_nurbs_curve.curve_knots.at(knot_pos) + 2;
						knot_pos++;
					}
				}
				else if (pos - curve_knots_begin + order < curve_knots_end)
				{
					for (k3d::uint_t x = curve_knots_end - 1; x > curve_knots_end - order; --x)
						m_nurbs_curve.curve_knots.at(x) = knot_at_pos;
					k3d::uint_t knot_pos = curve_knots_end - order;
					while ((m_nurbs_curve.curve_knots.at(knot_pos) - m_nurbs_curve.curve_knots.at(knot_pos - 1) > 2) && (knot_pos < curve_knots_begin))
					{
						m_nurbs_curve.curve_knots.at(knot_pos - 1) = m_nurbs_curve.curve_knots.at(knot_pos) - 2;
						knot_pos--;
					}
				}
				else
					if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Should split up the curve here" << std::endl;
			}
		}
	}
}

void nurbs_curve_modifier::flip_curve(k3d::uint_t curve)
{
	if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Flipping curve " << curve << std::endl;

	const k3d::uint_t curve_point_begin = m_nurbs_curve.curve_first_points.at(curve);
	const k3d::uint_t curve_point_end = curve_point_begin + m_nurbs_curve.curve_point_counts.at(curve);

	const k3d::uint_t curve_knots_begin = m_nurbs_curve.curve_first_knots.at(curve);
	const k3d::uint_t curve_knots_end = curve_knots_begin + (curve_point_end - curve_point_begin) + m_nurbs_curve.curve_orders.at(curve);

	if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Get last knot within " << curve_knots_begin << " and " << curve_knots_end << " with size " << m_nurbs_curve.curve_knots.size() << std::endl;
	double last = m_nurbs_curve.curve_knots.at(curve_knots_end - 1);

	k3d::mesh::knots_t new_knots;

	//flip knot vector
	for (int knot = curve_knots_end - 1; (knot >= curve_knots_begin) && (knot >= 0); knot--)
	{
		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Flip knot vector (knot = " << knot << "), last was " << last  << std::endl;
		new_knots.push_back(last - m_nurbs_curve.curve_knots.at(knot));
	}

	k3d::mesh::knots_t::iterator knot_iter = m_nurbs_curve.curve_knots.begin() + curve_knots_begin;

	if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "erase old knots" << std::endl;
	for (k3d::uint_t i = curve_knots_begin; i < curve_knots_end; i++)
		m_nurbs_curve.curve_knots.erase(knot_iter);

	if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "insert new ones" << std::endl;
	m_nurbs_curve.curve_knots.insert(knot_iter, new_knots.begin(), new_knots.end());

	print_knot_vector(curve);

	//flip point indices
	k3d::mesh::indices_t::iterator curve_points_begin_iter = m_nurbs_curve.curve_points.begin() + curve_point_begin;
	k3d::mesh::indices_t::iterator curve_points_end_iter = m_nurbs_curve.curve_points.begin() + curve_point_end;

	if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "reverse points" << std::endl;
	std::reverse(curve_points_begin_iter, curve_points_end_iter);

	//flip weights
	k3d::mesh::weights_t::iterator point_weights_begin_iter = m_nurbs_curve.curve_point_weights.begin() + curve_point_begin;
	k3d::mesh::weights_t::iterator point_weights_end_iter = m_nurbs_curve.curve_point_weights.begin() + curve_point_end;
	if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "reverse weights" << std::endl;
	std::reverse(point_weights_begin_iter, point_weights_end_iter);
	if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "finished flipping\n" << std::endl;
}

void nurbs_curve_modifier::normalize_knot_vector(k3d::uint_t curve)
{
	if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Normalizing knot vector of curve" << curve << std::endl;
	print_knot_vector(curve);

	const k3d::uint_t curve_point_begin = m_nurbs_curve.curve_first_points.at(curve);
	const k3d::uint_t curve_point_end = curve_point_begin + m_nurbs_curve.curve_point_counts.at(curve);

	const k3d::uint_t curve_knots_begin = m_nurbs_curve.curve_first_knots.at(curve);
	const k3d::uint_t curve_knots_end = curve_knots_begin + (curve_point_end - curve_point_begin) + m_nurbs_curve.curve_orders.at(curve);

	double min = m_nurbs_curve.curve_knots.at(curve_knots_begin);
	double max = m_nurbs_curve.curve_knots.at(curve_knots_end - 1) - min;

	for (k3d::uint_t knot = curve_knots_begin; knot < curve_knots_end; knot++)
	{
		m_nurbs_curve.curve_knots.at(knot) = (m_nurbs_curve.curve_knots.at(knot) - min) / max;
	}
	print_knot_vector(curve);
}

int nurbs_curve_modifier::count_all_curves_in_groups()
{
	return m_nurbs_curve.curve_first_knots.size();
}

void nurbs_curve_modifier::delete_curve(k3d::uint_t curve)
{
	try
	{
		const k3d::uint_t curve_points_begin = m_nurbs_curve.curve_first_points.at(curve);
		const k3d::uint_t curve_points_end = curve_points_begin + m_nurbs_curve.curve_point_counts.at(curve);

		const k3d::uint_t curve_knots_begin = m_nurbs_curve.curve_first_knots.at(curve);
		const k3d::uint_t curve_knots_end = curve_knots_begin + (curve_points_end - curve_points_begin) + m_nurbs_curve.curve_orders.at(curve);

		MY_DEBUG << "Deleting curve " << curve << " with points " << curve_points_begin << " to " << curve_points_end << " and knots " << curve_knots_begin << " to " << curve_knots_end << std::endl;

		int point_offset = curve_points_end - curve_points_begin;
		int knot_offset = curve_knots_end - curve_knots_begin;

		//offset all following points and knots
		for (k3d::uint_t curr_curve = 0; curr_curve < m_nurbs_curve.curve_first_points.size(); curr_curve++)
		{
			if (m_nurbs_curve.curve_first_points.at(curr_curve) > curve_points_begin)
				m_nurbs_curve.curve_first_points.at(curr_curve) -= point_offset;
			if (m_nurbs_curve.curve_first_knots.at(curr_curve) > curve_knots_begin)
				m_nurbs_curve.curve_first_knots.at(curr_curve) -= knot_offset;
		}

		//remove points, weights, knots, point_count and order

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Erasing knots" << std::endl;
		k3d::mesh::knots_t::iterator knots_begin = m_nurbs_curve.curve_knots.begin() + curve_knots_begin;
		k3d::mesh::knots_t::iterator knots_end = m_nurbs_curve.curve_knots.begin() + curve_knots_end;
		m_nurbs_curve.curve_knots.erase(knots_begin, knots_end);

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Erasing points & weights" << std::endl;
		k3d::mesh::indices_t::iterator points_begin = m_nurbs_curve.curve_points.begin() + curve_points_begin;
		k3d::mesh::indices_t::iterator points_end = m_nurbs_curve.curve_points.begin() + curve_points_end;
		k3d::mesh::weights_t::iterator weights_begin = m_nurbs_curve.curve_point_weights.begin() + curve_points_begin;
		k3d::mesh::weights_t::iterator weights_end = m_nurbs_curve.curve_point_weights.begin() + curve_points_end;
		m_nurbs_curve.curve_points.erase(points_begin, points_end);
		m_nurbs_curve.curve_point_weights.erase(weights_begin, weights_end);

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Erasing point counts" << std::endl;
		k3d::mesh::counts_t::iterator point_count_pos = m_nurbs_curve.curve_point_counts.begin() + curve;
		m_nurbs_curve.curve_point_counts.erase(point_count_pos);

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Erasing orders" << std::endl;
		k3d::mesh::orders_t::iterator order_pos = m_nurbs_curve.curve_orders.begin() + curve;
		m_nurbs_curve.curve_orders.erase(order_pos);

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Erasing first_points" << std::endl;
		k3d::mesh::indices_t::iterator first_points_pos = m_nurbs_curve.curve_first_points.begin() + curve;
		m_nurbs_curve.curve_first_points.erase(first_points_pos);

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Erasing first knots" << std::endl;
		k3d::mesh::indices_t::iterator first_knots_pos = m_nurbs_curve.curve_first_knots.begin() + curve;
		m_nurbs_curve.curve_first_knots.erase(first_knots_pos);

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Erasing selection" << std::endl;
		k3d::mesh::selection_t::iterator selection_pos = m_nurbs_curve.curve_selections.begin() + curve;
		m_nurbs_curve.curve_selections.erase(selection_pos);
	}
	catch (...)
	{
		k3d::log() << error << nurbs_debug << "Error in DeleteCurve " << curve << std::endl;
	}
}

void nurbs_curve_modifier::join_curves(k3d::uint_t point1, k3d::uint_t curve1, k3d::uint_t point2, k3d::uint_t curve2)
{
	try
	{
		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Join curves " << curve1 << " and " << curve2 << std::endl;
		//	if point is the first or last point of both curves, flip one of them
		//	.choose them in a way that they can be joined
		//	.normalize knot vectors
		//	.add 1 to all knots in the 2nd curve
		//	.put all points, weights and knots from 2nd into 1st curve
		//	.delete 2nd curve

		if (m_nurbs_curve.curve_orders.at(curve1) < m_nurbs_curve.curve_orders.at(curve2))
		{
			int dif = m_nurbs_curve.curve_orders.at(curve2) - m_nurbs_curve.curve_orders.at(curve1);
			for (int i = 0; i < dif; i++)
				curve_degree_elevate(curve1);
		}
		else if (m_nurbs_curve.curve_orders.at(curve2) < m_nurbs_curve.curve_orders.at(curve1))
		{
			int dif = m_nurbs_curve.curve_orders.at(curve1) - m_nurbs_curve.curve_orders.at(curve2);
			for (int i = 0; i < dif; i++)
				curve_degree_elevate(curve2);
		}


		const k3d::uint_t curve_points_begin[2] = {m_nurbs_curve.curve_first_points.at(curve1), m_nurbs_curve.curve_first_points.at(curve2)};
		const k3d::uint_t curve_points_end[2] = { curve_points_begin[0] + m_nurbs_curve.curve_point_counts.at(curve1), curve_points_begin[1] + m_nurbs_curve.curve_point_counts.at(curve2) };

		const k3d::uint_t curve_knots_begin[2] = { m_nurbs_curve.curve_first_knots.at(curve1), m_nurbs_curve.curve_first_knots.at(curve2)};
		const k3d::uint_t curve_knots_end[2] = { curve_knots_begin[0] + m_nurbs_curve.curve_orders.at(curve1) + m_nurbs_curve.curve_point_counts.at(curve1), curve_knots_begin[1] + m_nurbs_curve.curve_orders.at(curve2) + m_nurbs_curve.curve_point_counts.at(curve2)};

		//we might need to use curve 2 as curve 1 so we dont have to flip
		k3d::uint_t use_curve1 = curve1;
		k3d::uint_t use_curve2 = curve2;

		k3d::uint_t use1 = 0;
		k3d::uint_t use2 = 1;

		if (point1 == curve_points_begin[0] && point2 == curve_points_begin[1])
			flip_curve(curve1);
		else if (point1 == curve_points_end[0] - 1 && point2 == curve_points_end[1] - 1)
			flip_curve(curve2);
		else if (point1 == curve_points_begin[0] && point2 == curve_points_end[1] - 1)
		{
			use_curve1 = curve2;
			use_curve2 = curve1;
			use1 = 1;
			use2 = 0;
		}
		else if (point1 == curve_points_end[0] - 1 && point2 == curve_points_begin[1])
		{
			//no changes
		}
		else
		{
			k3d::log() << error << "You need to select and end-point of each curve!" << std::endl;
			return;
		}

		//if this is true, then we can remove one of the points
		bool isOnePoint = point3_float_equal(mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_end[use2] - 1)), mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_begin[use1])), 0.000001);

		normalize_knot_vector(curve1);
		normalize_knot_vector(curve2);

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Copy Points" << std::endl;
		//copy points
		k3d::mesh::indices_t::iterator points_begin = m_nurbs_curve.curve_points.begin() + curve_points_begin[use2];
		if (isOnePoint) points_begin++;
		k3d::mesh::indices_t::iterator points_end = m_nurbs_curve.curve_points.begin() + curve_points_end[use2];

		k3d::mesh::indices_t::iterator points_insert_at = m_nurbs_curve.curve_points.begin() + curve_points_end[use1];
		m_nurbs_curve.curve_points.insert(points_insert_at, points_begin, points_end);

		k3d::uint_t point_offset = curve_points_end[use2] - curve_points_begin[use2];
		if (isOnePoint) point_offset--;

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "PointOffset " << point_offset << std::endl;

		m_nurbs_curve.curve_point_counts.at(use_curve1) += point_offset;

		for (k3d::uint_t curr_curve = 0; curr_curve < count_all_curves_in_groups(); curr_curve++)
		{
			if (m_nurbs_curve.curve_first_points.at(curr_curve) > m_nurbs_curve.curve_first_points.at(use_curve1))
				m_nurbs_curve.curve_first_points.at(curr_curve) += point_offset;
		}

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Copy Weights" << std::endl;
		//copy weights

		k3d::mesh::weights_t::iterator weights_begin = m_nurbs_curve.curve_point_weights.begin() + curve_points_begin[use2];
		k3d::mesh::weights_t::iterator weights_end = m_nurbs_curve.curve_point_weights.begin() + curve_points_end[use2];

		k3d::mesh::weights_t::iterator weights_insert_at = m_nurbs_curve.curve_point_weights.begin() + curve_points_end[use1];
		if (isOnePoint)
		{
			weights_insert_at--;
			*weights_insert_at = (*weights_insert_at + *weights_begin) / 2;
			weights_insert_at++;
			weights_begin++;
		}

		m_nurbs_curve.curve_point_weights.insert(weights_insert_at, weights_begin, weights_end);

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Copy Knots" << std::endl;
		//copy knots
		k3d::mesh::knots_t::iterator knots1_begin = m_nurbs_curve.curve_knots.begin() + curve_knots_begin[use1];
		k3d::mesh::knots_t::iterator knots1_end = m_nurbs_curve.curve_knots.begin() + curve_knots_end[use1];

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Found where to insert new knots" << std::endl;

		//only delete within the 1st curve
		k3d::uint_t offset1 = std::count(knots1_begin, knots1_end, 1.0); //Get the number that need deleting

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Found " << offset1 << " knots with value 1.0 in curve 1" << std::endl;

		m_nurbs_curve.curve_knots.erase(std::remove(knots1_begin, knots1_end, 1.0), knots1_end);

		//if curve_knots_begin[use_curve1] < curve_knots_begin[use_curve2] do nothing, else subtract offset1
		knots1_end = m_nurbs_curve.curve_knots.begin() + curve_knots_end[use1] - offset1;
		k3d::mesh::knots_t::iterator knots2_begin = m_nurbs_curve.curve_knots.begin();
		k3d::mesh::knots_t::iterator knots2_end = m_nurbs_curve.curve_knots.begin();

		if (curve_knots_begin[use2] > curve_knots_begin[use1])
		{
			knots2_begin +=  curve_knots_begin[use2] - offset1;
			knots2_end +=  curve_knots_end[use2] - offset1;
			if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "With offset " << std::distance(knots2_begin, knots2_end) << std::endl;
		}
		else
		{
			knots2_begin +=  curve_knots_begin[use2];
			knots2_end +=  curve_knots_end[use2];
			if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Without offset " << std::distance(knots2_begin, knots2_end) << std::endl;
		}

		knots1_end--;

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Adding " << 1 << " to each knot of 2nd curve" << std::endl;

		std::stringstream str;
		str << "Knot vector:";

		for (k3d::mesh::knots_t::iterator i = knots2_begin; i < knots2_end; ++i)
		{
			*i += 1;
			str << " " << *i;
		}
		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << str.str() << std::endl;

		double find_first = *knots2_begin;
		k3d::uint_t offset2 = std::count(knots2_begin, knots2_end, find_first);
		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Found " << offset2 << " knots with value " << find_first << " in curve 2" << std::endl;

		knots1_end = m_nurbs_curve.curve_knots.begin() + curve_knots_end[use1] - offset1 - 1;
		knots2_begin += offset2;

		k3d::mesh::knots_t new_knots;
		double a = *knots1_end;
		if (isOnePoint)
		{
			for (int i = 0; i < m_nurbs_curve.curve_orders.at(use_curve1); i++)
			{
				new_knots.push_back(1.0);//the first one is just a dummy but we need the rest
			}
			new_knots.push_back(*knots2_begin);
		}
		else
		{
			double step = (*knots2_begin - a) / (m_nurbs_curve.curve_orders.at(use_curve1) + 1);

			for (int i = 0; i < m_nurbs_curve.curve_orders.at(use_curve1) + 2; i++)
			{
				new_knots.push_back(a + step*i);
				MY_DEBUG << new_knots.back() << std::endl;
			}
		}

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Going to insert at pos " << std::distance(m_nurbs_curve.curve_knots.begin(), knots1_end) << " with size " << m_nurbs_curve.curve_knots.size() << std::endl;
		knots1_end++;
		new_knots.erase(new_knots.begin());
		m_nurbs_curve.curve_knots.insert(knots1_end, new_knots.begin(), new_knots.end());

		knots1_end = m_nurbs_curve.curve_knots.begin() + curve_knots_end[use1] - offset1 + new_knots.size();
		knots2_begin = m_nurbs_curve.curve_knots.begin() + curve_knots_begin[use2] + offset2 + 1;
		knots2_end = m_nurbs_curve.curve_knots.begin() + curve_knots_end[use2];

		if (curve_knots_begin[use2] > curve_knots_begin[use1])
		{
			knots2_begin += new_knots.size() - offset1;
			knots2_end += new_knots.size() - offset1;
		}

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Inserting : " << std::distance(knots2_begin, knots2_end) << " knots from " << std::distance(m_nurbs_curve.curve_knots.begin(), knots2_begin) << " ( is " << *knots2_begin << ") to " << std::distance(m_nurbs_curve.curve_knots.begin(), knots2_end) << " ( is " << *knots2_end << ")" << std::endl;

		new_knots.clear();
		new_knots.insert(new_knots.end(), knots2_begin, knots2_end);

		for (k3d::mesh::knots_t::iterator i = new_knots.begin(); i != new_knots.end(); ++i)
		{
			MY_DEBUG << *i << std::endl;
		}

		m_nurbs_curve.curve_knots.insert(knots1_end, new_knots.begin(), new_knots.end());
		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Inserted Knots" << std::endl;

		int knot_offset = curve_knots_end[use2] - curve_knots_begin[use2] - offset1 - offset2 + m_nurbs_curve.curve_orders.at(use_curve1);
		if (isOnePoint)
			knot_offset--;

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Offset for first_knots is " << knot_offset << std::endl;

		for (k3d::uint_t curr_curve = 0; curr_curve < count_all_curves_in_groups(); curr_curve++)
		{
			if (m_nurbs_curve.curve_first_knots.at(curr_curve) > m_nurbs_curve.curve_first_knots.at(use_curve1))
				m_nurbs_curve.curve_first_knots.at(curr_curve) += knot_offset;
		}

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Deleting 2nd curve.." << std::endl;
		//delete the 2nd curve
		delete_curve(use_curve2);
	}
	catch (...)
	{
		k3d::log() << error << nurbs_debug << "Error in JoinCurves" << std::endl;
	}
}

void nurbs_curve_modifier::close_curve(k3d::uint_t curve, bool keep_ends)
{
	if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "NurbsCloseCurve.." << std::endl;
	/*
	    - get both end points
	    if(keep_ends)
	        . add the middle of the points as new one and use the old ends as CV
	    else . replace them by their arithmetical middle
	*/
	const k3d::uint_t curve_points_begin = m_nurbs_curve.curve_first_points.at(curve);
	const k3d::uint_t curve_points_end = curve_points_begin + m_nurbs_curve.curve_point_counts.at(curve);

	const k3d::uint_t curve_knots_begin = m_nurbs_curve.curve_first_knots.at(curve);
	const k3d::uint_t curve_knots_end = curve_knots_begin + (curve_points_end - curve_points_begin) + m_nurbs_curve.curve_orders.at(curve);

	k3d::point3 new_point = (mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_begin)) + mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_end - 1))) * 0.5;
	mesh_points->push_back(new_point);
	point_selection->push_back(0.0);
	if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "New point inserted" << std::endl;

	k3d::uint_t new_index = mesh_points->size() - 1;

	if (!keep_ends)
	{
		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Simple way:" << std::endl;
		replace_point(new_index, curve, m_nurbs_curve.curve_points.at(curve_points_begin), true);
		replace_point(new_index, curve, m_nurbs_curve.curve_points.at(curve_points_end - 1), true);
	}
	else
	{
		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "We're going the hard way:" << std::endl;
		normalize_knot_vector(curve);
		k3d::uint_t order = m_nurbs_curve.curve_orders[curve];
		//take the first knot different from 0 and the last one different from 1.0, double the
		k3d::uint_t first = curve_knots_begin;

		while (m_nurbs_curve.curve_knots.at(first) == 0.0 && first < curve_knots_end)
		{
			first++;
		}
		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "We need to insert 0.0 at " << first << std::endl;

		k3d::uint_t last = curve_knots_end - 1;

		while (m_nurbs_curve.curve_knots.at(last) == 1.0 && last >= curve_knots_begin)
		{
			last--;
		}
		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "and 1.0 at " << last << std::endl;
		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Applying offsets" << std::endl;
		//insert knot at the end
		double diff = 1.0 - m_nurbs_curve.curve_knots.at(last);
		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Creating iterator" << std::endl;

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "looping through knots and applying diff=" << diff << std::endl;
		for (k3d::uint_t i = last + 1; i < curve_knots_end; i++)
			m_nurbs_curve.curve_knots.at(i) += diff;

		//insert knot at the beginning
		diff = m_nurbs_curve.curve_knots.at(first);
		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "iterator" << std::endl;

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "loop with diff=" << diff << std::endl;
		for (int i = first - 1; i >= static_cast<int>(curve_knots_begin); i--)
		{
			k3d::log() << i << std::endl;
			m_nurbs_curve.curve_knots.at(i) -= diff;
		}

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Inserting knots 1" << std::endl;
		k3d::mesh::knots_t::iterator last_iter = m_nurbs_curve.curve_knots.begin() + last + 1;
		m_nurbs_curve.curve_knots.insert(last_iter, 1.0);
		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Inserting knots 2" << std::endl;
		k3d::mesh::knots_t::iterator first_iter = m_nurbs_curve.curve_knots.begin() + first;
		m_nurbs_curve.curve_knots.insert(first_iter, 0.0);

		if (MODULE_NURBS_DEBUG) if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Inserting points" << std::endl;
		//insert point indices to the new point
		k3d::mesh::indices_t::iterator point_iter = m_nurbs_curve.curve_points.begin() + curve_points_begin;
		m_nurbs_curve.curve_points.insert(point_iter, new_index);

		point_iter = m_nurbs_curve.curve_points.begin() + curve_points_end + 1; //because we've already inserted one point
		m_nurbs_curve.curve_points.insert(point_iter, new_index);

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Inserting weights" << std::endl;
		k3d::mesh::weights_t::iterator weights_iter = m_nurbs_curve.curve_point_weights.begin() + curve_points_begin;
		m_nurbs_curve.curve_point_weights.insert(weights_iter, 1.0);
		weights_iter = m_nurbs_curve.curve_point_weights.begin() + curve_points_end;
		m_nurbs_curve.curve_point_weights.insert(weights_iter, 1.0);

		m_nurbs_curve.curve_point_counts.at(curve) += 2;

		for (k3d::uint_t curr_curve = 0; curr_curve < count_all_curves_in_groups(); curr_curve++)
		{
			if (m_nurbs_curve.curve_first_points.at(curve) < m_nurbs_curve.curve_first_points.at(curr_curve))
				m_nurbs_curve.curve_first_points.at(curr_curve) += 2;
			if (m_nurbs_curve.curve_first_knots.at(curve) < m_nurbs_curve.curve_first_knots.at(curr_curve))
				m_nurbs_curve.curve_first_knots.at(curr_curve) += 2;
		}
		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Normalizing knot vector" << std::endl;
		normalize_knot_vector(curve);
	}
}

bool nurbs_curve_modifier::point3_float_equal(const k3d::point3& p1, const k3d::point3& p2, float threshold)
{
	float point1[3], point2[3];

	point1[0] = static_cast<float>(p1[0]);
	point1[1] = static_cast<float>(p1[1]);
	point1[2] = static_cast<float>(p1[2]);

	point2[0] = static_cast<float>(p2[0]);
	point2[1] = static_cast<float>(p2[1]);
	point2[2] = static_cast<float>(p2[2]);

	if (fabs(point1[0] - point2[0]) < threshold
	    && fabs(point1[1] - point2[1]) < threshold
	    && fabs(point1[2] - point2[2]) < threshold)
		return true;
	return false;
}

int nurbs_curve_modifier::find_span(k3d::uint_t curve, double u)
{
	try
	{

		k3d::uint_t order = m_nurbs_curve.curve_orders.at(curve);
		k3d::uint_t nr_points = m_nurbs_curve.curve_point_counts.at(curve);

		const k3d::uint_t curve_point_begin = m_nurbs_curve.curve_first_points.at(curve);
		const k3d::uint_t curve_point_end = curve_point_begin + m_nurbs_curve.curve_point_counts.at(curve);

		const k3d::uint_t curve_knots_begin = m_nurbs_curve.curve_first_knots.at(curve);
		const k3d::uint_t curve_knots_end = curve_knots_begin + (curve_point_end - curve_point_begin) + m_nurbs_curve.curve_orders.at(curve);

		int n = curve_knots_end - curve_knots_begin - order;

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "FindSpan of " << u << " in curve " << curve << " with n=" << n << std::endl;

		if (u >= m_nurbs_curve.curve_knots.at(curve_knots_begin + n + 1)) return n - 1;

		int low = order - 1;
		int high = n + 1;
		int mid = (low + high) / 2;

		while ((u < m_nurbs_curve.curve_knots.at(curve_knots_begin + mid) || u >= m_nurbs_curve.curve_knots.at(curve_knots_begin + mid + 1)) && mid != low)
		{
			if (u < m_nurbs_curve.curve_knots.at(curve_knots_begin + mid))
			{
				MY_DEBUG << "Lowering upper limit from " << high << " to " << mid << std::endl;
				high = mid;
			}
			else
			{
				MY_DEBUG << "Inreasing lower limit from " << low << " to " << mid << std::endl;
				low = mid;
			}
			mid = (low + high) / 2;
		}
		MY_DEBUG << "Found span " << mid << std::endl;
		return mid;
	}
	catch (...)
	{
		k3d::log() << error << nurbs_debug << "Exception in FindSpan" << std::endl;
		return -1;
	}
}

std::vector<double> nurbs_curve_modifier::basis_functions(k3d::uint_t curve, double u, k3d::uint_t span)
{
	if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Getting basis functions" << std::endl;

	k3d::uint_t order = m_nurbs_curve.curve_orders.at(curve);
	k3d::uint_t nr_points = m_nurbs_curve.curve_point_counts.at(curve);

	const k3d::uint_t curve_points_begin = m_nurbs_curve.curve_first_points.at(curve);
	const k3d::uint_t curve_points_end = curve_points_begin + m_nurbs_curve.curve_point_counts.at(curve);

	const k3d::uint_t curve_knots_begin = m_nurbs_curve.curve_first_knots.at(curve);
	const k3d::uint_t curve_knots_end = curve_knots_begin + (curve_points_end - curve_points_begin) + m_nurbs_curve.curve_orders.at(curve);

	std::vector<double> bases(order, 0.0);
	bases.at(0) = 1.0;

	std::vector<double> left(order, 0.0);
	std::vector<double> right(order, 0.0);

	for (int j = 1; j < order; j++)
	{
		left.at(j) = u - m_nurbs_curve.curve_knots.at(curve_knots_begin + span + 1 - j);
		right.at(j) = m_nurbs_curve.curve_knots.at(curve_knots_begin + span + j) - u;

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "j = " << j << " | left = " << left.at(j) << " | right = " << right.at(j) << std::endl;

		double saved = 0.0;

		for (int r = 0; r < j; r++)
		{
			float temp = bases.at(r) / (right.at(r + 1) + left.at(j - r));
			bases.at(r) = saved + right.at(r + 1) * temp;
			saved = left.at(j - r) * temp;
		}
		bases.at(j) = saved;
	}
	return bases;
}

k3d::point4 nurbs_curve_modifier::curve_point(k3d::uint_t curve, double u)
{
	try
	{
		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Evaluating point on curve " << curve << " with knot " << u << std::endl;

		k3d::uint_t order = m_nurbs_curve.curve_orders.at(curve);
		k3d::uint_t nr_points = m_nurbs_curve.curve_point_counts.at(curve);

		const k3d::uint_t curve_points_begin = m_nurbs_curve.curve_first_points.at(curve);
		const k3d::uint_t curve_points_end = curve_points_begin + m_nurbs_curve.curve_point_counts.at(curve);

		const k3d::uint_t curve_knots_begin = m_nurbs_curve.curve_first_knots.at(curve);
		const k3d::uint_t curve_knots_end = curve_knots_begin + (curve_points_end - curve_points_begin) + m_nurbs_curve.curve_orders.at(curve);

		k3d::uint_t span = find_span(curve, u);
		std::vector<double> bases = basis_functions(curve, u, span);

		k3d::point4 cv;

		for (k3d::uint_t i = 0; i < order; i++)
		{
			MY_DEBUG << "loading point(size = " << m_nurbs_curve.curve_points.size() << "): " << curve_points_begin + span - order + 1 + i << std::endl;
			k3d::point3 p = mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_begin + span - order + 1 + i));
			double w = m_nurbs_curve.curve_point_weights.at(curve_points_begin + span - order + 1 + i);
			if (w == 0.0)
			{
				k3d::log() << error << "A point had weight zero" << std::endl;
			}
			cv = cv + bases.at(i) * k3d::point4(p[0] * w, p[1] * w, p[2] * w, w);
		}
		return cv;
	}
	catch (...)
	{
		k3d::log() << error << nurbs_debug << "Error in CurvePoint" << std::endl;
	}
}

/*
    Implementation of Algorithm "CurveKnotInsertion" from
    "The NURBS book", page 151, by Piegl and Tiller
*/
bool nurbs_curve_modifier::curve_knot_insertion(k3d::uint_t curve, double u, k3d::uint_t r)
{
	try
	{
		//*******************************************************************
		//Set up and find multiplicity of "u"
		//*******************************************************************

		k3d::uint_t order = m_nurbs_curve.curve_orders.at(curve);
		k3d::uint_t nr_points = m_nurbs_curve.curve_point_counts.at(curve);

		const k3d::uint_t curve_points_begin = m_nurbs_curve.curve_first_points.at(curve);
		const k3d::uint_t curve_points_end = curve_points_begin + m_nurbs_curve.curve_point_counts.at(curve);

		const k3d::uint_t curve_knots_begin = m_nurbs_curve.curve_first_knots.at(curve);
		const k3d::uint_t curve_knots_end = curve_knots_begin + (curve_points_end - curve_points_begin) + m_nurbs_curve.curve_orders.at(curve);

		MY_DEBUG << "Curve knot insertion called: curve=" << curve << " u=" << u << " r=" << r << std::endl;
		print_knot_vector(curve);

		int k = -1;
		k3d::uint_t s = 0;

		//look where to insert the new knot, check multiplicity(s) of the knot at u
		int i = curve_knots_begin;

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Starting while loop" << std::endl;


		while ((i < (int)curve_knots_end) && m_nurbs_curve.curve_knots.at(i) - u < 0.000001)
		{
			i++;
		}

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "While loop stopped with i=" << i << std::endl;
		//we found the first knot greater than u or we're at the end so thats our k now
		k = --i - curve_knots_begin;
		if (i < 0)
			i++;
		if (i < curve_knots_end)
		{
			//we go back to see how often we have this knot
			if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Starting while loop" << std::endl;

			while ((i >= (int)curve_knots_begin) && fabs(m_nurbs_curve.curve_knots.at(i) - u) < 0.000001)
			{
				i--;
				s++;
			}
			if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "While loop stopped with i=" << i << ", s=" << s << " and u=" << u << std::endl;
		}

		if (s + r > order - 1)
		{
			k3d::log() << error << "Cannot insert knot r=" << r << " times, multiplicity would be greater than degree " << order - 1 << std::endl;
			int new_r = order - 1 - s;
			if (new_r <= 0)
			{
				//we had a knot that was already inserted too often, so dont insert it again
				new_r = 0;
				k3d::log() << error << "We're not going to insert the knot at all" << std::endl;
				return false;
			}
			r = new_r;
			if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Reducing r to the maximum possible value " << r << std::endl;
		}

		//*******************************************************************
		//Algorithm from "The NURBS book" page 151
		//*******************************************************************

		k3d::uint_t mp = curve_knots_end - curve_knots_begin;
		k3d::uint_t nq = nr_points + r;

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Inserting knot " << u << " with multiplicity " << r << " into curve" << std::endl;

		k3d::mesh::knots_t knots;

		for (k3d::uint_t i = 0; i <= k; i++)
		{
			knots.push_back(m_nurbs_curve.curve_knots.at(i + curve_knots_begin));
		}


		for (k3d::uint_t i = 1; i <= r; i++)
		{
			knots.push_back(u);
		}

		for (k3d::uint_t i = k + 1; i < mp; i++)
		{
			knots.push_back(m_nurbs_curve.curve_knots.at(i + curve_knots_begin));
		}

		k3d::point3 p = mesh_points->at((m_nurbs_curve.curve_points.at(curve_points_begin)));
		std::vector<k3d::point4> points(nq, k3d::point4(p[0], p[1], p[2], m_nurbs_curve.curve_point_weights.at(curve_points_begin)));

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Points array has size " << points.size() << std::endl;
		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Loading control points part 1" << std::endl;

		for (k3d::uint_t i = 0; i <= k + 1 - order; i++)
		{
			p = mesh_points->at((m_nurbs_curve.curve_points.at(curve_points_begin + i)));
			points.at(i) = k3d::point4(p[0], p[1], p[2], m_nurbs_curve.curve_point_weights.at(curve_points_begin + i));
		}

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Control points part 2" << std::endl;

		for (k3d::uint_t i = k - s; i < nr_points; i++)
		{
			p = mesh_points->at((m_nurbs_curve.curve_points.at(curve_points_begin + i)));
			points.at(i + r) = k3d::point4(p[0], p[1], p[2], m_nurbs_curve.curve_point_weights.at(curve_points_begin + i));
		}


		std::vector<k3d::point4> tmp;

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Filling temp array with weighted control points" << std::endl;

		for (k3d::uint_t i = 0; i <= order - 1 - s; i++)
		{
			p = mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_begin + i + k - (order - 1)));
			const k3d::double_t w = m_nurbs_curve.curve_point_weights.at(curve_points_begin + i + k - (order - 1));
			tmp.push_back(k3d::point4(w*p[0], w*p[1], w*p[2], w));
		}
		k3d::uint_t L = 0;

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Starting calculation loop" << std::endl;

		for (k3d::uint_t j = 1; j <= r; j++)
		{
			L = k - (order - 1) + j;
			double alpha = 0.0;
			for (k3d::uint_t i = 0; i <= order - 1 - j - s; i++)
			{
				alpha = (u - m_nurbs_curve.curve_knots.at(curve_knots_begin + L + i)) / (m_nurbs_curve.curve_knots.at(curve_knots_begin + i + k + 1) - m_nurbs_curve.curve_knots.at(curve_knots_begin + L + i));
				tmp[i] = alpha * tmp.at(i + 1) + (1.0 - alpha) * tmp.at(i);
			}
			const k3d::point4& t1 = tmp[0];
			double w = t1[3];
			points[L] = k3d::point4(t1[0] / w, t1[1] / w, t1[2] / w, w);
			const k3d::point4& t2 = tmp[order - 1 - j - s];
			w = t2[3];
			points[k + r - j - s] = k3d::point4(t2[0] / w, t2[1] / w, t2[2] / w, w);
		}

		for (k3d::uint_t i = L + 1; i < k - s; i++)
		{
			const k3d::point4& t = tmp[i - L];
			double w = t[3];
			points[i] = k3d::point4(t[0] / w, t[1] / w, t[2] / w, w);
		}

		//*******************************************************************
		//Insert new points and knot vector into the mesh
		//*******************************************************************

		//insert knots
		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Inserting knots" << std::endl;
		const k3d::uint_t knot_offset = knots.size() - mp;
		k3d::mesh::knots_t::iterator knot = m_nurbs_curve.curve_knots.begin();
		knot += m_nurbs_curve.curve_first_knots.at(curve);
		//remove old knot vector
		for (k3d::uint_t i = 0; i < mp; i++)
			m_nurbs_curve.curve_knots.erase(knot);
		//insert new one
		m_nurbs_curve.curve_knots.insert(knot, knots.begin(), knots.end());

		for (k3d::uint_t curr_curve = 0; curr_curve < count_all_curves_in_groups(); curr_curve++)
		{
			if (m_nurbs_curve.curve_first_knots.at(curr_curve) > m_nurbs_curve.curve_first_knots.at(curve))
				m_nurbs_curve.curve_first_knots.at(curr_curve) += knot_offset;
		}

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Inserting points" << std::endl;
		//new points
		k3d::mesh::indices_t new_curve_points;

		k3d::mesh::weights_t new_curve_point_weights;

		for (k3d::uint_t i = 0; i < points.size(); i++)
		{
			p = k3d::point3(points[i][0], points[i][1], points[i][2]);

			k3d::mesh::points_t::iterator index = std::find(mesh_points->begin(), mesh_points->end(), p);
			if (index != mesh_points->end())
			{
				new_curve_points.push_back(distance(mesh_points->begin(), index));
				if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Point already there, adding index " << new_curve_points[i] << " to curve_points" << std::endl;
				if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Point: " << points.at(i).n[0] << " x " << points.at(i).n[1] << " x " << points.at(i).n[2] << " x " << points.at(i).n[3] << std::endl;
			}
			else
			{
				if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Need to add point:" << std::endl;
				if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Point: " << points.at(i).n[0] << " x " << points.at(i).n[1] << " x " << points.at(i).n[2] << " x " << points.at(i).n[3] << std::endl;
				mesh_points->push_back(p);
				point_selection->push_back(0.0);
				new_curve_points.push_back(mesh_points->size() - 1);
			}
			//insert curve_point_weight
			new_curve_point_weights.push_back(points[i][3]);
		}

		k3d::mesh::weights_t::iterator point_weight = m_nurbs_curve.curve_point_weights.begin() + curve_points_begin;
		for (k3d::uint_t i = 0; i < curve_points_end - curve_points_begin; i++)
		{
			m_nurbs_curve.curve_point_weights.erase(point_weight);
		}

		m_nurbs_curve.curve_point_weights.insert(point_weight, new_curve_point_weights.begin(), new_curve_point_weights.end());


		k3d::mesh::indices_t::iterator point = m_nurbs_curve.curve_points.begin();
		point += m_nurbs_curve.curve_first_points.at(curve);
		for (k3d::uint_t i = 0; i < nr_points; i++)
			m_nurbs_curve.curve_points.erase(point);
		m_nurbs_curve.curve_points.insert(point, new_curve_points.begin(), new_curve_points.end());

		const k3d::uint_t point_offset = new_curve_points.size() - (curve_points_end - curve_points_begin);

		for (k3d::uint_t curr_curve = 0; curr_curve < count_all_curves_in_groups(); curr_curve++)
		{
			if (m_nurbs_curve.curve_first_points.at(curr_curve) > m_nurbs_curve.curve_first_points.at(curve))
				m_nurbs_curve.curve_first_points.at(curr_curve) += point_offset;
		}

		m_nurbs_curve.curve_point_counts.at(curve) += point_offset;

		if (MODULE_NURBS_DEBUG) k3d::log() << debug << nurbs_debug << "Inserted " << knot_offset << " knots and " << point_offset << " points!" << std::endl;
		return true;
	}
	catch (...)
	{
		k3d::log() << error << "Tried to access nonexisting value in a std::vector" << std::endl;
		return false;
	}
}

int nurbs_curve_modifier::factorial(int n)
{
	if (n == 0) return 1;
	if (n < 3) return n;
	return factorial(n -1)*n;
}

double nurbs_curve_modifier::binomial_coefficient(int n, int k)
{
	if (n - k >= 0)
	{
		double result = factorial(n);
		result /= factorial(k) * factorial(n - k);
		MY_DEBUG << n << " over " << k << " = " << result << std::endl;
		return result;
	}
	else
	{
		MY_DEBUG << "n - k has to be greater than zero!" << std::endl;
		return 0.0;
	}
}

int nurbs_curve_modifier::Min(int a, int b)
{
	if (a < b)
		return a;
	else
		return b;
}

int nurbs_curve_modifier::Max(int a, int b)
{
	if (a > b)
		return a;
	else
		return b;
}

void nurbs_curve_modifier::fill_bezalfs(std::vector<std::vector<double> >& bezalfs, int power, int t)
{
	try
	{
		int ph = power + t;
		int ph2 = ph / 2;

		bezalfs.at(0).at(0) = 1.0;
		bezalfs.at(ph).at(power) = 1.0;

		for (int i = 1; i <= ph2; i++)
		{
			double inv = 1.0 / binomial_coefficient(ph, i);
			int mpi = Min(power, i);

			for (int j = Max(0, i - t); j <= mpi; j++)
			{
				bezalfs.at(i).at(j) = inv * binomial_coefficient(power, j) * binomial_coefficient(t, i - j);
				MY_DEBUG << "bezalfs[" << i << "][" << j << "] = " << bezalfs.at(i).at(j) << std::endl;
			}
		}

		for (int i = ph2 + 1; i <= ph - 1; i++)
		{
			int mpi = Min(power, i);
			for (int j = Max(0, i - t); j <= mpi; j++)
			{
				bezalfs.at(i).at(j) = bezalfs.at(ph - i).at(power - j);
				MY_DEBUG << "bezalfs[" << i << "][" << j << "] = " << bezalfs.at(i).at(j) << std::endl;
			}
		}
	}
	catch (...)
	{
		k3d::log() << error << nurbs_debug << " Error in fill bezalfs" << std::endl;
	}
}

k3d::point4 nurbs_curve_modifier::get_homogenous_point(k3d::uint_t point)
{
	try
	{
		k3d::point3 p = mesh_points->at(m_nurbs_curve.curve_points.at(point));
		double w = m_nurbs_curve.curve_point_weights.at(point);
		return k3d::point4(p[0]*w, p[1]*w, p[2]*w, w);
	}
	catch (...)
	{
		k3d::log() << error << nurbs_debug << "Error while getting homogenous point " << point << std::endl;
		return k3d::point4();
	}
}

std::string nurbs_curve_modifier::output_point(const k3d::point4& point)
{
	std::stringstream str;
	str << point[0] << " x " << point[1] << " x " << point[2] << " x " << point[3];
	return str.str();
}

std::string nurbs_curve_modifier::output_point(const k3d::point3& point)
{
	std::stringstream str;
	str << point[0] << " x " << point[1] << " x " << point[2];
	return str.str();
}

k3d::mesh::indices_t nurbs_curve_modifier::create_curve_points(std::vector<k3d::point4>& points)
{
	k3d::mesh::indices_t new_curve_points;

	for (int i = 0; i < points.size(); i++)
	{
		double w = points.at(i)[3];
		k3d::point3 p(points.at(i)[0] / w, points.at(i)[1] / w, points.at(i)[2] / w);
		int found = -1;

		for (int j = 0; j < mesh_points->size(); j++)
		{
			if (point3_float_equal(mesh_points->at(j), p, 0.000001))
			{
				found = j;
			}
		}

		if (found < 0)
		{
			//we need to insert the point
			mesh_points->push_back(p);
			point_selection->push_back(0.0);
			new_curve_points.push_back(mesh_points->size() - 1);
		}
		else new_curve_points.push_back(found);
	}

	return new_curve_points;
}

k3d::mesh::weights_t nurbs_curve_modifier::create_curve_point_weights(std::vector<k3d::point4>& points)
{
	k3d::mesh::weights_t new_point_weights;

	for (int i = 0; i < points.size(); i++)
	{
		new_point_weights.push_back(points.at(i)[3]);
	}

	return new_point_weights;
}
/*
    Implementation of Algorithm A5.9 "DegreeElevateCurve" from
    "The NURBS book", page 206, by Piegl and Tiller
*/
void nurbs_curve_modifier::curve_degree_elevate(k3d::uint_t curve)
{
	try
	{
		MY_DEBUG << "DegreeElevation started on curve " << curve << std::endl;

		const k3d::uint_t t = 1;
		const k3d::uint_t order = m_nurbs_curve.curve_orders.at(curve);
		k3d::uint_t power = order - 1;
		const k3d::uint_t nr_points = m_nurbs_curve.curve_point_counts.at(curve);

		const k3d::uint_t curve_points_begin = m_nurbs_curve.curve_first_points.at(curve);
		const k3d::uint_t curve_points_end = curve_points_begin + m_nurbs_curve.curve_point_counts.at(curve);

		const k3d::uint_t curve_knots_begin = m_nurbs_curve.curve_first_knots.at(curve);
		const k3d::uint_t curve_knots_end = curve_knots_begin + (curve_points_end - curve_points_begin) + m_nurbs_curve.curve_orders.at(curve);

		std::vector<std::vector<double> > bezalfs(power + t + 1, std::vector<double>(power + 1, 1.0));
		std::vector<k3d::point4> bpts(power + 1, k3d::point4(0.0, 0.0, 0.0, 0.0));
		std::vector<k3d::point4> ebpts(power + t + 1, k3d::point4(0.0, 0.0, 0.0, 0.0));
		std::vector<k3d::point4> next_bpts(power - 1, k3d::point4(0.0, 0.0, 0.0, 0.0));
		std::vector<double> alphas(power - 1, 0.0);

		k3d::mesh::knots_t new_knots(2*(nr_points + order + t), 0.0);
		std::vector<k3d::point4> new_points(2*(nr_points + t), k3d::point4(0.0, 0.0, 0.0, 0.0));

		MY_DEBUG << "Arrays created, filling bezalfs" << std::endl;

		fill_bezalfs(bezalfs, power, t);

		int m = nr_points + power;
		int mh = power + t;
		int kind = mh + 1;
		int r = -1;
		int a = power;
		int b = order;
		int cind = 1;
		double ua = m_nurbs_curve.curve_knots.at(curve_knots_begin);

		new_points.at(0) = get_homogenous_point(curve_points_begin);
		MY_DEBUG << "new_points.at(0) = " << output_point(new_points.at(0)) << std::endl;

		MY_DEBUG << "Pushing back initial value for knot vector: " << ua << std::endl;
		for (int i = 0; i <= power + t; i++)
			new_knots.push_back(ua);

		MY_DEBUG << "Initialize first bezier segment" << std::endl;
		//initialize first bezier segment
		for (int i = 0; i <= power; i++)
		{
			bpts.at(i) = get_homogenous_point(curve_points_begin + i);
			MY_DEBUG << "Loading bpts[" << i << "] = " << output_point(bpts.at(i)) << std::endl;
		}

		while (b < m)//big loop through knot vector
		{
			MY_DEBUG << "Looping through knot vector.. b = " << b << std::endl;
			int i = b;
			MY_DEBUG << "Looking for knot " << m_nurbs_curve.curve_knots.at(curve_knots_begin + b) << std::endl;
			while (b < m && fabs(m_nurbs_curve.curve_knots.at(curve_knots_begin + b) - m_nurbs_curve.curve_knots.at(curve_knots_begin + b + 1)) < 0.00001)
				b++;
			int mul = b - i + 1;
			MY_DEBUG << "Found knot " << mul << " times in" << std::endl;
			print_knot_vector(curve);
			mh = mh + mul + t;
			double ub = m_nurbs_curve.curve_knots.at(curve_knots_begin + b);
			int oldr = r;
			r = power - mul;
			//insert knot ub r times
			int lbz, rbz;
			if (oldr > 0)
				lbz = (oldr + 2) / 2;
			else
				lbz = 1;

			if (r > 0)
				rbz = power + t - ((r + 1) / 2);
			else
				rbz = power + t;

			if (r > 0)
			{
				//insert knot to get bezier segment
				double numer = ub - ua;
				MY_DEBUG << "Inserting knot " << ub << " " << r << " times" << std::endl;
				for (int k = power; k > mul; k--)
				{
					alphas.at(k - mul - 1) = numer / (m_nurbs_curve.curve_knots.at(curve_knots_begin + a + k) - ua);
					MY_DEBUG << "alpha[" << k - mul - 1 << "] = " << alphas.at(k - mul - 1) << std::endl;
				}
				MY_DEBUG << "Calculated Alphas" << std::endl;
				for (int j = 1; j <= r; j++)
				{
					int save = r - j;
					int s = mul + j;
					for (int k = power; k >= s; k--)
					{
						bpts.at(k) = alphas.at(k - s) * bpts.at(k) + (1.0 - alphas.at(k - s)) * bpts.at(k - 1);
						MY_DEBUG << "Calculated bpts[" << k << "] = " << output_point(bpts.at(k)) << std::endl;
					}
					next_bpts.at(save) = bpts.at(power);
				}
			}//end of "insert knot"
			else
				MY_DEBUG << "Didnt have to insert knot " << ub << " because r = " << r << std::endl;
			//degree elevate bezier
			for (int i = lbz; i <= power + t; i++)
			{
				//only points lbz, ... , power+t are used here
				ebpts.at(i) = k3d::point4(0.0, 0.0, 0.0, 0.0);
				int mpi = Min(power, i);
				for (int j = Max(0, i - t); j <= mpi; j++)
				{
					ebpts.at(i) += k3d::to_vector(bezalfs.at(i).at(j) * bpts.at(j));
					MY_DEBUG << "ebpts[" << i << "] = " << output_point(ebpts.at(i)) << " = " << bezalfs.at(i).at(j) << " * " << output_point(bpts.at(j)) << std::endl;
				}
			}
			MY_DEBUG << "ebpts calculation done" << std::endl;

			if (oldr > 1)
			{
				//must remove knot ua oldr times
				MY_DEBUG << "we have to remove our knot " << oldr << "times" << std::endl;
				int first = kind - 2;
				int last = kind;
				double den = ub - ua;
				double bet = (ub - new_knots.at(kind - 1)) / den;
				//knot removal loop
				for (int tr = 1; tr < oldr; tr++)
				{
					int i = first;
					int j = last;
					int kj = j - kind + 1;
					while (j - i > tr)//loop and compute the new control points for one removal step
					{
						if (i < cind)
						{
							double alf = (ub - new_knots.at(i)) / (ua - new_knots.at(i));
							new_points.at(i) = alf * new_points.at(i) + (1.0 - alf) * new_points.at(i - 1);
							MY_DEBUG << "Calculated new_point[" << i << "] = " << output_point(new_points.at(i)) << " with alf = " << alf << std::endl;
						}
						if (j >= lbz)
						{
							if (j - tr <= kind - power - t + oldr)
							{
								double gam = (ub - new_knots.at(j - tr)) / den;
								ebpts.at(kj) = gam * ebpts.at(kj) + (1.0 - gam) * ebpts.at(kj + 1);
							}
							else
								ebpts.at(kj) = bet * ebpts.at(kj) + (1.0 - bet) * ebpts.at(kj + 1);
						}
						i++;
						j--;
						kj--;
					}
					first--;
					last++;
				}
			}//end of removing knot ua
			MY_DEBUG << "Either we finished removing the knot or we didnt have to" << std::endl;
			if (a != power)
			{
				MY_DEBUG << a << " != " << power << std::endl;
				for (int i = 0; i < power + t - oldr; i++)
				{
					new_knots.at(kind) = ua;
					MY_DEBUG << "new_knots[" << kind << "] = " << ua << std::endl;
					kind++;
				}
			}
			MY_DEBUG << "We're going to add the points: " << lbz << " to " << rbz << " starting at " << cind << std::endl;
			MY_DEBUG << "Points array has size " << new_points.size() << std::endl;
			for (int j = lbz; j <= rbz; j++)
			{
				new_points.at(cind) = ebpts.at(j);
				MY_DEBUG << "new_points[" << cind << "] = " << output_point(ebpts.at(j)) << std::endl;
				cind++;

			}
			MY_DEBUG << "Done adding points" << std::endl;

			if (b < m)
			{
				MY_DEBUG << b << " < " << m << std::endl;
				for (int j = 0; j < r; j++)
					bpts.at(j) = next_bpts.at(j);
				for (int j = r; j <= power; j++)
				{
					bpts.at(j) = get_homogenous_point(curve_points_begin + b - power + j);
					MY_DEBUG << "loading further mesh points: " << output_point(bpts.at(j)) << std::endl;
				}
				a = b;
				b++;
				ua = ub;
			}
			else//end knot
			{
				MY_DEBUG << b << " >= " << m << std::endl;
				for (int i = 0; i <= power + t; i++)
				{
					new_knots.at(kind + i) = ub;
				}
			}
		}//end while loop (b < m)

		int new_n = mh - power - t;
		MY_DEBUG << "new number of points = " << new_n << std::endl;
		MY_DEBUG << "new_points.size() = " << new_points.size() << std::endl;
		for (int i = 0; i < new_points.size(); i++)
			MY_DEBUG << output_point(new_points.at(i)) << std::endl;
		MY_DEBUG << "new_knots.size() = " << new_knots.size() << std::endl;
		for (int i = 0; i < new_knots.size(); i++)
			MY_DEBUG << new_knots.at(i) << std::endl;

		MY_DEBUG << "Algorithm finished, going to insert knots and points into the curve" << std::endl;


		new_points.at(new_n - 1) = get_homogenous_point(curve_points_end - 1);
		new_points.resize(new_n);
		new_knots.resize(new_n + order + t);
		k3d::mesh::weights_t new_weights = create_curve_point_weights(new_points);
		k3d::mesh::indices_t new_curve_points = create_curve_points(new_points);

		k3d::mesh::indices_t::iterator curve_points_begin_iter = m_nurbs_curve.curve_points.begin() + curve_points_begin;
		k3d::mesh::indices_t::iterator curve_points_end_iter = m_nurbs_curve.curve_points.begin() + curve_points_end;

		m_nurbs_curve.curve_points.erase(curve_points_begin_iter, curve_points_end_iter);
		m_nurbs_curve.curve_points.insert(curve_points_begin_iter, new_curve_points.begin(), new_curve_points.end());

		k3d::mesh::knots_t::iterator curve_knots_begin_iter = m_nurbs_curve.curve_knots.begin() + curve_knots_begin;
		k3d::mesh::knots_t::iterator curve_knots_end_iter = m_nurbs_curve.curve_knots.begin() + curve_knots_end;

		m_nurbs_curve.curve_knots.erase(curve_knots_begin_iter, curve_knots_end_iter);
		m_nurbs_curve.curve_knots.insert(curve_knots_begin_iter, new_knots.begin(), new_knots.end());

		k3d::mesh::weights_t::iterator curve_point_weights_begin_iter = m_nurbs_curve.curve_point_weights.begin() + curve_points_begin;
		k3d::mesh::weights_t::iterator curve_point_weights_end_iter = m_nurbs_curve.curve_point_weights.begin() + curve_points_end;

		m_nurbs_curve.curve_point_weights.erase(curve_point_weights_begin_iter, curve_point_weights_end_iter);
		m_nurbs_curve.curve_point_weights.insert(curve_point_weights_begin_iter, new_weights.begin(), new_weights.end());

		int point_offset = new_points.size() - (curve_points_end - curve_points_begin);
		int knot_offset = new_knots.size() - (curve_knots_end - curve_knots_begin);

		for (k3d::uint_t curr_curve = 0; curr_curve < count_all_curves_in_groups(); curr_curve++)
		{
			if (m_nurbs_curve.curve_first_points.at(curr_curve) > m_nurbs_curve.curve_first_points.at(curve))
				m_nurbs_curve.curve_first_points.at(curr_curve) += point_offset;
			if (m_nurbs_curve.curve_first_knots.at(curr_curve) > m_nurbs_curve.curve_first_knots.at(curve))
				m_nurbs_curve.curve_first_knots.at(curr_curve) += knot_offset;
		}

		m_nurbs_curve.curve_point_counts.at(curve) += point_offset;
		m_nurbs_curve.curve_orders.at(curve)++;

		MY_DEBUG << "DegreeElevation ended, added curve!" << std::endl;
	}
	catch (...)
	{
		k3d::log() << error << nurbs_debug << "Tried to access nonexisting values" << std::endl;
	}
}

bool nurbs_curve_modifier::split_curve_at(k3d::uint_t curve, double u, bool reconnect)
{
	try
	{

		normalize_knot_vector(curve);
		//prepare curve for splitting
		curve_knot_insertion(curve, u, m_nurbs_curve.curve_orders.at(curve) - 1);

		//double the point at knot "u"
		k3d::point4 p = curve_point(curve, u);
		double w = p[3];
		k3d::point3 p3(p[0] / w, p[1] / w, p[2] / w);
		MY_DEBUG << "Point: " << p[0] << " x " << p[1] << " x " << p[2] << " x " << p[3] << std::endl;

		//search the point in the actual curve
		k3d::uint_t curve_points_begin = m_nurbs_curve.curve_first_points.at(curve);
		k3d::uint_t curve_points_end = curve_points_begin + m_nurbs_curve.curve_point_counts.at(curve);
		int curve_point_index = -1;

		bool isClosed = is_closed(curve);
		k3d::uint_t point1 = curve_points_begin;
		k3d::uint_t point2 = curve_points_end - 1;

		for (k3d::uint_t point = curve_points_begin; point < curve_points_end; point++)
		{
			if (point3_float_equal(mesh_points->at(m_nurbs_curve.curve_points.at(point)), p3, 0.000001))
			{
				if (curve_point_index < 0)
					curve_point_index = point;
				else
				{
					k3d::log() << error << "Curve contains this point several times, don't know where to split" << std::endl;
					return false;
				}
			}
		}

		if (curve_point_index < 0)
		{
			k3d::log() << error << nurbs_debug << "Curve does not contain this point" << std::endl << "Points are: " << std::endl;
			for (k3d::uint_t point = curve_points_begin; point < curve_points_end; point++)
			{
				k3d::log() << error << nurbs_debug << output_point(mesh_points->at(m_nurbs_curve.curve_points.at(point))) << std::endl;
			}
			return false;
		}
		MY_DEBUG  << "Found in curve at index: " << curve_point_index << std::endl;

		//double points and weights
		k3d::mesh::indices_t::iterator point_iter = m_nurbs_curve.curve_points.begin();
		point_iter += curve_point_index + 1;
		mesh_points->push_back(p3);
		point_selection->push_back(0.0);
		m_nurbs_curve.curve_points.insert(point_iter, mesh_points->size() - 1);

		k3d::mesh::weights_t::iterator weight_iter = m_nurbs_curve.curve_point_weights.begin();
		weight_iter += curve_point_index;
		m_nurbs_curve.curve_point_weights.insert(weight_iter, w);

		//insert knots
		k3d::uint_t curve_knots_begin = m_nurbs_curve.curve_first_knots.at(curve);
		k3d::uint_t curve_knots_end = curve_knots_begin + m_nurbs_curve.curve_point_counts.at(curve) + m_nurbs_curve.curve_orders.at(curve);
		int knot_index = -1;

		for (k3d::uint_t knot = curve_knots_begin; knot < curve_knots_end; knot++)
		{
			if (fabs(m_nurbs_curve.curve_knots.at(knot) - u) < 0.000001 && knot_index < 0)
				knot_index = knot;
		}

		if (knot_index < 0)
		{
			k3d::log() << error << "Curve does not contain knot " << u << std::endl;
			return false;
		}
		MY_DEBUG  << "First occurrence of knot " << u << " is " << knot_index << std::endl;

		k3d::mesh::knots_t::iterator knot = m_nurbs_curve.curve_knots.begin() + knot_index;
		m_nurbs_curve.curve_knots.insert(knot, m_nurbs_curve.curve_orders.at(curve) + 1, u);

		//offset all first_knots and first_points
		for (k3d::uint_t curr_curve = 0; curr_curve < count_all_curves_in_groups(); curr_curve++)
		{
			if (m_nurbs_curve.curve_first_points.at(curr_curve) > m_nurbs_curve.curve_first_points.at(curve))
				m_nurbs_curve.curve_first_points.at(curr_curve)++;

			if (m_nurbs_curve.curve_first_knots.at(curr_curve) > m_nurbs_curve.curve_first_knots.at(curve))
				m_nurbs_curve.curve_first_knots.at(curr_curve) += m_nurbs_curve.curve_orders.at(curve) + 1;
		}

		MY_DEBUG  << "Inserting new curve" << std::endl;
		//insert new curve
		m_nurbs_curve.curve_point_counts.at(curve) -= curve_points_end - curve_point_index - 1;

		k3d::mesh::indices_t::iterator cfp = m_nurbs_curve.curve_first_points.begin() + curve + 1;
		m_nurbs_curve.curve_first_points.insert(cfp, curve_point_index + 1);

		k3d::mesh::indices_t::iterator cfk = m_nurbs_curve.curve_first_knots.begin() + curve + 1;
		m_nurbs_curve.curve_first_knots.insert(cfk, knot_index + m_nurbs_curve.curve_orders.at(curve));

		k3d::mesh::orders_t::iterator co = m_nurbs_curve.curve_orders.begin() + curve + 1;
		m_nurbs_curve.curve_orders.insert(co, m_nurbs_curve.curve_orders.at(curve));

		k3d::mesh::counts_t::iterator cpc = m_nurbs_curve.curve_point_counts.begin() + curve + 1;
		m_nurbs_curve.curve_point_counts.insert(cpc, curve_points_end - curve_point_index);

		k3d::mesh::selection_t::iterator cs = m_nurbs_curve.curve_selections.begin() + curve + 1;
		m_nurbs_curve.curve_selections.insert(cs, 0.0);

		MY_DEBUG  << "Increasing curve_counts" << std::endl;

		if (isClosed && reconnect)
		{
			point2 = m_nurbs_curve.curve_first_points.at(curve + 1) + m_nurbs_curve.curve_point_counts.at(curve + 1) - 1;
			join_curves(point1, curve, point2, curve + 1);
		}
	}
	catch (std::exception& e)
	{
		MY_DEBUG << "Error in SplitCurve: " << e.what() << std::endl;
		return false;
	}
	catch (...)
	{
		MY_DEBUG << "Error in SplitCurve" << std::endl;
		return false;
	}
	return true;
}

void nurbs_curve_modifier::traverse_curve(k3d::uint_t curve1, k3d::uint_t curve2, bool create_caps)
{
	//move the 1st curve along the 2nd

	const k3d::uint_t curve_points_begin[2] = {m_nurbs_curve.curve_first_points.at(curve1), m_nurbs_curve.curve_first_points.at(curve2)};
	const k3d::uint_t curve_points_end[2] = { curve_points_begin[0] + m_nurbs_curve.curve_point_counts.at(curve1), curve_points_begin[1] + m_nurbs_curve.curve_point_counts.at(curve2) };

	const k3d::uint_t curve_knots_begin[2] = { m_nurbs_curve.curve_first_knots.at(curve1), m_nurbs_curve.curve_first_knots.at(curve2)};
	const k3d::uint_t curve_knots_end[2] = { curve_knots_begin[0] + m_nurbs_curve.curve_orders.at(curve1) + m_nurbs_curve.curve_point_counts.at(curve1), curve_knots_begin[1] + m_nurbs_curve.curve_orders.at(curve2) + m_nurbs_curve.curve_point_counts.at(curve2)};

	k3d::mesh::points_t new_points;
	k3d::mesh::weights_t new_weights;

	k3d::mesh::knots_t u_knots;
	u_knots.insert(u_knots.end(), m_nurbs_curve.curve_knots.begin() + curve_knots_begin[0], m_nurbs_curve.curve_knots.begin() + curve_knots_end[0]);

	k3d::mesh::knots_t v_knots;
	v_knots.insert(v_knots.end(), m_nurbs_curve.curve_knots.begin() + curve_knots_begin[1], m_nurbs_curve.curve_knots.begin() + curve_knots_end[1]);

	k3d::uint_t u_order = m_nurbs_curve.curve_orders.at(curve1);
	k3d::uint_t v_order = m_nurbs_curve.curve_orders.at(curve2);

	const k3d::uint_t point_count = m_nurbs_curve.curve_point_counts.at(curve1);

	for (int i = 0; i < m_nurbs_curve.curve_point_counts.at(curve2); i++)
	{
		k3d::point3 delta_u = mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_begin[1] + i)) + (-mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_begin[1])));
		double w_u = m_nurbs_curve.curve_point_weights.at(curve_points_begin[1] + i);

		for (int j = 0; j < point_count; j++)
		{
			k3d::point3 p_v = mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_begin[0] + j));
			double w_v = m_nurbs_curve.curve_point_weights.at(curve_points_begin[0] + j);

			new_points.push_back(p_v + delta_u);
			new_weights.push_back(w_u * w_v);
		}
	}
	
	boost::scoped_ptr<k3d::nurbs_patch::primitive> nurbs_patches(get_first_nurbs_patch(m_instance));
	if(!nurbs_patches)
		nurbs_patches.reset(k3d::nurbs_patch::create(m_instance));

	nurbs_patch_modifier mod(m_instance, *nurbs_patches);
	nurbs_patch p;
	p.control_points = new_points;
	p.point_weights = new_weights;
	p.u_knots = u_knots;
	p.v_knots = v_knots;
	p.u_order = u_order;
	p.v_order = v_order;

	mod.insert_patch(p, true);
	if (create_caps)
	{
		if (m_nurbs_curve.curve_points.at(curve_points_begin[0]) == m_nurbs_curve.curve_points.at(curve_points_end[0] - 1) || point3_float_equal(mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_begin[0])), mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_end[0] - 1)), 0.000001))
		{
			create_cap(curve1);

			nurbs_curve temp = extract_curve(curve1);

			k3d::point3 delta_u = mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_end[1] - 1)) + (-mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_begin[1])));
			double w_u = m_nurbs_curve.curve_point_weights.at(curve_points_end[1] - 1);

			for (int i = 0; i < temp.control_points.size(); i++)
			{
				temp.control_points.at(i) = delta_u + temp.control_points.at(i);
				temp.curve_point_weights.at(i) *= w_u;
			}

			int c = add_curve(temp, true);
			create_cap(c);
			delete_curve(c);
		}

		if (m_nurbs_curve.curve_points.at(curve_points_begin[1]) == m_nurbs_curve.curve_points.at(curve_points_end[1] - 1) || point3_float_equal(mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_begin[1])), mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_end[1] - 1)), 0.000001))
		{
			create_cap(curve2);

			nurbs_curve temp = extract_curve(curve2);

			k3d::point3 delta_v = mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_end[0] - 1)) + (-mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_begin[0])));
			double w_v = m_nurbs_curve.curve_point_weights.at(curve_points_end[0] - 1);

			for (int i = 0; i < temp.control_points.size(); i++)
			{
				temp.control_points.at(i) = delta_v + temp.control_points.at(i);
				temp.curve_point_weights.at(i) *= w_v;
			}

			int c = add_curve(temp, true);
			create_cap(c);
			delete_curve(c);
		}
	}
}

void nurbs_curve_modifier::revolve_curve(k3d::uint_t curve, k3d::axis axis, double angle, int segments, bool caps)
{
	//revolve this curve to a certain angle

	const k3d::uint_t curve_points_begin = m_nurbs_curve.curve_first_points.at(curve);
	const k3d::uint_t curve_points_end = curve_points_begin + m_nurbs_curve.curve_point_counts.at(curve);

	const k3d::uint_t curve_knots_begin = m_nurbs_curve.curve_first_knots.at(curve);
	const k3d::uint_t curve_knots_end = curve_knots_begin + (curve_points_end - curve_points_begin) + m_nurbs_curve.curve_orders.at(curve);

	//create a circle with the given angle
	k3d::mesh::knots_t u_knots;
	k3d::mesh::weights_t weights;
	k3d::mesh::points_t control_points;
	switch (axis)
	{
	case k3d::Z :
		k3d::nurbs_curve::circular_arc(k3d::vector3(1, 0, 0), k3d::vector3(0, 1, 0), 0, angle, segments, u_knots, weights, control_points);
		break;
	case k3d::X :
		k3d::nurbs_curve::circular_arc(k3d::vector3(0, 0, 1), k3d::vector3(0, 1, 0), 0, angle, segments, u_knots, weights, control_points);
		break;
	case k3d::Y :
		k3d::nurbs_curve::circular_arc(k3d::vector3(1, 0, 0), k3d::vector3(0, 0, 1), 0, angle, segments, u_knots, weights, control_points);
		break;
	}

	for (int i = 0; i < control_points.size(); i++)
	{
		MY_DEBUG << output_point(control_points.at(i)) << " x " << weights.at(i) << std::endl;
	}

	//we're going to scale this arc to the size of the distance of each point of the curve
	k3d::mesh::points_t new_points;
	k3d::mesh::weights_t new_weights;

	k3d::mesh::knots_t v_knots;
	v_knots.insert(v_knots.end(), m_nurbs_curve.curve_knots.begin() + curve_knots_begin, m_nurbs_curve.curve_knots.begin() + curve_knots_end);

	k3d::uint_t v_order = m_nurbs_curve.curve_orders.at(curve);
	k3d::uint_t u_order = 3;//the circle is of degree 2

	for (int i = 0; i < m_nurbs_curve.curve_point_counts.at(curve); i++)
	{
		k3d::point3 p = mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_begin + i));
		double w = m_nurbs_curve.curve_point_weights.at(curve_points_begin + i);
		double distance;
		switch (axis)
		{
		case k3d::Z :
			distance = sqrt((p[0] * p[0]) + (p[1] * p[1])); // we want the distance to the z axis
			break;
		case k3d::X :
			distance = sqrt((p[2] * p[2]) + (p[1] * p[1])); // we want the distance to the x axis
			break;
		case k3d::Y :
			distance = sqrt((p[0] * p[0]) + (p[2] * p[2])); // we want the distance to the y axis
			break;
		}

		MY_DEBUG << "Working with point: " << output_point(p) << " and weight " << w << "Creating circle with radius: " << distance << std::endl;

		for (int j = 0; j < control_points.size(); j++)
		{
			k3d::point3 p_u;
			switch (axis)
			{
			case k3d::Z :
				p_u = k3d::point3(control_points.at(j)[0] * distance, control_points.at(j)[1] * distance, p[2]);
				break;
			case k3d::X :
				p_u = k3d::point3(p[0], control_points.at(j)[1] * distance, control_points.at(j)[2] * distance);
				break;
			case k3d::Y :
				p_u = k3d::point3(control_points.at(j)[0] * distance, p[1], control_points.at(j)[2] * distance);
				break;
			}

			new_points.push_back(p_u);
			new_weights.push_back(w * weights.at(j));
		}
	}

	{
		boost::scoped_ptr<k3d::nurbs_patch::primitive> nurbs_patches(get_first_nurbs_patch(m_instance));
		if(!nurbs_patches)
			nurbs_patches.reset(k3d::nurbs_patch::create(m_instance));
		nurbs_patch_modifier mod(m_instance, *nurbs_patches);
		nurbs_patch p;
		p.control_points = new_points;
		p.point_weights = new_weights;
		p.u_knots = u_knots;
		p.v_knots = v_knots;
		p.u_order = u_order;
		p.v_order = v_order;

		mod.insert_patch(p, true);
	}

	if (caps && angle >= M_PI * 2)
	{
		MY_DEBUG << "Creating Caps for the revolved curve" << std::endl;
		int i = 0;
		{
			k3d::point3 p = mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_begin + i));
			double w = m_nurbs_curve.curve_point_weights.at(curve_points_begin + i);
			double distance;
			switch (axis)
			{
			case k3d::Z :
				distance = sqrt((p[0] * p[0]) + (p[1] * p[1])); // we want the distance to the z axis
				break;
			case k3d::X :
				distance = sqrt((p[2] * p[2]) + (p[1] * p[1])); // we want the distance to the x axis
				break;
			case k3d::Y :
				distance = sqrt((p[0] * p[0]) + (p[2] * p[2])); // we want the distance to the y axis
				break;
			}
			nurbs_curve cap_curve;
			for (int j = 0; j < control_points.size(); j++)
			{
				k3d::point3 p_u;
				switch (axis)
				{
				case k3d::Z :
					p_u = k3d::point3(control_points.at(j)[0] * distance, control_points.at(j)[1] * distance, p[2]);
					break;
				case k3d::X :
					p_u = k3d::point3(p[0], control_points.at(j)[1] * distance, control_points.at(j)[2] * distance);
					break;
				case k3d::Y :
					p_u = k3d::point3(control_points.at(j)[0] * distance, p[1], control_points.at(j)[2] * distance);
					break;
				}
				cap_curve.control_points.push_back(p_u);
				cap_curve.curve_point_weights.push_back(w * weights.at(j));
			}
			cap_curve.curve_knots.insert(cap_curve.curve_knots.begin(), u_knots.begin(), u_knots.end());

			k3d::log() << debug << "adding first cap curve" << std::endl;
			int newcurve = add_curve(cap_curve, true);
			k3d::log() << debug << "add_curve returned" << std::endl;
			create_cap(newcurve);
			k3d::log() << debug << "delete curve " << newcurve << std::endl;
			delete_curve(newcurve);
		}

		i = m_nurbs_curve.curve_point_counts.at(curve) - 1;
		{
			k3d::point3 p = mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_begin + i));
			double w = m_nurbs_curve.curve_point_weights.at(curve_points_begin + i);
			double distance;
			switch (axis)
			{
			case k3d::Z :
				distance = sqrt((p[0] * p[0]) + (p[1] * p[1])); // we want the distance to the z axis
				break;
			case k3d::X :
				distance = sqrt((p[2] * p[2]) + (p[1] * p[1])); // we want the distance to the x axis
				break;
			case k3d::Y :
				distance = sqrt((p[0] * p[0]) + (p[2] * p[2])); // we want the distance to the y axis
				break;
			}
			nurbs_curve cap_curve;
			for (int j = 0; j < control_points.size(); j++)
			{
				k3d::point3 p_u;
				switch (axis)
				{
				case k3d::Z :
					p_u = k3d::point3(control_points.at(j)[0] * distance, control_points.at(j)[1] * distance, p[2]);
					break;
				case k3d::X :
					p_u = k3d::point3(p[0], control_points.at(j)[1] * distance, control_points.at(j)[2] * distance);
					break;
				case k3d::Y :
					p_u = k3d::point3(control_points.at(j)[0] * distance, p[1], control_points.at(j)[2] * distance);
					break;
				}
				cap_curve.control_points.push_back(p_u);
				cap_curve.curve_point_weights.push_back(w * weights.at(j));
			}
			cap_curve.curve_knots.insert(cap_curve.curve_knots.begin(), u_knots.begin(), u_knots.end());

			int newcurve = add_curve(cap_curve, true);
			create_cap(newcurve);
			delete_curve(newcurve);
		}
	}
}

nurbs_curve nurbs_curve_modifier::extract_curve(k3d::uint_t curve)
{
	try
	{
		nurbs_curve result;

		const k3d::uint_t curve_points_begin = m_nurbs_curve.curve_first_points.at(curve);
		const k3d::uint_t curve_points_end = curve_points_begin + m_nurbs_curve.curve_point_counts.at(curve);

		const k3d::uint_t curve_knots_begin = m_nurbs_curve.curve_first_knots.at(curve);
		const k3d::uint_t curve_knots_end = curve_knots_begin + (curve_points_end - curve_points_begin) + m_nurbs_curve.curve_orders.at(curve);

		k3d::mesh::indices_t::iterator point_end = m_nurbs_curve.curve_points.begin() + curve_points_end;
		for (k3d::mesh::indices_t::iterator i = m_nurbs_curve.curve_points.begin() + curve_points_begin; i != point_end; ++i)
		{
			result.control_points.push_back(mesh_points->at(*i));
		}

		k3d::mesh::weights_t::iterator weights_end = (m_nurbs_curve.curve_point_weights.begin() + curve_points_end);
		for (k3d::mesh::weights_t::iterator i = m_nurbs_curve.curve_point_weights.begin() + curve_points_begin; i != weights_end; ++i)
		{
			result.curve_point_weights.push_back(*i);
		}

		k3d::mesh::knots_t::iterator knots_end = (m_nurbs_curve.curve_knots.begin() + curve_knots_end);
		for (k3d::mesh::knots_t::iterator i = m_nurbs_curve.curve_knots.begin() + curve_knots_begin; i != knots_end; ++i)
		{
			result.curve_knots.push_back(*i);
		}

		return result;
	}
	catch (...)
	{
		k3d::log() << error << nurbs_debug << "Error in extractCurve " << curve << std::endl;
		nurbs_curve tmp;
		return tmp;
	}
}

void nurbs_curve_modifier::knot_vector_adaption(std::vector<k3d::uint_t> curves)
{
	try
	{
		//get maximum degree
		int max_degree = -1;

		for (int i = 0; i < curves.size(); i++)
		{
			max_degree = Max(max_degree, m_nurbs_curve.curve_orders.at(curves.at(i)));
		}

		//degree elevate all which are below this one
		for (int i = 0; i < curves.size(); i++)
		{
			int dif = max_degree - m_nurbs_curve.curve_orders.at(curves.at(i));

			for (int j = 0; j < dif; j++)
			{
				curve_degree_elevate(curves.at(i));
			}
		}


		MY_DEBUG << "KnotVectorAdaption" << std::endl;

		k3d::mesh::knots_t new_knot_vector;

		for (int i = 0; i < curves.size(); i++)
		{
			k3d::mesh::knots_t::iterator position = new_knot_vector.begin();

			normalize_knot_vector(curves.at(i));

			const k3d::uint_t curve_points_begin = m_nurbs_curve.curve_first_points.at(curves.at(i));
			const k3d::uint_t curve_points_end = curve_points_begin + m_nurbs_curve.curve_point_counts.at(curves.at(i));

			const k3d::uint_t curve_knots_begin = m_nurbs_curve.curve_first_knots.at(curves.at(i));
			const k3d::uint_t curve_knots_end = curve_knots_begin + (curve_points_end - curve_points_begin) + m_nurbs_curve.curve_orders.at(curves.at(i));

			MY_DEBUG << "Checking curve " << curves.at(i) << " for new knots" << std::endl;

			for (k3d::mesh::knots_t::iterator j = m_nurbs_curve.curve_knots.begin() + curve_knots_begin; j != m_nurbs_curve.curve_knots.begin() + curve_knots_end;)
			{
				//add all new knots to new_knot_vector
				if (position == new_knot_vector.end())
				{
					MY_DEBUG << "Found new knot at the end: " << *j << std::endl;
					new_knot_vector.push_back(*j);
					position = new_knot_vector.end();
					j++;
				}
				else if (*position > *j && fabs(*position - *j) > 0.000001)
				{
					MY_DEBUG << "Found new knot before " << *position << ": " << *j << std::endl;
					position = new_knot_vector.insert(position, *j);
					position++;
					j++;
				}
				else if (*position < *j && fabs(*position - *j) > 0.000001)
				{
					position++;
				}
				else
				{
					position++;
					j++;
				}
			}
		}

		std::stringstream str;
		for (int i = 0; i < new_knot_vector.size(); i++)
		{
			str << new_knot_vector.at(i) << " ";
		}
		MY_DEBUG << "Found shared knot vector: " << str.str() << std::endl;

		//now we need to do the same thing but do knot insertion for each knot which is missing..
		for (int i = 0; i < curves.size(); i++)
		{
			const k3d::uint_t curve_points_begin = m_nurbs_curve.curve_first_points.at(curves.at(i));
			const k3d::uint_t curve_points_end = curve_points_begin + m_nurbs_curve.curve_point_counts.at(curves.at(i));

			const k3d::uint_t curve_knots_begin = m_nurbs_curve.curve_first_knots.at(curves.at(i));
			const k3d::uint_t curve_knots_end = curve_knots_begin + (curve_points_end - curve_points_begin) + m_nurbs_curve.curve_orders.at(curves.at(i));

			int k = 0;

			while (k < new_knot_vector.size())
			{
				if (fabs(m_nurbs_curve.curve_knots.at(curve_knots_begin + k) - new_knot_vector.at(k)) < 0.000001)
				{
					//we dont need to insert anything
					k++;
				}
				else
				{
					//insert the missing knot
					if (!curve_knot_insertion(curves.at(i), new_knot_vector.at(k), 1))
					{
						k3d::log() << error << nurbs_debug << "Tried to insert existing knot, something went wrong" << std::endl;
						break;
					}
					k = 0; //start looking again as knot vector changed?
				}
			}
		}
	}
	catch (...)
	{
		MY_DEBUG << "Access violation in knot_vector_adaption" << std::endl;
	}
}

void nurbs_curve_modifier::ruled_surface(k3d::uint_t curve1, k3d::uint_t curve2)
{
	try
	{
		MY_DEBUG << "Ruled Surface between " << curve1 << " and " << curve2 << std::endl;
		int order = Max(m_nurbs_curve.curve_orders.at(curve1), m_nurbs_curve.curve_orders.at(curve2));
		int diff = order - m_nurbs_curve.curve_orders.at(curve1);

		for (int i = 0; i < diff; i++)
		{
			curve_degree_elevate(curve1);
		}

		diff = order - m_nurbs_curve.curve_orders.at(curve2);
		for (int i = 0; i < diff; i++)
		{
			curve_degree_elevate(curve2);
		}

		std::vector<k3d::uint_t> curves;
		curves.push_back(curve1);
		curves.push_back(curve2);

		normalize_knot_vector(curve1);
		normalize_knot_vector(curve2);

		knot_vector_adaption(curves);

		k3d::mesh::points_t patch_points;
		k3d::mesh::weights_t patch_point_weights;

		k3d::mesh::knots_t u_knots;
		u_knots.push_back(0);
		u_knots.push_back(0);
		u_knots.push_back(1);
		u_knots.push_back(1);

		k3d::mesh::knots_t v_knots;

		const k3d::uint_t curve_points_begin = m_nurbs_curve.curve_first_points.at(curve1);
		const k3d::uint_t curve_points_end = curve_points_begin + m_nurbs_curve.curve_point_counts.at(curve1);

		const k3d::uint_t curve2_points_begin = m_nurbs_curve.curve_first_points.at(curve2);
		const k3d::uint_t curve2_points_end = curve2_points_begin + m_nurbs_curve.curve_point_counts.at(curve2);

		const k3d::uint_t curve_knots_begin = m_nurbs_curve.curve_first_knots.at(curve1);
		const k3d::uint_t curve_knots_end = curve_knots_begin + (curve_points_end - curve_points_begin) + m_nurbs_curve.curve_orders.at(curve1);

		//copy knots
		for (int i = curve_knots_begin; i < curve_knots_end; i++)
		{
			v_knots.push_back(m_nurbs_curve.curve_knots.at(i));
		}

		k3d::uint_t v_order = m_nurbs_curve.curve_orders.at(curve1);

		for (int i = 0; i < m_nurbs_curve.curve_point_counts.at(curve1); i++)
		{
			patch_points.push_back(mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_begin + i)));
			patch_points.push_back(mesh_points->at(m_nurbs_curve.curve_points.at(curve2_points_begin + i)));

			patch_point_weights.push_back(m_nurbs_curve.curve_point_weights.at(curve_points_begin + i));
			patch_point_weights.push_back(m_nurbs_curve.curve_point_weights.at(curve2_points_begin + i));
		}

		nurbs_patch p;
		p.u_knots = u_knots;
		p.v_knots = v_knots;
		p.u_order = 2;
		p.v_order = v_order;
		p.point_weights = patch_point_weights;
		p.control_points = patch_points;

		boost::scoped_ptr<k3d::nurbs_patch::primitive> nurbs_patches(get_first_nurbs_patch(m_instance));
			if(!nurbs_patches)
				nurbs_patches.reset(k3d::nurbs_patch::create(m_instance));
		
		nurbs_patch_modifier mod(m_instance, *nurbs_patches);
		mod.insert_patch(p, true);
	}
	catch (std::exception& e)
	{
		MY_DEBUG << "Error in RuledSurface: " << e.what() << std::endl;
	}
	catch (...)
	{
		MY_DEBUG << "Error in RuledSurface" << std::endl;
	}
}

bool nurbs_curve_modifier::is_closed(k3d::uint_t curve)
{
	k3d::uint_t curve_points_begin = m_nurbs_curve.curve_first_points.at(curve);
	k3d::uint_t curve_points_end = curve_points_begin + m_nurbs_curve.curve_point_counts.at(curve);

	return (m_nurbs_curve.curve_points.at(curve_points_begin) == m_nurbs_curve.curve_points.at(curve_points_end - 1));
}

bool nurbs_curve_modifier::create_cap(k3d::uint_t curve)
{
	try
	{
		MY_DEBUG << "Create Cap" << std::endl;
		normalize_knot_vector(curve);
		//check whether curve is a loop
		const k3d::uint_t curve_points_begin = m_nurbs_curve.curve_first_points.at(curve);
		const k3d::uint_t curve_points_end = curve_points_begin + m_nurbs_curve.curve_point_counts.at(curve);

		if (m_nurbs_curve.curve_points.at(curve_points_begin) == m_nurbs_curve.curve_points.at(curve_points_end - 1) || point3_float_equal(mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_begin)), mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_end - 1)), 0.000001))
		{
			MY_DEBUG << "Found a loop" << std::endl;
			k3d::mesh t;
			boost::scoped_ptr<k3d::nurbs_curve::primitive> temp_curve(k3d::nurbs_curve::create(t));
			k3d::log() << debug << "temp curve curve count: " << temp_curve->curve_first_points.size() << std::endl;
			nurbs_curve_modifier mod(t, *temp_curve);
			nurbs_curve tmp = extract_curve(curve);
			double u = tmp.curve_knots.size() * 0.5;
			u = tmp.curve_knots.at(floor(u));
			mod.add_curve(tmp, false);

			MY_DEBUG << "Curve extracted" << std::endl;

			//split it up at the middle
			mod.normalize_knot_vector(0);
			k3d::log() << debug << "create_cap: attempt to split curve at " << u << std::endl;
			if(!mod.split_curve_at(0, u, false))
			{
				k3d::log() << debug << "curve splitting failed" << std::endl;
				return false;
			}
			mod.flip_curve(0);
			//->ruled surface
			mod.ruled_surface(0, 1);

			boost::scoped_ptr<k3d::nurbs_patch::primitive> nurbs_patches(get_first_nurbs_patch(mod.m_instance));
			return_val_if_fail(nurbs_patches, false);
			
			//now extract the surface and add it to this mesh
			MY_DEBUG << "Extracting patch" << std::endl;
			nurbs_patch_modifier mod2(mod.m_instance, *nurbs_patches);
			nurbs_patch p = mod2.extract_patch(mod2.get_patch_count() - 1);

			//insert with shared points!!
			MY_DEBUG << "Insert patch into this mesh" << std::endl;
			boost::scoped_ptr<k3d::nurbs_patch::primitive> new_nurbs_patches(get_first_nurbs_patch(m_instance));
			if(!new_nurbs_patches)
				new_nurbs_patches.reset(k3d::nurbs_patch::create(m_instance));
			nurbs_patch_modifier mod3(m_instance, *new_nurbs_patches);
			mod3.insert_patch(p, true);
			MY_DEBUG << "Done adding patch" << std::endl;
			return true;
		}
		return false;
	}
	catch (std::exception& e)
	{
		MY_DEBUG << "Error in CreateCap: " << e.what() << std::endl;
		return false;
	}
	catch (...)
	{
		MY_DEBUG << "Error in CreateCap" << std::endl;
		return false;
	}
}

void nurbs_curve_modifier::skinned_surface(std::vector<k3d::uint_t> curves, k3d::axis axis)
{
	MY_DEBUG << "Skinned Surface along " << axis << std::endl;

	if (curves.size() < 2)
	{
		k3d::log() << error << nurbs_debug << "Too few curves selected for skinning" << std::endl;
		return;
	}
	//adapt knot vectors
	knot_vector_adaption(curves);

	//calculate center for each curve
	std::vector<k3d::vector3> centers;

	for (int i = 0; i < curves.size(); i++)
	{
		k3d::point3 c;
		const k3d::uint_t curve_points_begin = m_nurbs_curve.curve_first_points.at(curves.at(i));
		const k3d::uint_t curve_points_end = curve_points_begin + m_nurbs_curve.curve_point_counts.at(curves.at(i));
		for (int j = curve_points_begin; j < curve_points_end; j++)
		{
			c = c + mesh_points->at(m_nurbs_curve.curve_points.at(j));
		}
		c = (1.0 / m_nurbs_curve.curve_point_counts.at(curves.at(i))) * c;
		centers.push_back(k3d::vector3(c[0], c[1], c[2]));
	}

	//order along y axis
	std::deque<k3d::uint_t> ordered_by_axis;

	ordered_by_axis.push_back(0);
	int component;

	switch (axis)
	{
	case k3d::X :
		component = 0;
		break;
	case k3d::Y :
		component = 1;
		break;
	case k3d::Z :
		component = 2;
		break;
	}

	for (int i = 1; i < curves.size(); i++)
	{
		if (centers.at(ordered_by_axis.back())[component] < centers.at(i)[component])
		{
			ordered_by_axis.push_back(i);
		}
		else if (centers.at(ordered_by_axis.front())[component] > centers.at(i)[component])
		{
			ordered_by_axis.push_front(i);
		}
		else
		{
			int j = 0;
			while (j < ordered_by_axis.size() && centers.at(ordered_by_axis.at(j))[component] < centers.at(i)[component])
			{
				j++;
			}
			std::deque<k3d::uint_t>::iterator iter = ordered_by_axis.begin() + j;
			ordered_by_axis.insert(iter, i);
		}
	}

	//create surface
	nurbs_patch skin;
	std::vector<nurbs_curve> rips;

	for (int i = 0; i < ordered_by_axis.size(); i++)
	{
		rips.push_back(extract_curve(ordered_by_axis.at(i)));
	}

	skin.u_order = rips.front().curve_knots.size() - rips.front().control_points.size();
	skin.u_knots = rips.front().curve_knots;
	skin.v_order = 3;

	skin.v_knots.push_back(0);
	skin.v_knots.push_back(0);
	for (int i = 0; i < ordered_by_axis.size() - 1; i++)
	{
		skin.v_knots.push_back(i);
	}
	skin.v_knots.push_back(skin.v_knots.back());
	skin.v_knots.push_back(skin.v_knots.back());

	for (std::vector<nurbs_curve>::iterator i = rips.begin(); i != rips.end(); ++i)
	{
		skin.point_weights.insert(skin.point_weights.end(), (*i).curve_point_weights.begin(), (*i).curve_point_weights.end());
		skin.control_points.insert(skin.control_points.end(), (*i).control_points.begin(), (*i).control_points.end());
	}
	
	boost::scoped_ptr<k3d::nurbs_patch::primitive> nurbs_patches(get_first_nurbs_patch(m_instance));
	if(!nurbs_patches)
		nurbs_patches.reset(k3d::nurbs_patch::create(m_instance));

	nurbs_patch_modifier mod(m_instance, *nurbs_patches);
	mod.insert_patch(skin, true);
}

void nurbs_curve_modifier::sweep_surface(k3d::uint_t curve1, k3d::uint_t curve2, k3d::uint_t segments, bool create_caps)
{
	try
	{
		k3d::point3 c;

		normalize_knot_vector(curve1);
		normalize_knot_vector(curve2);

		const k3d::uint_t curve_points_begin = m_nurbs_curve.curve_first_points.at(curve1);
		const k3d::uint_t curve_points_end = curve_points_begin + m_nurbs_curve.curve_point_counts.at(curve1);

		bool isClosed = point3_float_equal(mesh_points->at(m_nurbs_curve.curve_points.at(m_nurbs_curve.curve_first_points.at(curve2))), mesh_points->at(m_nurbs_curve.curve_points.at(m_nurbs_curve.curve_first_points.at(curve2) + m_nurbs_curve.curve_point_counts.at(curve2) - 1)), 0.000001);

		const k3d::uint_t curve_knots_begin = m_nurbs_curve.curve_first_knots.at(curve1);
		const k3d::uint_t curve_knots_end = curve_knots_begin + (curve_points_end - curve_points_begin) + m_nurbs_curve.curve_orders.at(curve1);

		MY_DEBUG << "Calculating center" << std::endl;

		for (int j = curve_points_begin; j < curve_points_end; j++)
		{
			bool already_there = false;
			for (int i = curve_points_begin; i < j; i++)
			{
				if (point3_float_equal(mesh_points->at(m_nurbs_curve.curve_points.at(j)), mesh_points->at(m_nurbs_curve.curve_points.at(i)), 0.000001))
					already_there = true;
			}

			if (!already_there)
				c = c + mesh_points->at(m_nurbs_curve.curve_points.at(j));
		}
		c = (1.0 / m_nurbs_curve.curve_point_counts.at(curve1)) * c;

		MY_DEBUG << "Getting first tangent" << std::endl;

		k3d::point3 rt = calculate_curve_tangent(curve2, 0.0);

		k3d::mesh::points_t control_points;
		k3d::mesh::weights_t new_weights;

		double step = 1.0 / segments;

		for (int i = 0; i <= segments; i++)
		{
			double u_value = step * i;
			if (u_value == 1.0)
				u_value = 0.99;
			k3d::point4 p = curve_point(curve2, u_value);
			if (p[3] == 0)
			{
				k3d::log() << error << nurbs_debug << "Point weight equals zero" << std::endl;
				return;
			}



			k3d::point3 t = calculate_curve_tangent(curve2, step * i);

			//cross product of the 2 tangents
			k3d::vector3 rotation_axis(t[1]*rt[2] - t[2]*rt[1], t[2]*rt[0] - t[0]*rt[2], t[0]*rt[1] - t[1]*rt[0]);
			//angle between the 2 tangents: r*rt / (length(r) * length(rt)) = cos alpha
			double alpha = t[0] * rt[0] + t[1] * rt[1] + t[2] * rt[2];
			if (alpha >= 1.0)
			{
				alpha = 0.0;
			}
			else
			{
				alpha = - acos(alpha);
			}

			k3d::matrix4 rot_mat = k3d::rotate3(alpha, rotation_axis);

			for (int j = curve_points_begin; j < curve_points_end; j++)
			{
				if (i == segments && isClosed)
				{
					control_points.push_back(control_points.at(j - curve_points_begin));
					new_weights.push_back(new_weights.at(j - curve_points_begin));
				}
				else
				{
					k3d::point3 cp = mesh_points->at(m_nurbs_curve.curve_points.at(j)) + (-c);
					control_points.push_back(k3d::point3(p[0] / p[3], p[1] / p[3], p[2] / p[3]) + rot_mat * cp);
					new_weights.push_back(m_nurbs_curve.curve_point_weights.at(j) * p[3]);
				}
			}
		}
		
		boost::scoped_ptr<k3d::nurbs_patch::primitive> nurbs_patches(get_first_nurbs_patch(m_instance));
		if(!nurbs_patches)
			nurbs_patches.reset(k3d::nurbs_patch::create(m_instance));

		nurbs_patch_modifier mod(m_instance, *nurbs_patches);
		nurbs_patch p;
		p.control_points = control_points;
		p.point_weights = new_weights;

		nurbs_curve tmp = extract_curve(curve1);
		p.u_knots = tmp.curve_knots;

		p.v_knots.push_back(0);
		p.v_knots.push_back(0);
		for (int i = 0; i < segments; i++)
		{
			p.v_knots.push_back(i);
		}
		p.v_knots.push_back(p.v_knots.back());
		p.v_knots.push_back(p.v_knots.back());

		p.u_order = m_nurbs_curve.curve_orders.at(curve1);
		p.v_order = 3;

		mod.insert_patch(p, true);
	}
	catch (...)
	{
		k3d::log() << error << nurbs_debug << "Error in SweepSurface" << std::endl;
	}
}

k3d::point3 nurbs_curve_modifier::calculate_curve_tangent(k3d::uint_t curve, double u)
{
	normalize_knot_vector(curve);
	k3d::point4 after_u;
	if (u < 0.01)
	{
		k3d::point4 at_u = curve_point(curve, 0.0);
		after_u = curve_point(curve, 0.01);
		MY_DEBUG << output_point(after_u) << std::endl;
		after_u = after_u + (-at_u);
	}
	else if (u > 0.99)
	{
		k3d::point4 at_u = curve_point(curve, 1.0);
		after_u = curve_point(curve, 0.99);
		after_u = at_u + (-after_u);
	}
	else //we're in the middle of the curve
	{
		after_u = curve_point(curve, u + 0.005);
		MY_DEBUG << output_point(after_u) << std::endl;
		k3d::point4 before_u = curve_point(curve, u - 0.005);
		MY_DEBUG << output_point(before_u) << std::endl;
		after_u = after_u + (-before_u);
	}

	k3d::point3 tangent(after_u[0], after_u[1], after_u[2]);

	tangent = tangent / sqrt(tangent[0] * tangent[0] + tangent[1] * tangent[1] + tangent[2] * tangent[2]);

	MY_DEBUG << "Calculated normalized Tangent: " << output_point(tangent) << std::endl;

	return tangent;
}

void nurbs_curve_modifier::polygonize_curve(k3d::uint_t curve, k3d::uint_t segments, bool del_curve)
{
	try
	{
		MY_DEBUG << "Polygonize_curve" << std::endl;

		boost::scoped_ptr<k3d::linear_curve::primitive> linear_curves(k3d::linear_curve::create(m_instance));

		linear_curves->periodic.push_back(false);
		linear_curves->material.push_back(m_nurbs_curve.material.back());

		normalize_knot_vector(curve);

		linear_curves->curve_first_points.push_back(0);
		linear_curves->curve_point_counts.push_back(segments + 1);
		linear_curves->curve_selections.push_back(0.0);

		double step = 1.0 / segments;

		for (int i = 0; i <= segments; i++)
		{
			k3d::point4 ph = curve_point(curve, i * step);
			if (ph[3] == 0.0)
			{
				k3d::log() << error << nurbs_debug << "Point weight equals zero" << std::endl;
			}
			k3d::point3 p(ph[0] / ph[3], ph[1] / ph[3], ph[2] / ph[3]);
			MY_DEBUG << "Adding point " << p << " as segment " << i << " to the linear curve" << std::endl;

			linear_curves->curve_points.push_back(insert_point(p, false));
		}

		if (del_curve)
		{
			delete_curve(curve);
		}
	}
	catch (std::exception& e)
	{
		k3d::log() << error << nurbs_debug << "Error in Polygonize_curve: " << e.what() << std::endl;
	}
	catch (...)
	{
		k3d::log() << error << nurbs_debug << "Error in polygonize_curve" << std::endl;
	}
}

nurbs_trim_curve nurbs_curve_modifier::create_trim_curve(k3d::uint_t curve)
{
	nurbs_trim_curve result;
	nurbs_curve temp = extract_curve(curve);

	result.curve_knots.insert(result.curve_knots.begin(), temp.curve_knots.begin(), temp.curve_knots.end());
	result.curve_point_weights.insert(result.curve_point_weights.begin(), temp.curve_point_weights.begin(), temp.curve_point_weights.end());
	result.curve_point_weights.front() = 1.0;
	result.curve_point_weights.back() = 1.0;

	double min_x, min_y, max_x, max_y;

	for (int i = 0; i < temp.control_points.size() - 1; i++)
	{
		if (i == 0)
		{
			max_x = min_x = temp.control_points.at(i)[0];
			max_y = min_y = temp.control_points.at(i)[1];
		}
		else
		{
			if (max_x < temp.control_points.at(i)[0])
				max_x = temp.control_points.at(i)[0];
			if (min_x > temp.control_points.at(i)[0])
				min_x = temp.control_points.at(i)[0];

			if (max_y < temp.control_points.at(i)[1])
				max_y = temp.control_points.at(i)[1];
			if (min_y > temp.control_points.at(i)[1])
				min_y = temp.control_points.at(i)[1];
		}
	}

	for (int i = 0; i < temp.control_points.size() - 1; i++)
	{
		result.control_points.push_back(k3d::point2((temp.control_points.at(i)[0] - min_x) / max_x, (temp.control_points.at(i)[1] - min_y) / max_y));
	}
	result.control_points.push_back(result.control_points.front());

	return result;
}

void nurbs_curve_modifier::select_curve(k3d::uint_t curve)
{
	for (int i = 0; i < count_all_curves_in_groups(); i++)
	{
		if (i != curve)
			m_nurbs_curve.curve_selections.at(i) = 0.0;
		else
			m_nurbs_curve.curve_selections.at(i) = 1.0;
	}
}

void nurbs_curve_modifier::open_up_curve(k3d::uint_t curve)
{
	const k3d::uint_t curve_points_begin = m_nurbs_curve.curve_first_points.at(curve);
	const k3d::uint_t curve_points_end = curve_points_begin + m_nurbs_curve.curve_point_counts.at(curve);

	if (!is_closed(curve))
	{
		k3d::log() << error << nurbs_debug << "Curve was not closed, cannot open it up" << std::endl;
		return;
	}

	mesh_points->push_back(mesh_points->at(m_nurbs_curve.curve_points.at(curve_points_begin)));
	point_selection->push_back(1.0);
	m_nurbs_curve.curve_points.at(curve_points_end - 1) = mesh_points->size() - 1;
}

}//nurbs

}//module

