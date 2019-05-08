import leddar
import time

leddar.enable_debug_trace(True)

#Callback functions for the data thread
def echoes_callback(echoes):
    data = echoes["data"]
    indices = echoes["indices"]
    flags = echoes["flags"]
    #To avoid display of too much lines
    increment = 1
    if len(data) > 100 : 
        increment = 100

    print("Count:" + str(len(data)))

    print("Channel - Timestamp - Distance - Amplitude - Flag")
    for i in range(0, len(data), increment):
        print(str(indices[i]) + " - " + str(data[i, 0]) + " - " + str(data[i, 1]) + " - " + str(data[i, 2]) + " - " + str(flags[i]) )

def raw_traces_callback(traces):
    print("Timestamp: " + str(traces["timestamps"][0]))
    print("Traces " + str(traces["start_index"]) + " to " + str(traces["start_index"] + len(traces["data"])))
    print("Tracecount: " + str(len(traces["data"])))
    print("TraceLength: " + str(traces["data"].shape[1]))

def states_callback(states):
    print("Timestamp: " + str(states["timestamp"]))
    print("CPULoad " + str(states["cpu_load"]) + "%")
    print("Temperature " + str(states["system_temp"]) + " C")

leddar.enable_debug_trace(True)

#Create device
dev = leddar.Device()

#Connect to the device
sensorlist = leddar.get_devices("Usb")
#sensorlist = leddar.get_devices("Ethernet")
dev.connect(sensorlist[0], leddar.device_types["Usb"])

#Get properties value
print("ID_BASE_POINT_COUNT = " + dev.get_property_value(dev.properties_id["ID_BASE_POINT_COUNT"]))
print("ID_LED_INTENSITY = " + dev.get_property_value(dev.properties_id["ID_LED_INTENSITY"]))
print("ID_LED_AUTO_PWR_ENABLE = " + dev.get_property_value(dev.properties_id["ID_LED_AUTO_PWR_ENABLE"]))
dev.setPropertyValue(dev.properties_id["ID_LED_AUTO_PWR_ENABLE"],"true")
print("ID_LED_AUTO_PWR_ENABLE = " + dev.get_property_value(dev.properties_id["ID_LED_AUTO_PWR_ENABLE"]))

#Property available values
values = dev.get_property_available_values(dev.properties_id["ID_LED_INTENSITY"])
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