#!/usr/bin/python3

import sys
import logging
import paho.mqtt.client as mqtt
import random
from base64 import b64encode

USER = "XXXXXXXXXXX"
PASSWORD = "NNSXS.XXXXXXXXXXX.XXXXXXXXXXX"
PUBLIC_TLS_ADDRESS = "eu1.cloud.thethings.network"
PUBLIC_TLS_ADDRESS_PORT = 8883
DEVICE_ID = "XXXXXXXXXXX"

# Meaning Quality of Service (QoS)
# QoS = 0 - at most once
# The client publishes the message, and there is no acknowledgement by the broker.
# QoS = 1 - at least once
# The broker sends an acknowledgement back to the client.
# The client will re-send until it gets the broker's acknowledgement.
# QoS = 2 - exactly once
# Both sender and receiver are sure that the message was sent exactly once, using a kind of handshake
QOS = 0

def stop(client):
    client.disconnect()
    print("\nExit")
    sys.exit(0)


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("\nConnected successfully to MQTT broker")
    else:
        print("\nFailed to connect, return code = " + str(rc))


# mid = message ID
# It is an integer that is a unique message identifier assigned by the client.
# If you use QoS levels 1 or 2 then the client loop will use the mid to identify messages that have not been sent.
def on_subscribe(client, userdata, mid, granted_qos):
    print("\nSubscribed with message id (mid) = " + str(mid) + " and QoS = " + str(granted_qos))


def on_disconnect(client, userdata, rc):
    print("\nDisconnected with result code = " + str(rc))


def on_log(client, userdata, level, buf):
    print("\nLog: " + buf)
    logging_level = client.LOGGING_LEVEL[level]
    logging.log(logging_level, buf)


# Generate client ID with pub prefix randomly
client_id = f'python-mqtt-{random.randint(0, 1000)}'

print("Create new mqtt client instance")
mqttc = mqtt.Client(client_id)

print("Assign callback functions")
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
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

if len(DEVICE_ID) != 0:
    topic = "v3/" + USER + "/devices/" + DEVICE_ID + "/down/push"

    print("Subscribe to topic " + topic + " with QoS = " + str(QOS))
    mqttc.subscribe(topic, QOS)

    print("Publishing message to topic " + topic + " with QoS = " + str(QOS))

    # 00 = YELLOW LED OFF, GREEN LED OFF
    # 01 = YELLOW LED ON, GREEN LED OFF
    # 02 = YELLOW LED OFF, GREEN LED ON
    # 03 = YELLOW LED ON, GREEN LED ON
    hexadecimal_payload = '0202'
    fport = 3

    # Convert hexadecimal payload to base64
    b64 = b64encode(bytes.fromhex(hexadecimal_payload)).decode()
    print('Convert hexadecimal_payload: ' + hexadecimal_payload + ' to base64: ' + b64)

    msg = '{"downlinks":[{"f_port":' + str(1) + ',"frm_payload":"' + b64 + '","priority": "NORMAL"}]}'
    result = mqttc.publish(topic, msg, QOS)

    # result: [0, 2]
    status = result[0]
    if status == 0:
        print("Send " + msg + " to topic " + topic)
    else:
        print("Failed to send message to topic " + topic)

else:
    print("Can not subscribe or publish to topic")
    stop(mqttc)