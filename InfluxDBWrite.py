from influxdb_client_3 import InfluxDBClient3,write_client_options,WriteOptions,InfluxDBError,Point
import time
def write(name, token, url, bucket, iaq_index, temperature, humidity, air_pressure):
    # Create a single point with multiple fields for IAQ, temperature, humidity, and air pressure
    point = {
        "measurement": name,
        "tags":{"sensor":"aqi"},
        "fields": {
            "iaq_index": iaq_index,
            "temperature_c": temperature/100.0,
            "humidity_percent": humidity/100.0,
            "air_pressure_pa": air_pressure/100.0}
    }

    # point = {
    #     "measurement": name,
    #     "tags":{
    #         "iaq_index": iaq_index,
    #         "temperature_c": temperature/100.0,
    #         "humidity_percent": humidity/100.0,
    #         "air_pressure_pa": air_pressure/100.0},
    #     "fields":{"sensor":"Weather station"}

    # }
    
    # Write the point to InfluxDB
    org = "DataX"
    write_options = WriteOptions()
    wco = write_client_options(WriteOptions=write_options)
    client = InfluxDBClient3(host=url, token=token, database=bucket,org=org,write_client_options=wco)
    client.write(record=point,write_precision="s")
    
    time.sleep(1)



def main():
    url = "https://us-east-1-1.aws.cloud2.influxdata.com"
    bucket = "codeTest"
    token = 'UnrkgsQC56S3IfBw1uaCXaCkrOfNdXlpL-ZEDPUlsuFMU7FGj0kbfbNV1lI28VcQaadd1jVdEoKdLYHD_HlM8w=='
    
    # Example values - replace these with actual sensor readings
    iaq_index = 20  # Placeholder value for IAQ index
    temperature = 2250  # Placeholder value for temperature in °C
    humidity = 5000  # Placeholder value for humidity in %
    air_pressure = 101325  # Placeholder value for air pressure in hPa

    write("IAQ_Reading", token, url, bucket, iaq_index, temperature, humidity, air_pressure)

if __name__=='__main__':
    main()
