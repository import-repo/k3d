/*
  Copyright 2005-2007 Adobe Systems Incorporated
  Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
  or a copy at http://opensource.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef GIL_DEVICE_N_H
#define GIL_DEVICE_N_H


////////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Support for color space of N channels and variants
/// \author Lubomir Bourdev and Hailin Jin \n
///         Adobe Systems Incorporated
/// \date 2005-2007 \n Last updated on March 8, 2006
////////////////////////////////////////////////////////////////////////////////////////

#include "gil_config.hpp"
#include <boost/type_traits.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/vector_c.hpp>

namespace boost { namespace gil {

/// \brief unnamed color
/// \ingroup ColorNameModel
template <int N> struct devicen_color_t {};

template <int N> struct devicen_t;

/// \brief unnamed color space of two channels
/// \ingroup ColorSpaceModel
template <> struct devicen_t<2> : public mpl::vector2<devicen_color_t<0>, devicen_color_t<1> > {};

/// \brief unnamed color space of three channels
/// \ingroup ColorSpaceModel
template <> struct devicen_t<3> : public mpl::vector3<devicen_color_t<0>, devicen_color_t<1>, devicen_color_t<2> > {};

/// \brief unnamed color space of four channels
/// \ingroup ColorSpaceModel
template <> struct devicen_t<4> : public mpl::vector4<devicen_color_t<0>, devicen_color_t<1>, devicen_color_t<2>, devicen_color_t<3> > {};

/// \brief unnamed color space of five channels
/// \ingroup ColorSpaceModel
template <> struct devicen_t<5> : public mpl::vector5<devicen_color_t<0>, devicen_color_t<1>, devicen_color_t<2>, devicen_color_t<3>, devicen_color_t<4> > {};

/// \brief unnamed color layout of up to five channels
/// \ingroup LayoutModel
template <int N> struct devicen_layout_t : public layout<devicen_t<N> > {};

/// \ingroup ImageViewConstructors
/// \brief from 2-channel planar data
template <typename IC>
inline typename type_from_x_iterator<planar_pixel_iterator<IC,devicen_t<2> > >::view_t planar_devicen_view(int width, int height, IC c0, IC c1, std::ptrdiff_t rowsize_in_bytes) {
    return typename type_from_x_iterator<planar_pixel_iterator<IC,devicen_t<2> > >::view_t(width, height, planar_pixel_iterator<IC,devicen_t<2> >(c0,c1), rowsize_in_bytes);
}

/// \ingroup ImageViewConstructors
/// \brief from 3-channel planar data
template <typename IC>
inline typename type_from_x_iterator<planar_pixel_iterator<IC,devicen_t<3> > >::view_t planar_devicen_view(int width, int height, IC c0, IC c1, IC c2, std::ptrdiff_t rowsize_in_bytes) {
    return typename type_from_x_iterator<planar_pixel_iterator<IC,devicen_t<3> > >::view_t(width, height, planar_pixel_iterator<IC,devicen_t<3> >(c0,c1,c2), rowsize_in_bytes);
}

/// \ingroup ImageViewConstructors
/// \brief from 4-channel planar data
template <typename IC>
inline typename type_from_x_iterator<planar_pixel_iterator<IC,devicen_t<4> > >::view_t planar_devicen_view(int width, int height, IC c0, IC c1, IC c2, IC c3, std::ptrdiff_t rowsize_in_bytes) {
    return typename type_from_x_iterator<planar_pixel_iterator<IC,devicen_t<4> > >::view_t(width, height, planar_pixel_iterator<IC,devicen_t<4> >(c0,c1,c2,c3), rowsize_in_bytes);
}

/// \ingroup ImageViewConstructors
/// \brief from 5-channel planar data
template <typename IC>
inline typename type_from_x_iterator<planar_pixel_iterator<IC,devicen_t<5> > >::view_t planar_devicen_view(int width, int height, IC c0, IC c1, IC c2, IC c3, IC c4, std::ptrdiff_t rowsize_in_bytes) {
    return typename type_from_x_iterator<planar_pixel_iterator<IC,devicen_t<5> > >::view_t(width, height, planar_pixel_iterator<IC,devicen_t<5> >(c0,c1,c2,c3,c4), rowsize_in_bytes);
}

} }  // namespace boost::gil

#endif
