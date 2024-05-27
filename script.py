import os, time, sys
#from influxdb_client_3 import InfluxDBClient3, Point
import logging as log
from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_air_quality import BrickletAirQuality
import InfluxDBWrite

token = "x8hac3wi6m2Dx85QLV85eGF4FERCYagBNeROnkj_OidTLtiypStbk78z7Cv8isXXHi0SrfWinOZrbGzElOxiVA=="
org = "DataX"
host = "https://us-east-1-1.aws.cloud2.influxdata.com"


database="testCode"


class WeatherStation:
    HOST = "localhost"
    PORT = 4223
    
    def __init__(self):
        self.ipcon = IPConnection()
        self.air_quality = None

        #Make IP connection.
        while True:
            try:
                self.ipcon.connect(WeatherStation.HOST, WeatherStation.PORT)
                break
            except Exception as e:
                log.error('Connection Error: ' + str(e))
                time.sleep(1) 

        self.ipcon.register_callback(IPConnection.CALLBACK_ENUMERATE, self.cb_enumerate)
        self.ipcon.register_callback(IPConnection.CALLBACK_CONNECTED, self.cb_connected)
        self.ipcon.enumerate()

    def cb_all_values(self, iaq_index, iaq_index_accuracy, temperature, humidity, air_pressure):
        InfluxDBWrite.write('test2',token,url,bucket,iaq_index,temperature,humidity,air_pressure)

    #Enumerate device.
    def cb_enumerate(self, uid, connected_uid, position, hardware_version,
                     firmware_version, device_identifier, enumeration_type):
        if device_identifier == BrickletAirQuality.DEVICE_IDENTIFIER:
            self.air_quality = BrickletAirQuality(uid, self.ipcon)
            self.air_quality.set_all_values_callback_configuration(1000, False)
            self.air_quality.register_callback(BrickletAirQuality.CALLBACK_ALL_VALUES,
                                               self.cb_all_values)
            log.info('Air Quality initialized')

    def cb_connected(self, connected_reason):
        if connected_reason == IPConnection.CONNECT_REASON_AUTO_RECONNECT:
            log.info('Auto Reconnect')
            self.ipcon.enumerate()
if __name__ == '__main__':
    url = host
    bucket = database 
    log.basicConfig(level=log.INFO)
    log.info('Weather Station: Start')

    weather_station = WeatherStation()

    if sys.version_info < (3, 0):
        input = raw_input  # Compatibility for Python 2.x
    input('Press key to exit\n')

    weather_station.ipcon.disconnect()
    log.info('Weather Station: End')