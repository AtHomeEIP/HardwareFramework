// For documentation of the API, see https://woodbox.gitlab.io/Framework/
#ifdef ARDUINO_AVR_UNO
# include <SoftwareSerial.h>
#endif
#include <AtHome.h>
#include <stdint.h>

#define SENSOR_INTERVAL (60000)
#define UPLOAD_INTERVAL (SENSOR_INTERVAL * 15)

using MyModule = AtHomeWiFiModule<int32_t, 15>; // Create an alias on a specialized template version of the module, to shorten next uses

MyModule *module = reinterpret_cast<MyModule *>(MyModule::getInstance()); // Create a module instance, able to buffer 15 values
CommonCathodeRGBLed led(9, 10, 11); // pin9 => PWM red, pin10 => PWM green, pin11 => PWM blue
#ifdef ARDUINO_AVR_UNO
SoftwareSerial espSerial(7, 8); // pin7 => soft RX, pin8 => soft TX
#else
HardwareSerial &espSerial = Serial1;
#endif
ESP8266WiFiCommunicator esp8266(2, 3); // pin2 => CH_ED and pin3 => RST of the ESP8266
Stream *streams[] = {&Serial, &esp8266, nullptr}; // Streams are the hardware UART (used for USB) and software UART (used to communicate with the ESP)
DummySensor<ATemperatureSensor, int32_t, 50000000> dummyTemperatureSensor; // Generate random values
#ifdef __AVR__
ArduinoEEPROM eeprom;
#endif
FakeRTC rtc;

void setup() {
    // put your setup code here, to run once:
    espSerial.begin(115200); // Actually, I don't think the atmega328 is able to do software serial at 115200 bauds, but that's the default baudrate of the esp and you need to be able to communicate with it to change it
    Serial.begin(9600); // Basic initialization of the hardware serial interface for communication with the host
    esp8266.setStreamToChipset(&espSerial); // Define the stream used to communicate with the esp => esp32 supports also using SPI and I2C with the AT firmware, not only UART
    module->setWiFiCommunicator(esp8266); // Set the AWiFiCommunicator instance used to communicate through a WiFi adapter
    module->setStreams(streams); // Set the streams used by the module to communicate. It can be as much as you want
    module->setSensor(&dummyTemperatureSensor); // Set the sensor of the module, which will be sampled to get the data to send
    module->setDisplay(&led); // Set the led used to display the color of the module, if it's good or bad
#ifdef __AVR__
    module->setStorage(&eeprom); // Set the storage used to store permanent data such as the type, serial and vendor of the module
#endif
    module->setSerial(42); // Set the serial of the module -> Always an array of 32 bytes
    module->setClock(&rtc); // Set the clock used to get the current time -> emulated using the millis() function
    module->setSensorExecutionInterval(SENSOR_INTERVAL); // Samples the sensor every 60 seconds
    module->setUploadDataExecutionInterval(UPLOAD_INTERVAL); // Send the data after at most 15 time the sensor was sampled, as we can't buffer more anyway
    module->setCommunicationExecutionInterval(1); // Listen for input data every 1 millisecond
    module->setup(); // Initialize the module
}

void loop() {
    // put your main code here, to run repeatedly:
    module->run(); // Execute one time frame of the module (ie., one execution of every tasks ready to execute)
}