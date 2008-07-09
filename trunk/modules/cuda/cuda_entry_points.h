#ifndef CUDA_ENTRY_POINTS_H
#define CUDA_ENTRY_POINTS_H

// K-3D
// Copyright (c) 1995-2008, Timothy M. Shead
//
// Contact: tshead@k-3d.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

/** \file
	\author Evan Lezar (evanlezar@gmail.com)
*/

#include <k3d-platform-config.h>

#if defined K3D_API_WIN32
	#define K3D_CUDA_DECLSPEC __declspec(dllexport)
#else
	#define K3D_CUDA_DECLSPEC
#endif // !K3D_API_WIN32

// define the types of kernels supported
#define CUDA_BITMAP_ADD 0x00
#define CUDA_BITMAP_MULTIPLY 0x01
#define CUDA_BITMAP_SUBTRACT 0x02
#define CUDA_BITMAP_COLOR_MONOCHROME 0x03
#define CUDA_BITMAP_GAMMA 0x04

// define the profiling strings
#define PROFILE_STRING_HOST_TO_DEVICE "Copy from Host to Device"
#define PROFILE_STRING_EXECUTE_KERNEL "Execute Kernel"
#define PROFILE_STRING_DEVICE_TO_HOST "Copy from Device to Host"

/**
 * a struct to pass timing info back from external C functions
 */
typedef struct
{
	int numEntries;
	double *timings;
	char **labels;	
} timingInfo_t;

// forward declaration of the entry functions
// split the entry functions for timing reasons
extern "C" K3D_CUDA_DECLSPEC void CUDA_initialize_device();

extern "C" K3D_CUDA_DECLSPEC void bitmap_arithmetic_kernel_entry(int operation, unsigned short* p_deviceImage, int width, int height, float value);
extern "C" K3D_CUDA_DECLSPEC void bitmap_color_monochrome_kernel_entry(unsigned short* p_deviceImage, int width, int height, float redWeight, float greenWeight, float blueWeight);

extern "C" K3D_CUDA_DECLSPEC void apply_linear_transform_to_point_data ( float *device_points, float *device_matrix, int num_points );

extern "C" K3D_CUDA_DECLSPEC void allocate_device_memory ( void** device_pointer, int size_in_bytes );
extern "C" K3D_CUDA_DECLSPEC void copy_from_host_to_device ( void* device_pointer, const void* host_pointer, int size_in_bytes );
extern "C" K3D_CUDA_DECLSPEC void copy_from_device_to_host ( void* host_pointer, const void* device_pointer, int size_in_bytes );
extern "C" K3D_CUDA_DECLSPEC void free_device_memory ( void* device_pointer );
extern "C" K3D_CUDA_DECLSPEC void allocate_pinned_host_memory ( void** pointer_on_host, size_t size_in_bytes );
extern "C" K3D_CUDA_DECLSPEC void free_pinned_host_memory ( void* pointer_on_host );


extern "C" K3D_CUDA_DECLSPEC void copy_and_bind_texture_to_array( void** cudaArrayPointer, float* arrayData, int width, int height );
extern "C" K3D_CUDA_DECLSPEC void free_CUDA_array ( void* cudaArrayPointer );

extern "C" K3D_CUDA_DECLSPEC void transform_points_synchronous ( double *InputPoints, double *PointSelection, double *OutputPoints, int num_points, timingInfo_t* tInfo );
extern "C" K3D_CUDA_DECLSPEC void transform_points_asynchronous ( double *InputPoints, double *PointSelection, double *OutputPoints, int num_points, timingInfo_t* tInfo );

extern "C" K3D_CUDA_DECLSPEC void subdivide_edges_split_point_calculator ( unsigned int* edge_indices, 
                                                         unsigned int num_edge_indices, 
                                                         float* points_and_selection,
                                                         unsigned int num_points,  
                                                         unsigned int* edge_point_indices,
                                                         unsigned int* clockwise_edge_indices,
                                                         float* new_points_and_selection,
                                                         int num_split_points );

extern "C" K3D_CUDA_DECLSPEC void copy_2D_from_host_to_device_with_padding ( void* device_pointer, const void* host_pointer, size_t device_pitch, size_t host_pitch, size_t width_in_bytes, size_t rows );

extern "C" K3D_CUDA_DECLSPEC void synchronize_threads ();

#endif // !CUDA_ENTRY_POINTS_H
