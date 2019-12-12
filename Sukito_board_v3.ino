
#include "CaptivePortal_Module.h"
#include "UHF_RFID_Driver.h"
#include "MEMORY_Module.h"
#include "AWS_Module.h"

/*
* define declaration
*/

//GLOBAL DEFAULT CONSTANT

#define RFID_BAURDRATE 115200
#define EPC_SIZE 255
#define DEFAULT_TTL_EPC 5
#define DEBOUNCING_TIME 200
#define DEFAULT_POWER 2700
#define LED_BLINK_TIMING 500
#define NUMBER_EPC 5

#define ADDR_CONFIG 0
#define ADDR_ID 10
#define ADDR_SSID 20
#define ADDR_PWD 30

//PIN DECLARATION
#define START_BUTTON_PIN 27
#define POWER_COMMAND_PIN 35
#define TTL_COMMAND_PIN 34

#define STATUS_LED_RED_PIN 33
#define STATUS_LED_GREEN_PIN 32
#define WIFI_LED_RED_PIN 26
#define WIFI_LED_GREEN_PIN 25

/*
* struct declaration
*/
enum device_state
{
  INIT,
  WIFI_STA_CONNECTED,
  WIFI_STA_ERROR,
  NFC_READ_STOP,
  NFC_READ_RUNNING,
  NFC_ERROR
};

//BUTTON STRUCT
struct Button
{
  const uint8_t Pin;
  uint32_t numberKeyPresses;
  bool pressed;
};

//LED STRUCT
struct Led
{
  const uint8_t Pin;
  int State;
};

/*
* Variable declaration
*/

Button start_button = {START_BUTTON_PIN, 0, false};
Led led_red_status = {STATUS_LED_RED_PIN, LOW};
Led led_green_status = {STATUS_LED_GREEN_PIN, LOW};
Led led_red_wifi = {WIFI_LED_RED_PIN, LOW};
Led led_green_wifi = {WIFI_LED_GREEN_PIN, LOW};

//GLOBAL VARIABLE
unsigned long ttl_previous_time = 0;
unsigned long interrupt_previous_time = 0;
int ttl_epc = DEFAULT_TTL_EPC * 1000;
int nb_epc_read = 0;
String list_EPC[NUMBER_EPC];
String actual_epc_read = "";
String last_epc_read = "";

connection_status_t err_code;
device_state state = INIT;

//Wifi parameter is set
bool WIFI_is_set = false;
bool AWS_is_connect = false;

int rssi = 0;
int freq = 0;


//RFID module object
RFID nano;

//Start button Handler interrupt
void IRAM_ATTR isr()
{
  unsigned long interrupt_current_time = millis();
  if (interrupt_current_time - interrupt_previous_time > DEBOUNCING_TIME)
  {
    start_button.numberKeyPresses += 1;
    start_button.pressed = true;
  }
  interrupt_previous_time = interrupt_current_time;
}

boolean RFID_init(long baudRate)
{
  //SERIAL 2 TX:17 RX:18
  uint8_t cpt_try = 0;
  nano.begin(Serial2); //Tell the library to communicate over software serial port

  //Test to see if we are already connected to a module
  //This would be the case if the Arduino has been reprogrammed and the module has stayed powered
  Serial2.begin(baudRate); //For this test, assume module is already at our desired baud rate
  while (!Serial2)
    ;
  //About 200ms from power on the module will send its firmware version at 115200. We need to ignore this.
  while (Serial2.available())
    Serial2.read();

  while (cpt_try < 3)
  {
    nano.getVersion();
    Serial.println(nano.msg[0]);
    if (nano.msg[0] == ERROR_WRONG_OPCODE_RESPONSE)
    {
      //This happens if the baud rate is correct but the module is doing a continuous read
      nano.stopReading();
      Serial.println(F("Module continuously reading. Asking it to stop..."));
      delay(1500);
    }
    else if (nano.msg[0] != ALL_GOOD)
    {
      Serial.println(F("Try to reset RFID speed"));
      //The module did not respond so assume it's just been powered on and communicating at 115200bps
      Serial2.updateBaudRate(115200); //Start the software serial port, this time at user's chosen baud rate
      delay(1000);
      nano.setBaud(baudRate);           //Tell the module to go to the chosen baud rate. Ignore the response msg
      Serial2.updateBaudRate(baudRate); //Start the software serial port, this time at user's chosen baud rate
      delay(1000);
    }
    else
    {
      //The M6E has these settings no matter what
      nano.setTagProtocol(); //Set protocol to GEN2
      nano.setAntennaPort(); //Set TX/RX antenna ports to 1
      return (true);
    }
    cpt_try++;
  }
  return (false);
}

void GPIO_init()
{
  // Button
  pinMode(start_button.Pin, INPUT_PULLUP);
  attachInterrupt(start_button.Pin, isr, FALLING);

  // Led declaration
  pinMode(led_red_status.Pin, OUTPUT);
  pinMode(led_red_wifi.Pin, OUTPUT);
  pinMode(led_green_status.Pin, OUTPUT);
  pinMode(led_green_wifi.Pin, OUTPUT);

  digitalWrite(led_red_status.Pin, HIGH);
  digitalWrite(led_red_wifi.Pin, HIGH);
  digitalWrite(led_green_wifi.Pin, HIGH);
  digitalWrite(led_green_status.Pin, HIGH);
  delay(1000);
  digitalWrite(led_red_status.Pin, LOW);
  digitalWrite(led_red_wifi.Pin, LOW);
  digitalWrite(led_green_wifi.Pin, LOW);
  digitalWrite(led_green_status.Pin, LOW);
}

void Led_blink(Led *led_user, unsigned long blink_timing)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > blink_timing)
  {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    if (led_user->State == LOW)
    {
      led_user->State = HIGH;
    }
    else
    {
      led_user->State = LOW;
    }
    // set the LED with the ledState of the variable:
    digitalWrite(led_user->Pin, led_user->State);
  }
}

void setup()
{
  String memory_data;
  bool configured = false;
  int power_rfid = DEFAULT_POWER;

  Serial.begin(115200);
  while (!Serial)
    ;

  GPIO_init();
  Serial.println("Gpio Init");

  /**********
   * 
   * Read memory
   * 
   * **********/

  memory_init();
  Serial.println("memory init");
  memory_read_string(&memory_data, ADDR_CONFIG);
  Serial.print("Config data: ");
  Serial.println(memory_data);
  if (memory_data.equals("1"))
  {
    Serial.println("SUKITO module was configured");

    memory_read_string(&memory_data, ADDR_ID);
    Serial.println(memory_data);
    set_id(memory_data);

    memory_read_string(&memory_data, ADDR_SSID);
    Serial.println(memory_data);
    set_ssid_user(memory_data);

    memory_read_string(&memory_data, ADDR_PWD);
    Serial.println(memory_data);
    set_pwd_user(memory_data);
    configured = true;
  }
  else
  {
    Serial.println("SUKITO module was NOT configured");
    configured = false;
  }

  /**********
   * 
   * Captive portal start
   * 
   * **********/

  Serial.println("Mode AP begin");
  init_AP_com();
  Serial.println("Mode AP set");
  digitalWrite(led_green_wifi.Pin, HIGH);

  unsigned long start_ap_time = millis();

  while (1)
  {
    int captive_state = 0;
    captive_state = captive_portale_home(configured);
    if (captive_state == 1)
    {
      end_AP_com(); // Fermé le mode Access Point
      WIFI_is_set = true;
      break;
    }
    else if (captive_state == 2)
    {
      end_AP_com(); // Fermé le mode Access Point
      break;
    }
    else if ((millis() - start_ap_time) > 60000UL)
    {
      end_AP_com(); // Fermé le mode Access Point
      Serial.println("Timeout");
      break;
    }
    Led_blink(&led_green_wifi, 1000);
    //Serial.println(captive_state);
  }

  /**********
   * 
   * Save Configuration 
   * 
   * **********/
  if (WIFI_is_set)
  {
    Serial.println("memory saving:");

    memory_save_string("1", ADDR_CONFIG);
    memory_read_string(&memory_data, ADDR_CONFIG);
    Serial.println(memory_data);
    memory_send_save();

    memory_save_string(get_id(), ADDR_ID);
    memory_read_string(&memory_data, ADDR_ID);
    Serial.println(memory_data);
    memory_send_save();

    memory_save_string(get_ssid_user(), ADDR_SSID);
    memory_read_string(&memory_data, ADDR_SSID);
    Serial.println(memory_data);
    memory_send_save();

    memory_save_string(get_pwd_user(), ADDR_PWD);
    memory_read_string(&memory_data, ADDR_PWD);
    Serial.println(memory_data);
    memory_send_save();
  }

  digitalWrite(led_green_wifi.Pin, LOW);

  /**********
   * 
   * init rfid module
   * 
   * **********/
  if (RFID_init(RFID_BAURDRATE) == false)
  {
    Serial.println("Module failed to respond. Please check wiring.");
    digitalWrite(led_red_status.Pin, HIGH);
    AWS_publish_RFID(false, 0, 0, 0);
    while (1)
      ; //Freeze!
  }
  Serial.println("Module is initialized.");
  digitalWrite(led_green_status.Pin, HIGH);

  nano.setRegion(REGION_EUROPE);
  nano.setReadPower(DEFAULT_POWER);
  //Max Read TX Power is 27.00 dBm and may cause temperature-limit throttling
  delay(1500);

  digitalWrite(led_green_status.Pin, HIGH);
  power_rfid = analogRead(POWER_COMMAND_PIN);
  power_rfid = map(power_rfid, 0, 3120, 0, 2700);

  Serial.print("Power = ");
  Serial.println(power_rfid);

  nano.setReadPower(power_rfid);
  nano.startReading(); //Begin scanning for tags
  state = NFC_READ_RUNNING;

  /**********
   * 
   * Connection wifi
   * 
   * **********/

  if (connect_to_ssid(10) == STA_CONNECTED)
  { // connexion au réseau entré par l'utilisateur
    Serial.println("Connecté au réseaux");
    AWS_is_connect = AWS_connection(2);
    digitalWrite(led_green_wifi.Pin, HIGH);
  }
  else
  {
    Serial.println("Non connecté");
    digitalWrite(led_red_wifi.Pin, HIGH);
    state = WIFI_STA_ERROR;
  }
}

void loop()
{
  byte responseType;
  byte tagEPCBytes;
  unsigned long ttl_current_time;
  int power_rfid = DEFAULT_POWER;
  bool epc_present = false;
  int total_epc = 0;

  switch (state)
  {
  case NFC_READ_STOP:

    digitalWrite(led_green_status.Pin, HIGH);

    if (start_button.pressed)
    {
      delay(500);
      Serial.println("Start button pressed GO RUNNING STATE");
      start_button.pressed = false;
      state = NFC_READ_RUNNING;

      power_rfid = analogRead(POWER_COMMAND_PIN);
      power_rfid = map(power_rfid, 0, 3120, 0, 2700);

      Serial.print("Power = ");
      Serial.println(power_rfid);

      nano.setReadPower(power_rfid);
      nano.startReading(); //Begin scanning for tags
      delay(500);
    }
    break;

  case NFC_READ_RUNNING:
    ttl_current_time = millis();
    if (nano.check() == true) //Check to see if any new data has come in from module
    {
      responseType = nano.parseResponse(); //Break response into tag ID, RSSI, frequency, and timestamp
      ttl_epc = analogRead(TTL_COMMAND_PIN);
      ttl_epc = map(ttl_epc, 0, 3130, 2000, 10000);

      Serial.print("ttl_epc = ");
      Serial.println(ttl_epc);

      if (responseType == RESPONSE_IS_KEEPALIVE)
      {
        Led_blink(&led_green_status, ttl_epc);
        Serial.println(F("Scanning"));

        if (nb_epc_read > 0)
        {
          Serial.print("NB Epc = ");
          Serial.println(nb_epc_read);
          total_epc += nb_epc_read;
          for (int i = 0; i < nb_epc_read; i++)
          {
            Serial.print("Send EPC:");
            err_code = send_request(list_EPC[i]);
            if (err_code == HTTP_POST_SUCCESS)
            {
              Serial.println("Request send success");
              list_EPC[i] = "";
            }
            else
            {
              Serial.println("error sending request");
              state = WIFI_STA_ERROR;
            }
            if (AWS_is_connect)
            {
              if(AWS_publish_RFID(true, nb_epc_read, rssi, freq))
              {
                nb_epc_read=0;
              }
            }
          }
          nb_epc_read = 0;
        }
      }
      else if (responseType == RESPONSE_IS_TAGFOUND)
      {
        //If we have a full record we can pull out the fun bits

        rssi = nano.getTagRSSI(); //Get the RSSI for this tag read

        freq = nano.getTagFreq(); //Get the frequency this tag was detected at

        long timeStamp = nano.getTagTimestamp(); //Get the time this was read, (ms) since last keep-alive message

        tagEPCBytes = nano.getTagEPCBytes(); //Get the number of bytes of EPC from response

        Serial.print(F("rssi: "));
        Serial.println(rssi);

        Serial.print(F("freq: "));
        Serial.println(freq);

        Serial.print(F("time: "));
        Serial.println(timeStamp);

        //Print EPC bytes, this is a subsection of bytes from the response/msg array
        Serial.print(F("Read epc["));
        for (byte x = 0; x < tagEPCBytes; x++)
        {
          if (nano.msg[31 + x] < 0x10)
            Serial.print(F("0")); //Pretty print
          Serial.print(nano.msg[31 + x], HEX);
          Serial.print(F(" "));
          actual_epc_read = String(actual_epc_read + String(nano.msg[31 + x], HEX));
        }
        Serial.print(F("]"));
        Serial.println("\n");

        Serial.print("Last epc read: ");
        Serial.println(last_epc_read);

        Serial.print("Actual epc read: ");
        Serial.println(actual_epc_read);

        for (int i = 0; i < NUMBER_EPC; i++)
        {
          if (actual_epc_read.equals(list_EPC[i]))
          {
            epc_present = true;
            break;
          }
          else
          {
            epc_present = false;
          }
        }

        if (epc_present || actual_epc_read.equals(last_epc_read))
        {
          Serial.println("Read the same EPC");
          if (ttl_current_time - ttl_previous_time >= ttl_epc)
          {
            Serial.println("But the ttl is past, register the epc to send it later");
            ttl_previous_time = ttl_current_time;
            last_epc_read = actual_epc_read;
            list_EPC[nb_epc_read] = actual_epc_read;
            if (nb_epc_read == NUMBER_EPC - 1)
            {
              nb_epc_read = 0;
            }
            else
            {
              nb_epc_read++;
            }
            Serial.print("nb_epc_read: ");
            Serial.println(nb_epc_read);
          }
        }
        else
        {
          Serial.println("Read a different EPC, register to send it later");
          ttl_previous_time = ttl_current_time;
          last_epc_read = actual_epc_read;
          list_EPC[nb_epc_read] = actual_epc_read;
          if (nb_epc_read == NUMBER_EPC - 1)
          {
            nb_epc_read = 0;
          }
          else
          {
            nb_epc_read++;
          }
          Serial.print("nb_epc_read: ");
          Serial.println(nb_epc_read);
        }
        actual_epc_read = "";
      }
      else if (responseType == ERROR_CORRUPT_RESPONSE)
      {
        Serial.println("Bad CRC");
      }
      else
      {
        Serial.println("Unknown error");
        state = NFC_ERROR;
      }
    }
    break;
  case WIFI_STA_ERROR:
    digitalWrite(led_green_wifi.Pin, LOW);
    Led_blink(&led_red_wifi, 1000);
    if (connect_to_ssid(10) == STA_CONNECTED)
    { // connexion au réseau entré par l'utilisateur
      Serial.println("Connecté au réseaux");
      digitalWrite(led_red_wifi.Pin, HIGH);
      AWS_is_connect = AWS_connection(2);
      digitalWrite(led_red_wifi.Pin, LOW);
      digitalWrite(led_green_wifi.Pin, HIGH);
      state = NFC_READ_RUNNING;
    }
    else
    {
      Serial.println("Non connecté");
    }
    break;
  case NFC_ERROR:
    digitalWrite(led_green_status.Pin, LOW);
    Led_blink(&led_red_status, 1000);
    break;
  default:
    break;
  }
  if (start_button.pressed)
  {
    Serial.println("Start button pressed GO STOP STATE");
    start_button.pressed = false;
    state = NFC_READ_STOP;
    nano.stopReading();
  }
}
