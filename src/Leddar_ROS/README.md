# leddar_ros

ROS package exposing LeddarTech sensors based on the "leddar" Python module. This package has been tested with ros-melodic.

## Deployment

Build the LC4 SDK according to instructions.

Install ```leddar``` and ```leddar_utils``` modules:

```bash
cd [...]LC4/main/src/LeddarPy
python setup.py install --user
cd [...]LC4/main/src/Py_leddar_utils
python setup.py install --user
```

Follow ROS [installation instructions](http://wiki.ros.org/Installation/Ubuntu) and [catkin workspace instructions](http://wiki.ros.org/catkin/Tutorials/create_a_workspace). Short story:

``` bash
sudo sh -c 'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'
sudo apt-key adv --keyserver hkp://ha.pool.sks-keyservers.net:80 --recv-key 421C365BD9FF1F717815A3895523BAEEB01FA116
sudo apt update
sudo apt install ros-melodic-desktop-full
sudo apt install ros-melodic-roslaunch
source /opt/ros/melodic/setup.bash
mkdir -p ~/catkin_ws/src
cd ~/catkin_ws/
catkin_make
```

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

```bash
cd ~/catkin_ws/
source /opt/ros/melodic/setup.bash
source ./devel/setup.bash
roslaunch leddar_ros example.launch param1:=AK47035 device_type:=M16
```
