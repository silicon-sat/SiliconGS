#include <RadioLib.h>
#include <SPI.h>

// Create a custom SPI instance
SPIClass customSPI(VSPI);  // VSPI is commonly used with ESP32
// Custom SPI pin configuration for ESP32
#define LORA_MOSI 27
#define LORA_MISO 19
#define LORA_SCK 5

// LoRa module pin configuration for ESP32
#define LORA_NSS 18   
#define LORA_RST 14   
#define LORA_DIO0 26  
#define LORA_DIO1 32  

// Initialize LoRa module with custom SPI
SX1278 radio = new Module(LORA_NSS, LORA_DIO0, LORA_RST, LORA_DIO1, customSPI);

// Flag to indicate that a packet was received
volatile bool receivedFlag = false;

// Function to be called when a complete packet is received
#if defined(ESP8266) || defined(ESP32)
  ICACHE_RAM_ATTR
#endif
void setFlag() {
  // Set the flag when a packet is received
  receivedFlag = true;
}

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);

  // Initialize custom SPI pins
  customSPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);

  // Initialize the LoRa module with specified parameters for SX1278
  Serial.print(F("[SX1278] Initializing..."));
  
  //radio.begin(frequency, bandwidth, spreadingFactor, codingRate, syncWord, power, preambleLength, gain);
  int state = radio.begin(436.5, 250.0, 10, 5, 18, 5, 8, 0); // Sync word set to 18
  
  // Debug: Print status of initialization
  Serial.print(F("[SX1278] Begin state: "));
  Serial.println(state);

  // Check the initialization status
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true) { delay(10); };  // Halt the program if initialization fails
  }

  // Set the current limit to 120 mA as per the conditions
  radio.setCurrentLimit(120);
  
  // Enable CRC
  radio.setCRC(true);

  // Set to explicit header mode
  radio.explicitHeader();

  // Set the function that will be called when a new packet is received
  radio.setPacketReceivedAction(setFlag);

  // Start listening for LoRa packets
  Serial.print(F("[SX1278] Starting to listen ... "));
  state = radio.startReceive();
  
  // Debug: Print status of startReceive
  Serial.print(F("[SX1278] startReceive state: "));
  Serial.println(state);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("Listening..."));
  } else {
    Serial.print(F("Failed to start receive, code "));
    Serial.println(state);
    while (true) { delay(10); }
  }
}

void loop() {
  // Check if the flag is set
  if(receivedFlag) {
    // Reset flag
    receivedFlag = false;

    // Declare a string to hold the received data
    String receivedString;

    // Read received data into the string
    int state = radio.readData(receivedString);

    /*byte byteArr[8];
      int numBytes = radio.getPacketLength();
      int state = radio.readData(byteArr, numBytes);
     */
    // Debug: Print the receive state
    Serial.print(F("[SX1278] Receive state: "));
    Serial.println(state);

    if (state == RADIOLIB_ERR_NONE) {
      // Message received successfully
      Serial.println(F("[SX1278] Packet received!"));

      // Print the received string
      Serial.print(F("[SX1278] Data: "));
      Serial.println(receivedString);

      // Print RSSI (Received Signal Strength Indicator)
      Serial.print(F("[SX1278] RSSI: "));
      Serial.print(radio.getRSSI());
      Serial.println(F(" dBm"));

      // Print SNR (Signal-to-Noise Ratio)
      Serial.print(F("[SX1278] SNR: "));
      Serial.print(radio.getSNR());
      Serial.println(F(" dB"));

      // Print frequency error
      Serial.print(F("[SX1278] Frequency error: "));
      Serial.print(radio.getFrequencyError());
      Serial.println(F(" Hz"));

    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      // Packet was received, but is malformed
      Serial.println(F("[SX1278] CRC error!"));

    } else {
      // Some other error occurred
      Serial.print(F("[SX1278] Failed, code "));
      Serial.println(state);
    }
  }

  // Short delay before the next receive attempt (optional)
  delay(100);
}

