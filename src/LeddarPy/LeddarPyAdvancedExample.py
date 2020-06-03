import leddar
import time

leddar.enable_debug_trace(True)

#Callback functions for the data thread
def echoes_callback(echoes):
    data = echoes["data"]
    #To avoid display of too much lines
    increment = 1
    if len(data) > 100 :
        increment = 100

    print("Count:" + str(len(data)))
    print("timestamp:" + str(echoes['timestamp']))

    print("Channel - Timestamp - Distance - Amplitude - Flag")
    for i in range(0, len(data), increment):
        print(str(data[i]["indices"]) + " - " + str(data[i]["distances"]) + " - " + str(data[i]["amplitudes"]) + " - " + str(data[i]["flags"]))

def states_callback(states):
    print("timestamp: " + str(states["timestamp"]))
    print("cpu_load " + str(states["cpu_load"]) + "%")
    print("system_temp " + str(states["system_temp"]) + " C")

#Create device
dev = leddar.Device()

#Connect to the device
##Use one of this connection method
#Ethernet
sensor_list = leddar.get_devices("Ethernet")
print(leddar.device_types["Ethernet"])
dev.connect('192.168.0.2', leddar.device_types["Ethernet"], 48630)
connection = dev.connect('192.168.0.2', leddar.device_types["Ethernet"], 48630)

#M16 Usb
# sensor_list = leddar.get_devices("Usb")
# dev.connect(sensor_list[0]['name'], leddar.device_types["Usb"])

##For any sensors with modbus serial communication (LeddarOne, Vu8 or M16)
# sensor_list = leddar.get_devices("Serial")
# dev.connect(sensor_list[0]['name'], leddar.device_types["Serial"])

##For Vu8/M16 sensors with CAN BUS communication
# dev.connect(Baud rate (kbps), Type of sensor, Tx ID (optionnal), Rx ID (optionnal))
# dev.connect('1000', leddar.device_types["M16Komodo"])
# dev.connect('1000', leddar.device_types["Vu8Komodo"])

#Get properties value
print("ID_DEVICE_NAME = " + dev.get_property_value(leddar.property_ids["ID_DEVICE_NAME"]))
print("ID_SERIAL_NUMBER = " + dev.get_property_value(leddar.property_ids["ID_SERIAL_NUMBER"]))

#Property available values
values = dev.get_property_available_values(leddar.property_ids["ID_DISTANCE_SCALE"])
print(values["type"])
print(values["data"])

#Set callback method
dev.set_callback_state(states_callback)
dev.set_callback_echo(echoes_callback)
dev.set_data_mask(leddar.data_masks["DM_STATES"] | leddar.data_masks["DM_ECHOES"])

#Optionnal : set the delay between two request to the sensor
dev.set_data_thread_delay(10000)

dev.start_data_thread()

time.sleep( 100 )

dev.stop_data_thread()

time.sleep( 1 )
dev.disconnect()
del dev