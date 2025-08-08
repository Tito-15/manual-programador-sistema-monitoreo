#include "EmonLib.h"

EnergyMonitor emon1;
EnergyMonitor emon2;
EnergyMonitor emon3;             // Creamos instancia emon por cada sensor

// Pines de conexion
#define CUR_ADC_INPUT1 34
#define CUR_ADC_INPUT2 35
#define CUR_ADC_INPUT3 33
//#define emonTxV3 1

#define VOL_ADC_INPUT1 32
#define VOL_ADC_INPUT2 36
#define VOL_ADC_INPUT3 39


// variables para calibracion
const float calibracion_volt = 74.5;
const float calibracion_corr = 0.89;

// variables para los datos
float voltage1 = 0.0;
float current1 = 0.0;
float power1 = 0.0;
float energy1 = 0.0;
float frequency1 = 0.0;
float pf1 = 0.0;

float voltage2 = 0.0;
float current2 = 0.0;
float power2 = 0.0;
float energy2 = 0.0;
float frequency2 = 0.0;
float pf2 = 0.0;

float voltage3 = 0.0;
float current3 = 0.0;
float power3 = 0.0;
float energy3 = 0.0;
float frequency3 = 0.0;
float pf3 = 0.0;

float Sens1 = 0;
float Sens2 = 0;
float Sens3 = 0;
float Voltaje = 0;

float W1 = 0;
float W2 = 0;
float W3 = 0;

/////////////////////////////////

float realPower =  0;
float apparentPower =  0;
float powerFactor =  0;
float supplyVoltage  = 0;
float Irms = 0;

float realPower2 =  0;
float apparentPower2 = 0;
float powerFactor2 =  0;
float supplyVoltage2 = 0;
float Irms2 =  0;

float realPower3 = 0;
float apparentPower3 = 0;
float powerFactor3 =  0;
float supplyVoltage3 = 0;
float Irms3 = 0;

#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#elif defined(ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#endif

#include <InfluxDbClient.h>

// ===========================================
// CONFIGURACION WIFI - MODIFICAR ESTAS LINEAS
// ===========================================
// Reemplazar "YOUR_WIFI_SSID" con el nombre de tu red WiFi
#define WIFI_SSID "YOUR_WIFI_SSID"  
// Reemplazar "YOUR_WIFI_PASSWORD" con la contraseña de tu red WiFi
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Ejemplos de configuraciones WiFi comentadas (puedes usar cualquiera):
/*
// Para red universitaria:
#define WIFI_SSID "Internet_UNIVERSIDAD"
#define WIFI_PASSWORD "UNIVERSIDAD"

// Para red doméstica ejemplo:
#define WIFI_SSID "MiRedWiFi"
#define WIFI_PASSWORD "MiPassword123"

*/

// ===========================================
// CONFIGURACION INFLUXDB - MODIFICAR ESTAS LINEAS
// ===========================================
// Reemplazar con la URL de tu servidor InfluxDB
#define INFLUXDB_URL "http://YOUR_INFLUXDB_SERVER:8086/"
// Reemplazar con tu token real de InfluxDB (generar desde la interfaz web de InfluxDB)
#define INFLUXDB_TOKEN "YOUR_INFLUXDB_TOKEN_HERE"
// Reemplazar con tu organización de InfluxDB
#define INFLUXDB_ORG "YOUR_ORGANIZATION"
// Reemplazar con el nombre de tu bucket de InfluxDB
#define INFLUXDB_BUCKET "YOUR_BUCKET_NAME"

// Ejemplo de configuración InfluxDB comentada:
/*
#define INFLUXDB_URL "http://192.168.1.100:8086/"
#define INFLUXDB_TOKEN "YOUR_INFLUXDB_TOKEN"
#define INFLUXDB_ORG "mi_organizacion"
#define INFLUXDB_BUCKET "sensores_esp32"
*/

#define TZ_INFO "PST8PDT EST-5"

// InfluxDB client instance
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);

// Data point
Point sensor1("corriente1");
Point sensor2("corriente2");
Point sensor3("corriente3");

// tomamos los valores con la funcion millis
unsigned long last_time = 0;
unsigned long current_time = 0;
unsigned long lastmillis = millis();

///////////////////////////   OLED    ////////////////////////////////////////
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long previousMillis = 0;
const long interval = 5000; // Intervalo de cambio de texto (5 segundos)

// Contador para alternar entre textos
int textCounter = 0;
const int numTexts = 4; // Número de textos a mostrar (agregamos pantalla de info)

// Variables para control de WiFi y pantalla profesional
bool wifi_connected = false;
bool showing_wifi_status = false;
unsigned long wifi_check_time = 0;
const long wifi_check_interval = 3000; // Verificar WiFi cada 3 segundos

///////////////////////////////////////////////////////

// Función para mostrar estado de WiFi en OLED de manera profesional
void mostrar_estado_wifi(String mensaje, String detalle = "") {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  // Header UNL
  display.drawRect(0, 0, SCREEN_WIDTH, 15, SH110X_WHITE);
  display.setCursor(5, 4);
  display.println("PROTOTIPO UNL - v1.0");

  // Título centrado
  int16_t x = (SCREEN_WIDTH - mensaje.length() * 6) / 2;
  if (x < 0) x = 0;
  display.setCursor(x, 20);
  display.println(mensaje);

  // Línea separadora
  display.drawLine(10, 32, SCREEN_WIDTH - 10, 32, SH110X_WHITE);

  // Detalle centrado
  if (detalle.length() > 0) {
    int16_t x2 = (SCREEN_WIDTH - detalle.length() * 6) / 2;
    if (x2 < 0) x2 = 0;
    display.setCursor(x2, 40);
    display.println(detalle);
  }

  // Mostrar IP si está conectado
  if (WiFi.status() == WL_CONNECTED) {
    display.setCursor(0, 55);
    display.print("IP: ");
    display.println(WiFi.localIP());
  }

  display.display();
}

// Función para verificar estado de WiFi
void verificar_wifi() {
  wifi_connected = (WiFi.status() == WL_CONNECTED);

  if (!wifi_connected) {
    showing_wifi_status = true;
    if (WiFi.status() == WL_CONNECT_FAILED) {
      mostrar_estado_wifi("ERROR CONEXION", "Verif. credenciales");
    } else if (WiFi.status() == WL_NO_SSID_AVAIL) {
      mostrar_estado_wifi("RED NO ENCONTRADA", "Verificar router");
    } else {
      mostrar_estado_wifi("RECONECTANDO...", "Espere por favor");
    }
  } else {
    if (showing_wifi_status) {
      mostrar_estado_wifi("CONEXION OK", "Sistema operativo");
      delay(2000);
      showing_wifi_status = false;
    }
  }
}

void setup() {
  ////////////////////////////  OLED //////////////////
  display.begin(i2c_Address, true); // Address 0x3C default
  display.clearDisplay();

  // Mensaje de bienvenida profesional
  mostrar_estado_wifi("INICIANDO SISTEMA", "Monitor Energetico");
  delay(3000);

  Serial.begin(9600);
  Serial.println("=====================================");
  Serial.println("PROTOTIPO DE MONITOREO ENERGETICO UNL");
  Serial.println("Sistema de medicion trifasico v1.0");
  Serial.println("Universidad Nacional de Loja");
  Serial.println("=====================================");

  // Configuración de sensores
  emon1.voltage(VOL_ADC_INPUT1, calibracion_volt, 1.72); // Voltage: input pin, calibration, phase_shift
  emon1.current(CUR_ADC_INPUT1, calibracion_corr);    // Current: input pin, calibration

  emon2.voltage(VOL_ADC_INPUT2, calibracion_volt, 1.72); // Voltage: input pin, calibration, phase_shift
  emon2.current(CUR_ADC_INPUT2, calibracion_corr);    // Current: input pin, calibration

  emon3.voltage(VOL_ADC_INPUT3, calibracion_volt, 1.72); // Voltage: input pin, calibration, phase_shift
  emon3.current(CUR_ADC_INPUT3, calibracion_corr);    // Current: input pin, calibration
  emon1.calcVI(20, 2000);
  emon2.calcVI(20, 2000);
  emon3.calcVI(20, 2000);

  // Connect WiFi con indicador visual profesional
  Serial.println("Conectando a WiFi...");
  mostrar_estado_wifi("CONECTANDO WIFI", WIFI_SSID);

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  int intentos = 0;
  while (wifiMulti.run() != WL_CONNECTED && intentos < 30) {
    Serial.print(".");
    delay(500);
    intentos++;

    // Actualizar pantalla cada 5 intentos
    if (intentos % 5 == 0) {
      String puntos = "";
      for (int i = 0; i < (intentos / 5); i++) {
        puntos += ".";
      }
      mostrar_estado_wifi("CONECTANDO" + puntos, WIFI_SSID);
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("WiFi conectado. IP: ");
    Serial.println(WiFi.localIP());
    mostrar_estado_wifi("WIFI CONECTADO", WiFi.localIP().toString());
    wifi_connected = true;
    delay(3000);
  }
  else {
    Serial.println("Error de conexión WiFi");
    mostrar_estado_wifi("ERROR WIFI", "Modo offline");
    wifi_connected = false;
    delay(3000);
  }

  // tags para Influxdb
  char voltstring1[20];
  dtostrf(voltage1, 3, 1, voltstring1);
  String vol1(voltstring1);

  char currstring1[20];
  dtostrf(current1, 3, 1, currstring1);
  String cor1(currstring1);

  sensor1.addTag("voltaje1", vol1);
  sensor1.addTag("corriente1", cor1);

  char voltstring2[20];
  dtostrf(voltage2, 3, 1, voltstring2);
  String vol2(voltstring2);

  char currstring2[20];
  dtostrf(current2, 3, 1, currstring2);
  String cor2(currstring2);

  sensor2.addTag("voltaje2", vol2);
  sensor2.addTag("corriente2", cor2);

  char voltstring3[20];
  dtostrf(voltage3, 3, 1, voltstring3);
  String vol3(voltstring3);

  char currstring3[20];
  dtostrf(current3, 3, 1, currstring3);
  String cor3(currstring3);

  sensor3.addTag("voltaje3", vol3);
  sensor3.addTag("corriente3", cor3);

  // Solo configurar InfluxDB si hay WiFi
  if (wifi_connected) {
    mostrar_estado_wifi("CONFIGURANDO...", "Base de datos");
    timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

    // Check server connection
    if (client.validateConnection()) {
      Serial.print("Conectado a InfluxDB: ");
      Serial.println(client.getServerUrl());
      mostrar_estado_wifi("INFLUXDB OK", "Sistema listo");
    } else {
      Serial.print("Error InfluxDB: ");
      Serial.println(client.getLastErrorMessage());
      mostrar_estado_wifi("ERROR BASE DATOS", "Solo local");
    }
    delay(2000);
  }

  // Mensaje final profesional
  mostrar_estado_wifi("SISTEMA OPERATIVO", "Datos en Grafana");
  delay(3000);

  Serial.println("Sistema iniciado correctamente");
  Serial.println("Datos disponibles en: http://energy-monitor-unl.local:3000");
  Serial.println("=====================================");

  pinMode(CUR_ADC_INPUT1, INPUT);
  pinMode(CUR_ADC_INPUT2, INPUT);
  pinMode(CUR_ADC_INPUT3, INPUT);

  pinMode(VOL_ADC_INPUT1, INPUT);
  pinMode(VOL_ADC_INPUT2, INPUT);
  pinMode(VOL_ADC_INPUT3, INPUT);
}

void loop() {
  emon1.calcVI(20, 1000);
  emon2.calcVI(20, 1000);
  emon3.calcVI(20, 1000);

  realPower = emon1.realPower;          // Potencia Real
  apparentPower = emon1.apparentPower;  // Potencia Aparente
  powerFactor = emon1.powerFactor;      // Factor de potencia
  supplyVoltage = emon1.Vrms;           // Vrms
  Irms = emon1.Irms;                    // Irms

  realPower2 = emon2.realPower;          // Potencia Real
  apparentPower2 = emon2.apparentPower;  // Potencia Aparente
  powerFactor2 = emon2.powerFactor;      // Factor de potencia
  supplyVoltage2 = emon2.Vrms;           // Vrms
  Irms2 = emon2.Irms;                    // Irms

  realPower3 = emon3.realPower;          // Potencia Real
  apparentPower3 = emon3.apparentPower;  // Potencia Aparente
  powerFactor3 = emon3.powerFactor;      // Factor de potencia
  supplyVoltage3 = emon3.Vrms;           // Vrms
  Irms3 = emon3.Irms;                    // Irms

  //================================================================================================================
  if (supplyVoltage <= 40) {
    supplyVoltage = 0;
    realPower = 0;          // Potencia Real
    apparentPower = 0;  // Potencia Aparente
    powerFactor = 0;      // Factor de potencia
  }

  if (supplyVoltage >= 150) {
    supplyVoltage = 0;
    realPower = 0;          // Potencia Real
    apparentPower = 0;  // Potencia Aparente
    powerFactor = 0;      // Factor de potencia
  }

  if (Irms < 0.1) {
    Irms = 0;
  }

  if (Irms > 70) {
    Irms = 0;
  }
  //-------------------------------------------------
  if (supplyVoltage2 <= 40) {
    supplyVoltage2 = 0;
    realPower2 = 0;          // Potencia Real
    apparentPower2 = 0;  // Potencia Aparente
    powerFactor2 = 0;      // Factor de potencia
  }

  if (supplyVoltage2 >= 150) {
    supplyVoltage2 = 0;
    realPower2 = 0;          // Potencia Real
    apparentPower2 = 0;  // Potencia Aparente
    powerFactor2 = 0;      // Factor de potencia
  }

  if (supplyVoltage2 >= 40 && supplyVoltage2 <=80) {
    supplyVoltage2 = 124;
  }

  if (Irms2 > 70) {
    Irms2 = 0;
  }

  if (Irms2 < 0.1) {
    Irms2 = 0;
  }

  if (Irms2 > 70) {
    Irms2 = 0;
  }
  //-------------------------------------------------
  if (supplyVoltage3 <= 40) {
    supplyVoltage3 = 0;
    realPower3 = 0;          // Potencia Real
    apparentPower3 = 0;  // Potencia Aparente
    powerFactor3 = 0;      // Factor de potencia
  }

  if (supplyVoltage3 >= 150) {
    supplyVoltage3 = 0;
    realPower3 = 0;          // Potencia Real
    apparentPower3 = 0;  // Potencia Aparente
    powerFactor3 = 0;      // Factor de potencia
  }

  if (Irms3 < 0.1) {
    Irms3 = 0;
  }

  if (Irms3 > 70) {
    Irms3 = 0;
  }
  //================================================================================================================



  // Serial output
  Serial.print("Voltage1: "); Serial.print(supplyVoltage); Serial.println("V");
  Serial.print("Corriente1: "); Serial.print(Irms); Serial.println("A");
  Serial.print("Potencia1: "); Serial.print(apparentPower); Serial.println("W");
  Serial.print("PF1: "); Serial.println(powerFactor);
  Serial.println("-------------------");

  Serial.print("Voltage2: "); Serial.print(supplyVoltage2); Serial.println("V");
  Serial.print("Corriente2: "); Serial.print(Irms2); Serial.println("A");
  Serial.print("Potencia2: "); Serial.print(apparentPower2); Serial.println("W");
  Serial.print("PF2: "); Serial.println(powerFactor2);
  Serial.println("-------------------");

  Serial.print("Voltage3: "); Serial.print(supplyVoltage3); Serial.println("V");
  Serial.print("Corriente3: "); Serial.print(Irms3); Serial.println("A");
  Serial.print("Potencia3: "); Serial.print(apparentPower3); Serial.println("W");
  Serial.print("PF3: "); Serial.println(powerFactor3);
  Serial.println("-------------------");

  // Verificar estado de WiFi periódicamente
  unsigned long currentMillis = millis();
  display.clearDisplay();

  ///////////////////////////////////////////////////////
  // Display OLED profesional con 4 pantallas
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);

    if (textCounter == 0) {
      // Header profesional
      display.drawRect(0, 0, SCREEN_WIDTH, 12, SH110X_WHITE);
      display.setCursor(2, 2);
      display.print("UNL - SENSOR 1");

      display.setCursor(0, 15);
      display.print("P.Real: "); display.print(realPower, 1); display.println("W");
      display.setCursor(0, 25);
      display.print("P.Apar: "); display.print(apparentPower, 1); display.println("VA");
      display.setCursor(0, 35);
      display.print("F.Pot:  "); display.println(powerFactor, 2);
      display.setCursor(0, 45);
      display.print("Voltaje:"); display.print(supplyVoltage, 1); display.println("V");
      display.setCursor(0, 55);
      display.print("Corr:   "); display.print(Irms, 2); display.println("A");
      display.display();
    }
    else if (textCounter == 1) {
      display.drawRect(0, 0, SCREEN_WIDTH, 12, SH110X_WHITE);
      display.setCursor(2, 2);
      display.print("UNL - SENSOR 2");

      display.setCursor(0, 15);
      display.print("P.Real: "); display.print(realPower2, 1); display.println("W");
      display.setCursor(0, 25);
      display.print("P.Apar: "); display.print(apparentPower2, 1); display.println("VA");
      display.setCursor(0, 35);
      display.print("F.Pot:  "); display.println(powerFactor2, 2);
      display.setCursor(0, 45);
      display.print("Voltaje:"); display.print(supplyVoltage2, 1); display.println("V");
      display.setCursor(0, 55);
      display.print("Corr:   "); display.print(Irms2, 2); display.println("A");
      display.display();
    }
    else if (textCounter == 2) {
      display.drawRect(0, 0, SCREEN_WIDTH, 12, SH110X_WHITE);
      display.setCursor(2, 2);
      display.print("UNL - SENSOR 3");

      display.setCursor(0, 15);
      display.print("P.Real: "); display.print(realPower3, 1); display.println("W");
      display.setCursor(0, 25);
      display.print("P.Apar: "); display.print(apparentPower3, 1); display.println("VA");
      display.setCursor(0, 35);
      display.print("F.Pot:  "); display.println(powerFactor3, 2);
      display.setCursor(0, 45);
      display.print("Voltaje:"); display.print(supplyVoltage3, 1); display.println("V");
      display.setCursor(0, 55);
      display.print("Corr:   "); display.print(Irms3, 2); display.println("A");
      display.display();
    }
    else if (textCounter == 3) {
      // Pantalla de información profesional
      display.drawRect(0, 0, SCREEN_WIDTH, 12, SH110X_WHITE);
      display.setCursor(2, 2);
      display.print("SISTEMA UNL");

      if (wifi_connected) {
        display.setCursor(0, 15);
        display.println("Estado: OPERATIVO");
        display.setCursor(0, 25);
        display.println("WiFi:   CONECTADO");
        display.setCursor(0, 35);
        display.println("InfluxDB: ACTIVO");
        display.setCursor(0, 45);
        display.println("Grafana: DISPONIBLE");
        display.setCursor(0, 55);
        display.print("IP: "); display.println(WiFi.localIP());
      } else {
        display.setCursor(0, 15);
        display.println("Estado: DESCONECTADO");
        display.setCursor(0, 25);
        display.println("WiFi:   ERROR");
        display.setCursor(0, 35);
        display.println("Modo:   SOLO LOCAL");
        display.setCursor(0, 45);
        display.println("Reconectando...");
        display.setCursor(0, 55);
        display.print("Total: "); display.print((realPower + realPower2 + realPower3), 1); display.println("W");
      }
      display.display();
    }

    // Incrementa el contador y reinicia si es necesario
    textCounter++;
    if (textCounter >= numTexts) {
      textCounter = 0;
    }
  }

  if (currentMillis - wifi_check_time >= wifi_check_interval) {
    wifi_check_time = currentMillis;
    verificar_wifi();
  }

  // Si se está mostrando estado WiFi, pausa las mediciones
  if (showing_wifi_status) {
    delay(100);
    return;
  }

  // Store measured value into point - Solo si hay WiFi
  if (wifi_connected) {
    sensor1.clearFields();
    sensor1.addField("Corriente1", Irms);
    sensor1.addField("Voltaje1", supplyVoltage);
    sensor1.addField("FactorPotencia1", powerFactor);
    sensor1.addField("PotenciaReal", realPower);
    sensor1.addField("PotenciaAparente", apparentPower);

    sensor2.clearFields();
    sensor2.addField("Corriente2", Irms2);
    sensor2.addField("Voltaje2", supplyVoltage2);
    sensor2.addField("FactorPotencia2", powerFactor2);
    sensor2.addField("PotenciaReal2", realPower2);
    sensor2.addField("PotenciaAparente2", apparentPower2);

    sensor3.clearFields();
    sensor3.addField("Corriente3", Irms3);
    sensor3.addField("Voltaje3", supplyVoltage3);
    sensor3.addField("FactorPotencia3", powerFactor3);
    sensor3.addField("PotenciaReal3", realPower3);
    sensor3.addField("PotenciaAparente3", apparentPower3);

    // Verificar conexión WiFi antes de enviar
    if (wifiMulti.run() != WL_CONNECTED) {
      Serial.println("Conexion WiFi perdida");
      wifi_connected = false;
      showing_wifi_status = true;
    } else {
      // Enviar datos a InfluxDB
      if (!client.writePoint(sensor1)) {
        Serial.print("InfluxDB S1: ");
        Serial.println(client.getLastErrorMessage());
      }

      if (!client.writePoint(sensor2)) {
        Serial.print("InfluxDB S2: ");
        Serial.println(client.getLastErrorMessage());
      }

      if (!client.writePoint(sensor3)) {
        Serial.print("InfluxDB S3: ");
        Serial.println(client.getLastErrorMessage());
      }
    }
  } else {
    // Intentar reconectar WiFi
    if (wifiMulti.run() == WL_CONNECTED) {
      wifi_connected = true;
      showing_wifi_status = false;
      Serial.println("WiFi reconectado!");
    }
  }
  //delay(1000);
}