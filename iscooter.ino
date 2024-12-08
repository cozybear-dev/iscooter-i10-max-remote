#include <M5Atom.h>
#include <BLEDevice.h>
// #include <BLEUtils.h>
// #include <BLE2902.h>
#include <Arduino.h>
#include <esp_sleep.h>

// Target BLE device MAC address
#define TARGET_DEVICE_ADDRESS "11:22:38:03:01:fa"

// Target characteristic UUID
#define COMMAND_CHARACTERISTIC_UUID "6d581e71-15c6-11ec-82a8-0002a5d5c51b"

// Response characteristic UUID (where the notification comes)
#define RESPONSE_CHARACTERISTIC_UUID "6d581e74-15c6-11ec-82a8-0002a5d5c51b"

// Target service UUID
#define TARGET_SERVICE_UUID "6d581e70-15c6-11ec-82a8-0002a5d5c51b"

// Global variables
bool deviceFound = false;
BLEAdvertisedDevice* targetDevice = nullptr;
BLERemoteCharacteristic* pCommandCharacteristic = nullptr;
String finalauthresponsehex = "";
bool lockOn = false;
unsigned long startMillis = 0;

char inv[] = {0x9E, 0x71, 0x6E, 0x81, 0xE1, 0xA7, 0x1C, 0xB5, 0x24, 0x88, 0x49, 0x53, 0xB2, 0x27, 0x5B, 0xBD};
char key[] = {0x98, 0x83, 0x22, 0x2C, 0xD5, 0xB2, 0x57, 0x13, 0x3F, 0x3A, 0xC6, 0x75, 0xA7, 0x13, 0x2B, 0x44};
char key_tmp[16];
char bonding_hash[4];
char bonding_nonce[6];

char s_box[] = {'c', '|', 'w', '{', 242, 'k', 'o', 197, '0', 1, 'g', '+', 254, 215, 171, 'v',
                202, 130, 201, '}', 250, 'Y', 'G', 240, 173, 212, 162, 175, 156, 164, 'r', 192,
                183, 253, 147, '&', '6', '?', 247, 204, '4', 165, 229, 241, 'q', 216, '1', 21,
                4, 199, '#', 195, 24, 150, 5, 154, 7, 18, 128, 226, 235, '\'', 178, 'u', 9,
                131, ',', 26, 27, 'n', 'Z', 160, 'R', ';', 214, 179, ')', 227, '/', 132, 'S',
                209, 0, 237, ' ', 252, 177, '[', 'j', 203, 190, '9', 'J', 'L', 'X', 207, 208,
                239, 170, 251, 'C', 'M', '3', 133, 'E', 249, 2, 127, 'P', '<', 159, 168, 'Q',
                163, '@', 143, 146, 157, '8', 245, 188, 182, 218, '!', 16, 255, 243, 210, 205,
                12, 19, 236, '_', 151, 'D', 23, 196, 167, '~', '=', 'd', ']', 25, 's', '`',
                129, 'O', 220, '\"', '*', 144, 136, 'F', 238, 184, 20, 222, '^', 11, 219, 224,
                '2', ':', 10, 'I', 6, '$', '\\', 194, 211, 172, 'b', 145, 149, 228, 'y', 231,
                200, '7', 'm', 141, 213, 'N', 169, 'l', 'V', 244, 234, 'e', 'z', 174, 8, 186,
                'x', '%', '.', 28, 166, 180, 198, 232, 221, 't', 31, 'K', 189, 139, 138, 'p',
                '>', 181, 'f', 'H', 3, 246, 14, 'a', '5', 'W', 185, 134, 193, 29, 158, 225,
                248, 152, 17, 'i', 217, 142, 148, 155, 30, 135, 233, 206, 'U', '(', 223, 140,
                161, 137, 13, 191, 230, 'B', 'h', 'A', 153, '-', 15, 176, 'T', 187, 22};


uint8_t
    DisBuff[2 + 5 * 5 * 3];  // Used to store RGB color values.  用来存储RBG色值


void setBuff(uint8_t Rdata, uint8_t Gdata,
             uint8_t Bdata) {  // Set the colors of LED, and save the relevant
                               // data to DisBuff[].  设置RGB灯的颜色
    DisBuff[0] = 0x05;
    DisBuff[1] = 0x05;
    for (int i = 0; i < 25; i++) {
        DisBuff[2 + i * 3 + 0] = Rdata;
        DisBuff[2 + i * 3 + 1] = Gdata;
        DisBuff[2 + i * 3 + 2] = Bdata;
    }
}

void sub_bytes(char* arr, int len) {
  for (int i = 0; i < len; i++) {
    arr[i] = s_box[arr[i] & 0xFF];
  }
}

void bonding_key_generate() {
  char temp[4];
  memcpy(key_tmp, key, 16);
  sub_bytes(key_tmp, 16);
  memcpy(temp, key_tmp, 4);
  for (int i = 0; i < 12; i++) {
    key_tmp[i] = key_tmp[i + 4];
  }
  memcpy(key_tmp + 12, temp, 4);
}

void bonding_hash_generate() {
  char temp[32];
  memcpy(temp, bonding_nonce, 6);
  memcpy(temp + 6, bonding_nonce, 6);
  memcpy(temp + 12, bonding_nonce, 4);
  memcpy(temp + 16, temp + 1, 15);
  temp[31] = temp[0];

  bonding_key_generate();

  for (int i = 0; i < 16; i++) {
    temp[i] = (temp[i] ^ key_tmp[i]) ^ inv[i];
    temp[i + 16] = (key_tmp[i] ^ temp[i + 16]) ^ inv[i];
  }

  for (int i = 1; i < 16; i++) {
    temp[i] ^= temp[i - 1];
    temp[i + 16] ^= temp[i + 16 - 1];
  }

  for (int i = 0; i < 4; i++) {
    bonding_hash[i] = (temp[i] ^ temp[i + 4] ^ temp[i + 8] ^ (temp[i + 12] + temp[i + 16]) ^
                      temp[i + 20] ^ temp[i + 24] ^ temp[i + 28]) & 0xFF;
  }
}

String encryptionStringOfValue(String input) {
  if (input.length() < 12) {
    return "";
  }

  for (int i = 0; i < 6; i++) {
    bonding_nonce[i] = strtoul(input.substring(i * 2, i * 2 + 2).c_str(), NULL, 16);
  }

  bonding_hash_generate();

  String result = "";
  for (int i = 0; i < 4; i++) {
    if (bonding_hash[i] < 16) result += "0";
    result += String(bonding_hash[i], HEX);
  }

  Serial.println("--challenge: " + input + "\n--challenge response: " + result);
  return result;
}

String getChallenge() {
  String challenge = "";
  for (int i = 0; i < 6; i++) {
    challenge += String(random(0, 256), HEX);
  }

  Serial.println("--new challenge: " + challenge);
  return challenge;
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    // Log the device information
    Serial.print("Device found: ");
    Serial.print("Address: ");
    Serial.print(advertisedDevice.getAddress().toString().c_str());
    Serial.print(", Name: ");
    Serial.println(advertisedDevice.getName().c_str());

    // Check if this is the target device
    if (advertisedDevice.getAddress().toString() == TARGET_DEVICE_ADDRESS) {
      Serial.println(">>> Target device found!");
      targetDevice = new BLEAdvertisedDevice(advertisedDevice);
      deviceFound = true;
    }
  }
};

void discoverServicesAndCharacteristics(BLEClient* pClient) {
  Serial.println("Discovering services...");

  // Retrieve the list of services
  std::map<std::string, BLERemoteService*>* services = pClient->getServices();

  if (services->empty()) {
    Serial.println("No services found!");
    return;
  }

  for (auto& serviceEntry : *services) {
    BLERemoteService* service = serviceEntry.second;
    Serial.printf("Service found: UUID: %s\n", service->getUUID().toString().c_str());

    // Discover characteristics of this service
    std::map<std::string, BLERemoteCharacteristic*>* characteristics = service->getCharacteristics();
    if (characteristics->empty()) {
      Serial.println("  No characteristics found.");
      continue;
    }

    for (auto& charEntry : *characteristics) {
      BLERemoteCharacteristic* characteristic = charEntry.second;
      Serial.printf("  Characteristic found: UUID: %s, Properties: ", characteristic->getUUID().toString().c_str());

      if (characteristic->canRead()) Serial.print("Read ");
      if (characteristic->canWrite()) Serial.print("Write ");
      if (characteristic->canNotify()) Serial.print("Notify ");
      if (characteristic->canIndicate()) Serial.print("Indicate ");
      Serial.println();
    }
  }
}

void sendHexCommand(BLERemoteCharacteristic* pCommandCharacteristic, String command) {
  int len = command.length() / 2;
  byte commandBytes[len];

  for (int i = 0; i < len; i++) {
    commandBytes[i] = strtol(command.substring(i * 2, i * 2 + 2).c_str(), NULL, 16);
  }

  // Print the byte array in hex format for debugging
  Serial.print("Command bytes: ");
  for (int i = 0; i < len; i++) {
    if (commandBytes[i] < 0x10) {
      Serial.print("0");  // Add leading zero for single digit hex values
    }
    Serial.print(commandBytes[i], HEX);
    Serial.print(" ");
  }
  Serial.println();  // Print a new line after printing all the bytes


  if (pCommandCharacteristic->canWrite()) {
    Serial.print("Sending command: ");
    Serial.println(command);
    pCommandCharacteristic->writeValue(commandBytes, len, false);
  } else {
    Serial.println("Command not able to be written.");
  }
  delay(200);
}

String hexToUtf8(String hexString) {
  // Check if the length of the hex string is even
  if (hexString.length() % 2 != 0) {
    Serial.println("Error: Hex string must have an even length.");
    return "";
  }

  // Create a buffer to store the decoded bytes
  int len = hexString.length() / 2;  // Each byte is 2 hex characters
  char utf8Bytes[len + 1];          // +1 for null-terminator

  // Decode the hex string into bytes
  for (int i = 0; i < len; i++) {
    String byteString = hexString.substring(i * 2, i * 2 + 2);
    utf8Bytes[i] = (char) strtol(byteString.c_str(), NULL, 16);
  }

  // Null-terminate the string
  utf8Bytes[len] = '\0';

  // Return the decoded UTF-8 string
  return String(utf8Bytes);
}

String utf8ToHex(String utf8String) {
  String hexString = "";

  // Iterate through each character of the UTF-8 string
  for (int i = 0; i < utf8String.length(); i++) {
    // Get the ASCII/UTF-8 value of the character
    char c = utf8String.charAt(i);
    int value = (int)c;

    // Convert the value to a two-character hex string
    if (value < 16) {
      hexString += "0";  // Add leading zero for single-digit hex values
    }
    hexString += String(value, HEX);
  }

  return hexString;
}

void characteristicNotifyCallback(BLERemoteCharacteristic* pCharacteristic, uint8_t* data, size_t length, bool isNotify) {
  Serial.print("Notification received: ");
  // Create a String to hold the hex representation of the data
  String hexString = "";

  // Convert the data to a hex string
  for (size_t i = 0; i < length; i++) {
    if (data[i] < 0x10) {
      hexString += "0";  // Add leading zero for single-digit hex values
    }
    hexString += String(data[i], HEX);  // Append the hex value to the string
  }

  // Print the resulting hex string
  Serial.println(hexString);

  String PM = "2b504d3e"; //+PM>

  if (hexString.startsWith(PM)) { 
    Serial.print("Auth code received: ");
    String authcodehex = hexString.substring(PM.length());
    String authcode = hexToUtf8(authcodehex);
    Serial.println(authcode);
    Serial.println("Generating and sending verified response...");
    String secret = encryptionStringOfValue(authcode);
    String secrethex = utf8ToHex(secret);
    String responsehex = "2b504d3c"; //+PM<
    responsehex += secrethex;
    finalauthresponsehex = responsehex;
  }

  // elif (hexString == "2b4c4f434b3d31") { // lock is on
  //   Serial.println("Lock is on");
  //   lockOn = true;
  // } 

  // elif (hexString == "2b4c4f434b3d30") { // lock is off
  //   Serial.println("Lock is off");
  //   lockOn = false;
  // } 

  // if (hexstring != "2b4d4f44453d34") { // mode is not 4

  // } 
}


void setup() {
  // Initialize Serial Monitor
  M5.begin(true, false,
              true);
  delay(50);
  setBuff(0x00, 0x40, 0x00);
  M5.dis.displaybuff(
      DisBuff); 
  Serial.begin(115200);
  Serial.println("Starting BLE client...");
  startMillis = millis();

  // Initialize BLE
  BLEDevice::init("");

  // Start scanning for the target device
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
  pBLEScan->setActiveScan(true);

  Serial.println("Scanning for BLE devices...");
  while (!deviceFound) {
    Serial.println(">>> Scanning for 1 second...");
    pBLEScan->start(1, false); // Scan for 1 second

    if (millis() >= 30000) {
      Serial.println("Entering deep sleep now to trigger automatic shutdown...");
      esp_deep_sleep_start(); // Enter deep sleep
    }
  }
  
  // Stop scanning
  BLEDevice::getScan()->stop();
  
  // Connect to the target device
  Serial.println("Connecting to the target device...");
  BLEClient* pClient = BLEDevice::createClient();
  pClient->connect(targetDevice); // Connect to the device

  if (pClient->isConnected()) {
    Serial.println("Connected to the target device.");
    
    // Discover services and characteristics
    // discoverServicesAndCharacteristics(pClient);

    // Locate the specific target characteristic
    BLERemoteService* pService = pClient->getService(TARGET_SERVICE_UUID);
    delay(100);
    if (pService != nullptr) {
      pCommandCharacteristic = pService->getCharacteristic(COMMAND_CHARACTERISTIC_UUID);
      if (pCommandCharacteristic == nullptr) {
        Serial.println("ERROR: Command characteristic not found.");
        return;
      } else {
        Serial.println("Command characteristic found.");
      }
      delay(100);
      BLERemoteCharacteristic* pResponseCharacteristic = pService->getCharacteristic(RESPONSE_CHARACTERISTIC_UUID);
      if (pResponseCharacteristic == nullptr) {
        Serial.println("ERROR: Response characteristic not found.");
        return;
      } else {
        Serial.println("Response characteristic found.");
      }
      delay(100);
      // Enable notifications for the characteristic
      if (pResponseCharacteristic->canNotify()) {
        pResponseCharacteristic->registerForNotify(characteristicNotifyCallback);
        Serial.println("Notification registered.");
      } else {
        Serial.println("Response does not support notifications.");
      }
      delay(100);
      // sendHexCommand(pCommandCharacteristic, "2b5645523f"); // version
      sendHexCommand(pCommandCharacteristic, "2b504d3f"); // get code for auth
      while (finalauthresponsehex == "") {
        Serial.println("Waiting on auth completion...");
        delay(100);
      }
      sendHexCommand(pCommandCharacteristic, finalauthresponsehex);

      Serial.println("Sending counter challenge...");
      String PA = "2b50413c"; //+PA<
      String challenge = getChallenge();
      String challengehex = utf8ToHex(challenge);
      PA += challengehex;
      sendHexCommand(pCommandCharacteristic, PA);

      sendHexCommand(pCommandCharacteristic, "434f44453d303030303030");//code
      // sendHexCommand(pCommandCharacteristic, "4745544445564944"); //getdevid
      // sendHexCommand(pCommandCharacteristic, "2b4d4f44453d3f");// which mode?
      // sendHexCommand(pCommandCharacteristic, "2b4c4f434b3d3f");// lock on?

      // Serial.println("Locking scooter.");
      // sendHexCommand(pCommandCharacteristic, "aa03040488010020bb");

      Serial.println("Unlocking scooter.");
      sendHexCommand(pCommandCharacteristic, "aa03040488000021bb");

      Serial.println("Set to sport mode.");
      sendHexCommand(pCommandCharacteristic, "aa03040188030027bb");

      setBuff(0x40, 0x00, 0x00);
      delay(50);
      M5.dis.displaybuff(DisBuff); 

      // sendHexCommand(pCommandCharacteristic, "aa03040288010026bb"); //turn light on
    } else {
      Serial.println("ERROR: Target service not found.");
    }
    
    // Disconnect after communication
    // pClient->disconnect();
    // Serial.println("Disconnected from the target device.");
  } else {
    Serial.println("ERROR: Failed to connect to the target device.");
  }

  // // Cleanup
  // delete targetDevice;
  // BLEDevice::deinit();
}

uint8_t FSM = 0;

void loop() {
    if (M5.Btn
            .wasPressed()) {  // Check if the key is pressed. 检测按键是否被按下
        switch (FSM) {
            case 0:
                Serial.println("Locking scooter.");
                sendHexCommand(pCommandCharacteristic, "aa03040488010020bb");
                setBuff(0x00, 0x40, 0x00);
                break;
            case 1:
                Serial.println("Unlocking scooter.");
                sendHexCommand(pCommandCharacteristic, "aa03040488000021bb");
                setBuff(0x40, 0x00, 0x00);
                break;
            default:
                break;
        }
        M5.dis.displaybuff(DisBuff);

        FSM++;
        if (FSM >= 2) {
            FSM = 0;
        }
    }
    delay(50);
    M5.update();
    if (millis() >= 30000) {
      Serial.println("Entering deep sleep now to trigger automatic shutdown...");
      esp_deep_sleep_start(); // Enter deep sleep
    }
}
