# leddar_ros

ROS package exposing LeddarTech sensors based on the "leddar" Python module. This package has been tested with ros-melodic.

## Deployment

Build the SDK according to instructions.

Follow ROS [installation instructions](http://wiki.ros.org/Installation/Ubuntu) and [catkin workspace instructions](http://wiki.ros.org/catkin/Tutorials/create_a_workspace).

Copy Leddar_ROS as ```leddar_ros``` and clone ```ros_numpy``` in your catkin workspace src folder in your catkin workspace src folder (e.g. ```~/catkin_ws/src```) and source your ```devel/setup.bash``` :

``` bash
cd ~/catkin_ws/src
cp -r [...]src/Leddar_ROS leddar_ros
chmod +x leddar_ros/scripts/device.py
git clone https://github.com/eric-wieser/ros_numpy.git
cd ~/catkin_ws/
source /opt/ros/melodic/setup.bash
source ./devel/setup.bash
catkin_make
```

## Launch

### Using a Launch File

You can use and modify the provided `example.launch`. It will start a leddar_ros node to be visualized with RViz:

The example below is for M16 using an USB connection. For other sensors, please refer to the example.launch file in the Leddar_ROS folder

```bash
cd ~/catkin_ws/
source /opt/ros/melodic/setup.bash
source ./devel/setup.bash
roslaunch leddar_ros example.launch param1:=AK47035 device_type:=M16
```
