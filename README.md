# mqtt-led-strip-control
Control RGB LED strips with microcontroller via Android application using MQTT protocol

# mosquitto test commands
receiver: mosquitto_sub -h 10.0.0.x -p 12345 -t topicName

sender: mosquitto_pub -h 10.0.0.x -p 12345 -t topicName -m "Hello world"
