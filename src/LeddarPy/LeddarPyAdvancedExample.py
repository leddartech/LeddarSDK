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
sensorlist = leddar.get_devices("Usb")
dev.connect(sensorlist[0]['name'], leddar.device_types["Usb"])

#Get properties value
print("ID_BASE_POINT_COUNT = " + dev.get_property_value(leddar.property_ids["ID_BASE_POINT_COUNT"]))
print("ID_LED_INTENSITY = " + dev.get_property_value(leddar.property_ids["ID_LED_INTENSITY"]))

#Property available values
values = dev.get_property_available_values(leddar.property_ids["ID_LED_INTENSITY"])
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