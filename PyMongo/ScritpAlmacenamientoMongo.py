#!/usr/bin/python3
import os
import sys
import logging
import paho.mqtt.client as mqtt
import json
import csv
import random
from datetime import datetime
from pymongo import MongoClient, collection


USER = "XXXXXXX"
PASSWORD = "NNSXS.XXXXXXX.XXXXXXX"
PUBLIC_TLS_ADDRESS = "eu1.cloud.thethings.network"
PUBLIC_TLS_ADDRESS_PORT = 8883
DEVICE_ID = "XXXXXXX"
ALL_DEVICES = True

QOS = 0

DEBUG = True


def get_value_from_json_object(obj, key):
    try:
        return obj[key]
    except KeyError:
        return '-'


def stop(client):
    client.disconnect()
    print("\nExit")
    sys.exit(0)
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("\nConnected successfully to MQTT broker")
    else:
        print("\nFailed to connect, return code = " + str(rc))


# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, message):
    print("\nMessage received on topic '" + message.topic + "' with QoS = " + str(message.qos))

    parsed_json = json.loads(message.payload)

    #if DEBUG:
        #print("Payload (Collapsed): " + str(message.payload))
        #print("Payload (Expanded): \n" + json.dumps(parsed_json, indent=4)) #Aquí
    save_to_database(parsed_json)
    #save_to_file(parsed_json)



def on_subscribe(client, userdata, mid, granted_qos):
    print("\nSubscribed with message id (mid) = " + str(mid) + " and QoS = " + str(granted_qos))


def on_disconnect(client, userdata, rc):
    print("\nDisconnected with result code = " + str(rc))


def on_log(client, userdata, level, buf):
    print("\nLog: " + buf)
    logging_level = client.LOGGING_LEVEL[level]
    logging.log(logging_level, buf)

def save_to_database(some_json):
    client = MongoClient('localhost')

    db = client['Test_Sensores']

    collection = db['Temperaturas']
    if 'uplink_message' in some_json:
        uplink_message = some_json["uplink_message"]
        f_port = get_value_from_json_object(uplink_message, "f_port")

        # check if f_port is found
        if f_port != '-':
            frm_payload = uplink_message["frm_payload"]
            # If decoded_payload is a json object or a string "-" it will be converted to string
            decoded_payload = str(get_value_from_json_object(uplink_message, "decoded_payload"))
            collection.insert_one(get_value_from_json_object(uplink_message, "decoded_payload")) #Funciona



# Generate client ID with pub prefix randomly
client_id = f'python-mqtt-{random.randint(0, 1000)}'




print("Create new mqtt client instance")
mqttc = mqtt.Client(client_id)

print("Assign callback functions")
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_message = on_message
mqttc.on_disconnect = on_disconnect
# mqttc.on_log = on_log  # Logging for debugging OK, waste

# Setup authentication from settings above
mqttc.username_pw_set(USER, PASSWORD)

# IMPORTANT - this enables the encryption of messages
#mqttc.tls_set()  # default certification authority of the system

mqttc.tls_set(ca_certs="/mqtt-ca.pem") # Use this if you get security errors
# It loads the TTI security certificate. Download it from their website from this page: 
# https://www.thethingsnetwork.org/docs/applications/mqtt/api/index.html
# This is normally required if you are running the script on Windows

print("Connecting to broker: " + PUBLIC_TLS_ADDRESS + ":" + str(PUBLIC_TLS_ADDRESS_PORT))
mqttc.connect(PUBLIC_TLS_ADDRESS, PUBLIC_TLS_ADDRESS_PORT, 60)


if ALL_DEVICES:
    print("Subscribe to all topics (#) with QoS = " + str(QOS))
    mqttc.subscribe("#", QOS)
elif len(DEVICE_ID) != 0:
    topic = "v3/" + USER + "/devices/" + DEVICE_ID + "/up"
    print("Subscribe to topic " + topic + " with QoS = " + str(QOS))
    mqttc.subscribe(topic, QOS)
else:
    print("Can not subscribe to any topic")
    stop(mqttc)


print("And run forever")
try:
    run = True
    while run:
        mqttc.loop(10)  # seconds timeout / blocking time
        print(".", end="", flush=True)  # feedback to the user that something is actually happening
except KeyboardInterrupt:
    stop(mqttc)



#print(db.collection_names())
#print(client.list_database_names())
#print(collection.count_documents({}))

#for cursor in collection.find({}):
    #print(cursor)

