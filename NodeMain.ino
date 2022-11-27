
#include <Arduino.h>
#include <arduino_lmic_hal_boards.h>
#include <wiring_digital.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <header.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

static unsigned long lastSendSec = 0;
static unsigned long lastSec = 0;
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty cycle limitations).
const unsigned TX_INTERVAL = 30;

// LoRaWAN NwkSKey, network session key
// This should be in big-endian (aka msb).
static const PROGMEM u1_t NWKSKEY[16] = {0x33, 0x04, 0x33, 0x91, 0xF8, 0xF7, 0xD0, 0xDB, 0xCC, 0xCB, 0x45, 0x0C, 0x35, 0x4A, 0xF6, 0x12};

// LoRaWAN AppSKey, application session key
// This should also be in big-endian (aka msb).
static const u1_t PROGMEM APPSKEY[16] = {0xD4, 0xEF, 0xFE, 0x4C, 0x9B, 0x7A, 0xF8, 0x33, 0xC9, 0x1C, 0x75, 0xE5, 0x2C, 0x2B, 0x22, 0x7D};

// LoRaWAN end-device address (DevAddr)
// The library converts the address to network byte order as needed, so this should be in big-endian (aka msb) too.
static const u4_t DEVADDR = 0x260B0FEC; // <-- Change this address for every node!

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in arduino-lmic/project_config/lmic_project_config.h,
// otherwise the linker will complain).
void os_getArtEui(u1_t *buf) {}
void os_getDevEui(u1_t *buf) {}
void os_getDevKey(u1_t *buf) {}

// Pin mapping
// lmic_pinmap

// const lmic_pinmap lmic_pins = {
//     .nss = PA4,
//     .rxtx = LMIC_UNUSED_PIN,
//     .rst = PB0, // reset pin
//     .dio = {PA3, PB5, LMIC_UNUSED_PIN},
// };

const lmic_pinmap my_lmic_pins = {
  PA4,
  LMIC_UNUSED_PIN,
  PB0, // reset pin
  {PA3, PB5, LMIC_UNUSED_PIN},
};
SoftwareSerial mySerial(PA10, PA9); // RX, TX
void setup()
{
  // pinMode(LED_BUILTIN, OUTPUT);
  // digitalWrite(LED_BUILTIN, LOW);

  // while (!Serial); // wait for Serial to be initialized
  // digitalWrite(LED_BUILTIN, HIGH);

  // analogReadResolution(12);

  Serial.begin(115200);
  mySerial.begin(9600);
  delay(100);
  Serial.println();
  Serial.println();
  Serial.println(F("Starting LoraWAN-Node ... "));

  setup_real_values();

  //SetLightDimmer(50);

  prepare_lmic();

  // Start job
  do_send_values(&sendjob);
}
void onEvent(ev_t ev)
{
  Serial.print(os_getTime());
  Serial.print(": ");
  switch (ev)
  {
    case EV_SCAN_TIMEOUT:
      Serial.println(F("EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      Serial.println(F("EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      Serial.println(F("EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      Serial.println(F("EV_BEACON_TRACKED"));
      break;
    case EV_JOINING:
      Serial.println(F("EV_JOINING"));
      break;
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));
      break;
    /*
      || This event is defined but not used in the code. No
      || point in wasting codespace on it.
      ||
      || case EV_RFU1:
      ||     Serial.println(F("EV_RFU1"));
      ||     break;
    */
    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      break;
    case EV_TXCOMPLETE:
      Serial.println();
      Serial.println(F("++++++++++++++++++++++  EV_TXCOMPLETE (includes waiting for RX windows)"));
      lastSendSec = lastSec;
      if (LMIC.txrxFlags & TXRX_ACK)
        Serial.println(F("Received ack"));
      if (LMIC.dataLen)
      {
        mySerial.println("--------------- Received ");
        digitalWrite(PC13,!digitalRead(PC13));
        SetLightPower(true);
        mySerial.println(LMIC.dataLen);
        mySerial.println(F(" bytes of payload"));
        //mySerial.println(LMIC.frame,String);
        mySerial.println(LMIC.frame[LMIC.dataBeg]);
        SetLightDimmer(LMIC.frame[LMIC.dataBeg]);
        Serial.println();
      }
      // Schedule next transmission
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send_values);
      // os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      break;
    case EV_LOST_TSYNC:
      Serial.println(F("EV_LOST_TSYNC"));
      break;
    case EV_RESET:
      Serial.println(F("EV_RESET"));
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println(F("EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      Serial.println(F("EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      Serial.println(F("EV_LINK_ALIVE"));
      break;
    /*
      || This event is defined but not used in the code. No
      || point in wasting codespace on it.
      ||
      || case EV_SCAN_FOUND:
      ||    Serial.println(F("EV_SCAN_FOUND"));
      ||    break;
    */
    case EV_TXSTART:
      Serial.println(F("EV_TXSTART"));
      break;
    case EV_TXCANCELED:
      Serial.println(F("EV_TXCANCELED"));
      break;
    case EV_RXSTART:
      /* do not print anything -- it wrecks timing */
      break;
    case EV_JOIN_TXCOMPLETE:
      Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
      break;
    default:
      Serial.print(F("Unknown event: "));
      Serial.println((unsigned)ev);
      break;
  }
}

void do_send_values(osjob_t *j) // osjob_t *j)
{
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND)
  {
    Serial.println();
    Serial.println(F("***********  OP_TXRXPEND, not sending"));
    //// LMIC_reset();
    //// os_runloop_once();
    // Serial.println(F("***********  Will reset..."));
    // prepare_lmic();
  }
  else
  {
    // Prepare upstream data transmission at the next possible time.
    // LMIC_setTxData2(1, mydata, sizeof(mydata) - 1, 0);
    String str = readValues_Real();

    Serial.print("str:");
    Serial.println(str);
    const int n = str.length();
    Serial.print("len(str):");
    Serial.println(n);
    Serial.println();

    char str_char_array[n];
    strcpy(str_char_array, str.c_str());
    Serial.print("str_char_array:");
    Serial.println(str_char_array);
    Serial.println();

    LMIC_setTxData2(1, (uint8_t *)str_char_array, n, 0);
    Serial.println(F("Packet queued"));
    Serial.println();
  }
  // Next TX is scheduled after TX_COMPLETE event.
}

void prepare_lmic()
{
  // #ifdef VCC_ENABLE
  //     // For Pinoccio Scout boards
  //     pinMode(VCC_ENABLE, OUTPUT);
  //     digitalWrite(VCC_ENABLE, HIGH);
  //     delay(1000);
  // #endif

  // LMIC init
  // os_init();
  os_init_ex(&my_lmic_pins);

  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

  // Set static session parameters. Instead of dynamically establishing a session
  // by joining the network, precomputed session parameters are be provided.
  // once. Copy them to a temporary buffer here, LMIC_setSession will
  // copy them into a buffer of its own again.
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  Serial.println(F("appKey-mode-1"));
  LMIC_setSession(0x13, DEVADDR, nwkskey, appskey);

  //#if defined(CFG_eu868)
  // Set up the channels used by the Things Network, which corresponds
  // to the defaults of most gateways. Without this, only three base
  // channels from the LoRaWAN specification are used, which certainly
  // works, so it is good for debugging, but can overload those
  // frequencies, so be sure to configure the full frequency range of
  // your network here (unless your network autoconfigures them).
  // Setting up channels should happen after LMIC_setSession, as that
  // configures the minimal channel set. The LMIC doesn't let you change
  // the three basic settings, but we show them here.

  // Serial.println(F("CFG_eu868"));
  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI); // g-band
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK, DR_FSK), BAND_MILLI);   // g2-band

  // TTN defines an additional channel at 869.525Mhz using SF9 for class B
  // devices' ping slots. LMIC does not have an easy way to define set this
  // frequency and support for class B is spotty and untested, so this
  // frequency is not configured here.
  // #else
  // #error Region not supported
  // #endif

  // for (int channel = 0; channel < 8; ++channel)
  //     LMIC_disableChannel(channel);

  // for (int channel = 16; channel < 72; ++channel)
  //     LMIC_disableChannel(channel);

  // Disable link check validation
  LMIC_setLinkCheckMode(0);

  // TTN uses SF9 for its RX2 window.
  LMIC.dn2Dr = DR_SF9;

  // Set data rate and transmit power for uplink
  LMIC_setDrTxpow(DR_SF7, 14);
}


void loop()
{
  unsigned long nowSec;
  nowSec = millis() / 1000.0;
  if (nowSec != lastSec)
  {
    Serial.printf(".%i.", nowSec - lastSendSec);

    lastSec = nowSec;


//    if (GetLightDimmer() >= 100) {
//      for (int i = 100; i > 0; i - 10) {
//        SetLightDimmer(i);
//        delay(100);
//      }
//    }
  //  SetLightDimmer(GetLightDimmer() - 5);

    // if (nowSec % 45 == 0)
    // {
    //     lastSendSec = nowSec;
    //     Serial.println();
    //     Serial.println(F("########## Sending Values..."));
    //     do_send_values();
    // }
  }

  os_runloop_once();
}
