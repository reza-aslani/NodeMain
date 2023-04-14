# NodeMain

1.install MCCI_LoRaWAN_LMIC_library
2.change fie : C:\Users\Aslani\Documents\Arduino\libraries\MCCI_LoRaWAN_LMIC_library\project_config\lmic_project_config.h
to :

#define LMIC_LORAWAN_SPEC_VERSION   LMIC_LORAWAN_SPEC_VERSION_1_0_3

// project-specific definitions
#define CFG_eu868 1
//#define CFG_us915 1
//#define CFG_au915 1
//#define CFG_as923 1
// #define LMIC_COUNTRY_CODE LMIC_COUNTRY_CODE_JP      /* for as923-JP; also define CFG_as923 */
//#define CFG_kr920 1
//#define CFG_in866 1
#define CFG_sx1276_radio 1
//#define LMIC_USE_INTERRUPTS


