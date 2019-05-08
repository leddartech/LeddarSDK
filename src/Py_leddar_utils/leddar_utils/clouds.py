'''
Created on Mar 5, 2018

@author: Maxime Lemonnier, Guillaume Dumont

@file: clouds.py

@summary: utilities to unproject distances into 3D

@copyright: Copyright (c) 2018 LeddarTech Inc. All rights reserved.
'''

import numpy as np
import math
import datetime

def grid(v, h, v_from, v_to, h_from, h_to, dtype = np.float32):
    '''
    Computes a matrix of all possible pairs of angles in the 2d field of view.

    \param v vertical resolution
    \param h horizontal resolution
    \param v_fov vertical field of view (degrees)
    \param h_fov horizontal field of view (degrees)
    \param dtype the numpy data type
    '''
    # If you are wondering why there is a complex number here. Read the
    # numpy mgrid documentation. The lines below are equivalent to:
    # a = np.linspace(v_from, v_to, v)
    # b = np.linspace(h_from, h_to, h)
    # b, a = np.meshgrid(b, a)
    a, b = np.mgrid[  v_from:v_to:complex(0,v)
                    , h_from:h_to:complex(0,h)]
    return np.c_[a.ravel(), b.ravel()].astype(dtype)

def from_specs_dict(specs):
    return (specs[k] for k in ['v', 'h', 'v_fov', 'h_fov'])

def angles(v, h = None, v_fov = None, h_fov = None, dtype = np.float32):
    '''
    Computes a matrix of all possible pairs of angles in the 2d field of view.

    The generated grid follows the LCA axis system convention. That is the
    bottom-left corner (0, 0) corresponds to (-v_fov/2, -h_fov/2) and the top
    right corner (v-1, h-1) is (+v_fov/2, +h_fov/2). The grid is generated in
    a row-major order.

    \param v vertical resolution (or a dict with keys v', 'h', 'v_fov', 'h_fov')
    \param h horizontal resolution
    \param v_fov vertical field of view (degrees)
    \param h_fov horizontal field of view (degrees)
    \param dtype the numpy data type
    '''

    if isinstance(v, dict):
        v, h, v_fov, h_fov = from_specs_dict(v)

    v_fov_rad = math.radians(v_fov)
    h_fov_rad = math.radians(h_fov)

    v_offset = v_fov_rad/v/2
    h_offset = h_fov_rad/h/2

    return grid(v,h, -v_fov_rad/2 + v_offset, v_fov_rad/2 - v_offset
                   , -h_fov_rad/2 + h_offset, h_fov_rad/2 - h_offset, dtype)

# DISCLAIMER: everything that follows is solely based on glimpses of information
# that I (Guillaume Dumont) gathered here and there and some reverse engineering.
#
# The LCA2 and LCA3 have an unconventional axis system (at least for someone
# coming from a computer vision background) and here we describe how we
# transform the echo packages to point clouds and images.
#
# The leddar spherical projection model is usually defined with vertical and
# horizontal number of segments and field of views (FOV). These are commonly
# referred to as specs and represented in python code as:
#
# - `v`: vertical number of segments (pixels)
# - `h`: horizontal number of segments (pixels)
# - `v_fov`: vertical FOV (degrees)
# - `h_fov`: horizontal FOV (degrees)
#
# The vertical and horizontal naming is a bit misleading since it actually
# corresponds to the laser scan orientation and to the imaging orientation. For
# the LCA2 the scan axis is vertical and the  imager is horizontal. But for the
# LCA3 it is the opposite.
#
# The `clouds.angles` function generates a grid of angles that correspond to the
# spherical projection model of the LCAx. The `clouds.directions` function takes
# those angles and converts them to a series of unit spherical direction
# vectors. The spherical axis system defined by those two functions can be used
# to convert the indices of the echoes package to a point cloud that follows the
# camera axis system. This process looks like:
#
# from leddar_utils import clouds
#
# v = 8
# h = 32
# v_fov = 20
# h_fov = 30
#
# angles = clouds.angles(v, h, v_fov, h_fov)
# echoes = ... # load some echo package
# indices = echoes['indices']
# distances = echoes['distances']
# pts = clouds.to_point_cloud(angles, indices, distances)
#
# This will generate the correct point cloud for the LCA2. Since the `v` and `h`
# axes are inverted in the LCA3 we need to apply a rotation to the resulting
# point cloud, i.e.
#
# rotate 90 deg counter-clockwise
#
# R = np.array(
#  [[ 0,  1,  0],
#   [-1,  0,  0],
#   [ 0,  0,  1],]
# )
#
# lca3_pts_in_camera_system = np.dot(R, pts.T).T
#
# To generate an image from the amplitudes of the echo package for the LCA2 it
# is sufficient to use the echoes indices as linear indexers of a numpy array
# and flip the result upside down. Indeed, the convention for images is to have
# the origin in the top left corner but the convention for the LCAx is the
# bottom left corner. The process looks like this:
#
# image = np.zeros((v, h))
# image.flat[indices] = amplitudes
# image = np.flipud(image)
#
# for the LCA3 since the `v` and `h` axes are inverted we need to apply a
# 90 degree counter clockwise rotation after:
#
# image = np.rot90(image, k=1)
#

def direction(theta_x, theta_y):
    '''
    Convert angles of a spherical axis sytem into a cartesian direction vector.
    The cartesian axis system is the camera axis system.

      z

      +-------> x
      |
      |
    y v

    The z axis enters your screen (or paper if you are the kind of person that
    still prints code).

    Angles are measured as

    In the x-z plane:

                       0

                    z  ^
                       |
                       |
                       |
    - 90  ---------------------------> x  + 90

    x = sin(theta_y)
    z = cos(theta_y)

    In the y-z plane:

     -90  ---------------------------> z  + 90
                       |
                       |
                       |
                       v y

    y = - sin(theta_x)
    z = cos(theta_x)

    So the x, y, z coordinates should follow the equations below

    x = sin(theta_y)
    y = -sin(theta_x) * cos(theta_y)
    z = cos(theta_y) * cos(theta_x)
    '''
    cos_theta_y = np.cos(theta_y)
    x = np.sin(theta_y)
    y = -np.sin(theta_x) * cos_theta_y
    z = cos_theta_y * np.cos(theta_x)
    return [x, y, z]

def directions(angles):
    '''Generate a set of cartesian direction vectors from a grid of
    spherical coordinates angles. This function uses the same convention as
    the `direction` function.
    '''
    thetas_x, thetas_y = angles.T

    cos_thetas_y = np.cos(thetas_y)
    x = np.sin(thetas_y)
    y = -np.sin(thetas_x) * cos_thetas_y
    z = cos_thetas_y * np.cos(thetas_x)
    return np.stack([x, y, z], axis=1)

def quad_directions(v, h = None, v_fov = None, h_fov = None, dtype = np.float32):

    if isinstance(v, dict):
        v, h, v_fov, h_fov = from_specs_dict(v)

    v_fov_rad = math.radians(v_fov)
    h_fov_rad = math.radians(h_fov)

    v_cell_size = v_fov_rad/v
    h_cell_size = h_fov_rad/h

    return np.vstack((
     directions(grid(v,h,-v_fov_rad/2              ,v_fov_rad/2             ,-h_fov_rad/2             , h_fov_rad/2             , dtype))
    ,directions(grid(v,h,-v_fov_rad/2+v_cell_size  ,v_fov_rad/2+v_cell_size ,-h_fov_rad/2             , h_fov_rad/2             , dtype))
    ,directions(grid(v,h,-v_fov_rad/2+v_cell_size  ,v_fov_rad/2+v_cell_size ,-h_fov_rad/2+h_cell_size , h_fov_rad/2+h_cell_size , dtype))
    ,directions(grid(v,h,-v_fov_rad/2              ,v_fov_rad/2             ,-h_fov_rad/2+h_cell_size , h_fov_rad/2+h_cell_size , dtype)))
    )


def to_point_cloud(selection, distances, directions, dtype = np.float32):

    sn = selection.shape[0]

    points = np.empty((sn, 3), dtype)

    points.resize((sn, 3))
    points[:] = directions[selection] * distances.reshape(sn, 1)

    return points

def generate_quads_indices(n, dtype = np.uint32):

    iota = np.arange(n, dtype = dtype)
    iota_2n = iota+2*n
    return np.stack((iota, iota+n, iota_2n, iota, iota_2n, iota + 3*n), axis=1)

def triangle_to_echo_index(triangle):
    return triangle[0] # must be in accordance with generate_quads_indices()

def quad_stack(scalars):
    return np.concatenate((scalars,scalars,scalars,scalars))

def to_quad_cloud(selection, distances, amplitudes, quad_directions, v, h,  dtype = np.float32):

    sn = selection.shape[0]

    n = v * h

    points = np.empty((sn*4, 3), dtype)

    quad_amplitudes = np.empty((sn*4, 1), dtype)

    # four points per quad, 1 different direction per point, same distance for each
    points[0:sn]      = quad_directions[selection    ] * distances[:, np.newaxis]
    points[sn:2*sn]   = quad_directions[selection+n  ] * distances[:, np.newaxis]
    points[2*sn:3*sn] = quad_directions[selection+2*n] * distances[:, np.newaxis]
    points[3*sn:]     = quad_directions[selection+3*n] * distances[:, np.newaxis]

    # same amplitude for each four points
    quad_amplitudes[:] = quad_stack(amplitudes)[:, np.newaxis]
    # a quad is formed with 2 triangles
    quad_indices = generate_quads_indices(sn, np.uint32)

    return points, quad_amplitudes, quad_indices.flatten()

def convert_echo_package(old, specs = {"v" : None, "h" : None, "v_fov" : None, "h_fov" : None}):
    return to_echo_package(old['indices']
                           , distances = old['data'][:,1]
                           , amplitudes = old['data'][:,2]
                           , timestamps = old['data'][:,0].astype('u2')
                           , flags = old['flags'].astype('u2')
                           , timestamp = old['timestamp'] if 'timestamp' in old else old['data'][0,0]
                    , specs = specs
                    , distance_scale = 1.0, amplitude_scale = 1.0, led_power = 1.0)

def to_echo_package(indices, distances, amplitudes, timestamps = None, flags = None, timestamp = 0
                    , specs = {"v" : None, "h" : None, "v_fov" : None, "h_fov" : None}
                    , distance_scale = 1.0, amplitude_scale = 1.0, led_power = 1.0):
    '''
        This format MUST remain in synch with the one in LeddarPyDevice::PackageEchoes()
    '''
    package = specs.copy()

    package.update({"timestamp": timestamp
                   , "distance_scale": distance_scale
                   , "amplitude_scale": amplitude_scale
                   , "led_power": led_power})

    assert(indices.size == distances.size == amplitudes.size)

    if timestamps is not None:
        assert(indices.size == timestamps.size)
    if flags is not None:
        assert(indices.size == flags.size)


    package['data'] = np.empty(indices.size, dtype = np.dtype([('indices', 'u4')
                                                       , ('distances', 'f4')
                                                       , ('amplitudes', 'f4')
                                                       , ('timestamps', 'u2')
                                                       , ('flags', 'u2')]));

    package['data']["indices"] = indices
    package['data']["distances"] = distances
    package['data']["amplitudes"] = amplitudes
    package['data']["timestamps"] = 0 if timestamps is None else timestamps
    package['data']["flags"] = 1 if flags is None else flags

    return package

