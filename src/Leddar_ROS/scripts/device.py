#!/usr/bin/env python

import leddar
from leddar_utils import clouds
import rospy
import math
import numpy as np
import time

from rospy.numpy_msg import numpy_msg
import sys
from sensor_msgs.msg import Image
from sensor_msgs.point_cloud2 import create_cloud
from std_msgs.msg import Header, ColorRGBA
from sensor_msgs.msg import PointCloud2, Temperature
from visualization_msgs.msg import Marker
from geometry_msgs.msg import Point, Vector3
from leddar_ros.msg import Specs
import ros_numpy

TIMESTAMPS, DISTANCE, AMPLITUDE = range(3)


if __name__ == '__main__':
    
    rospy.init_node('leddar_ros', disable_signals=True)

    dev = leddar.Device()

    frame_id = rospy.get_param('~frame_id', 'map')
    param1 = rospy.get_param('~param1')
    device_type = rospy.get_param('~device_type',"not specified")
    param3 = rospy.get_param('~param3', 0)
    param4 = rospy.get_param('~param4', 0)
    param1 = str(param1) # Be sure its a string (for CANbus)
    param3 = int(param3)
    param4 = int(param4)

    dev_type = 0
    if(device_type != "not specified"):
        dev_type = leddar.device_types[device_type]

    if not dev.connect(param1, dev_type, param3, param4):
        err_msg = 'Error connecting to device type {0} with connection info {1}/{2}/{3}.'.format(device_type, param1, str(param3), str(param4))
        rospy.logerr(err_msg)
        raise RuntimeError(err_msg)
    

    specs = Specs()
    specs.v, specs.h, = [int(dev.get_property_value(x)) for x in ["ID_VERTICAL_CHANNEL_NBR", "ID_HORIZONTAL_CHANNEL_NBR"]]
    specs.v_fov, specs.h_fov = [float(dev.get_property_value(x)) for x in ["ID_VFOV", "ID_HFOV"]]

    directions = clouds.directions(clouds.angles(specs.v, specs.h, specs.v_fov, specs.h_fov, np.float32))
    quad_directions = clouds.quad_directions(specs.v, specs.h, specs.v_fov, specs.h_fov, np.float32)

    pub_specs = rospy.Publisher('specs', Specs, queue_size=100)
    pub_specs.publish(specs)
    pub_cloud = rospy.Publisher('scan_cloud', PointCloud2, queue_size=100)
    pub_raw = rospy.Publisher('scan_raw', PointCloud2, queue_size=100)
    frame_id = rospy.get_param('~frame_id', 'map')"
    
    def echoes_callback(echo):
        echo['data'] = echo['data'][np.bitwise_and(echo['data']['flags'], 0x01).astype(np.bool)] #keep valid echoes only
        indices, flags, distances, amplitudes, x, y, z = [echo['data'][x] for x in ['indices', 'flags', 'distances', 'amplitudes', 'x', 'y', 'z']]
        stamp = rospy.Time.now()
        #rospy.loginfo('pixell: echoes callback')

        if pub_raw.get_num_connections() > 0:
            pub_raw.publish(ros_numpy.msgify(PointCloud2, echo['data'], stamp, frame_id))

        if pub_cloud.get_num_connections() > 0:
            struct_cloud = np.empty(indices.size, dtype=[
              ('x', np.float32),
              ('y', np.float32),
              ('z', np.float32),
              ('intensity', np.float32)
            ])

            struct_cloud['x'] = x
            struct_cloud['y'] = y
            struct_cloud['z'] = z

            struct_cloud['intensity'] = amplitudes
            pub_cloud.publish(ros_numpy.msgify(PointCloud2, struct_cloud, stamp, frame_id))


    dev.set_callback_echo(echoes_callback)
    dev.set_data_thread_delay(1000)
    dev.start_data_thread()
    rospy.spin()
