import leddar
dev = leddar.Device()

##Use one of this connection method

sensor_list = leddar.get_devices("Usb")
dev.connect(sensor_list[0], leddar.device_types["Usb"])

#dev.connect("\\\.\\COM11", leddar.device_types["Serial"] )


echoes = dev.get_echoes()

v = int(dev.get_property_value(leddar.property_ids["ID_VERTICAL_CHANNEL_NBR"]))
h = int(dev.get_property_value(leddar.property_ids["ID_HORIZONTAL_CHANNEL_NBR"]))
h_fov = float(dev.get_property_value(leddar.property_ids["ID_HFOV"]))

print("resolution: {0} x {1}, fov: {2}".format(v,h, h_fov))

if echoes :
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

dev.disconnect()
del dev