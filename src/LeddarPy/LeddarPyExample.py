import leddar
dev = leddar.Device()

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

echoes = dev.get_echoes()

v = int(dev.get_property_value(leddar.property_ids["ID_VERTICAL_CHANNEL_NBR"]))
h = int(dev.get_property_value(leddar.property_ids["ID_HORIZONTAL_CHANNEL_NBR"]))
h_fov = float(dev.get_property_value(leddar.property_ids["ID_HFOV"]))

print("resolution: {0} x {1}, fov: {2}".format(v,h, h_fov))

if echoes :
    data = echoes["data"]
    #To avoid display of too much lines
    increment = 1
    if len(data) > 100 :
        increment = 100

    print("Count:" + str(len(data)))
    print("timestamp:" + str(echoes['timestamp']))

    print("Channel - Distance - Amplitude - Flag")
    for i in range(0, len(data), increment):
        print(str(data[i]["indices"]) + " - " + str(data[i]["distances"]) + " - " + str(data[i]["amplitudes"]) + " - " + str(data[i]["flags"]))

dev.disconnect()
del dev