#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include "DHT.h"
#include <Servo.h>

// ------------------------ PINES ------------------------
#define PIN_TOUCH 4        // TTP223 capacitivo
#define PIN_METAL 5        // Sensor inductivo LJ12A3
#define PIN_HUMEDAD 6      // DHT22

//SERVOS
//SERVO 0 - COMPUERTA TUBO
//SERVO 1 - COMPUERTA METAL
//SERVO 2 - COMPUERTA PLASTICO
//SERVO 3 - COMPUERTA ORGANICOS
//SERVO 4 - COMPUERTA PAPEL

// ------------------------ DHT11 cambiar a 22 ------------------------
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ------------------------ TCS34725 ------------------------
Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_50MS,
  TCS34725_GAIN_4X
);

// ------------------------ VARIABLES ------------------------
bool hay_objeto = false;
bool hay_metal = false;
bool hay_plastico = false;
float hay_humedad = 0;

//                FUNCIONES DE SERVOMOTORES
void abrir_compuerta(int numServo){
  if (numServo < 0 || numServo > 5) return; // Validación de rango
  
  servos[numServo].write(180);
  delay(4000);
}

void cerrar_compuerta(int numServo){
  if (numServo < 0 || numServo > 5) return; // Validación de rango
  
  servos[numServo].write(35);
}


//                FUNCIONES DE SENSORES

// ---- SENSOR CAPACITIVO TTP223 ----
bool sensor_capacitivo() {
  return digitalRead(PIN_TOUCH) == HIGH;   
}

// ---- SENSOR DE METAL ----
bool sensor_metal() {
  return digitalRead(PIN_METAL) == LOW;    // LOW = metal detectado
}

// ---- SENSOR DE COLOR PARA PLÁSTICO ----
bool sensor_plastico() {
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);

  if (c < 50) return false;  

  // Convertir a RGB (0–255)
  float red = (r * 255.0) / c;
  float green = (g * 255.0) / c;
  float blue = (b * 255.0) / c;

  // valor de prueba para plastico (no c cual sea)
  if (red > 80 && green > 80 && blue > 80) return true;

  return false;
}

// ---- SENSOR DE HUMEDAD ----
float sensor_humedad() {
  float h = dht.readHumidity();
  if (isnan(h)) return -1;   // En caso de error
  return h;
}

// =======================================================
//                        SETUP
// =======================================================
void setup() {
  Serial.begin(9600);

  pinMode(PIN_TOUCH, INPUT);
  pinMode(PIN_METAL, INPUT_PULLUP);

  dht.begin();

  if (tcs.begin()) {
    Serial.println("TCS34725 detectado!");
  } else {
    Serial.println("No se encontró el TCS34725!");
  }

  servos[0].attach(7);
  servos[1].attach(8);
  servos[2].attach(9);
  servos[3].attach(10);
  servos[4].attach(11);

  Serial.println("Sistema listo!");
}

void loop() {


  hay_objeto = sensor_capacitivo(); //para saber si hay algo

  if (hay_objeto) {
    Serial.println("Objeto detectado");
    // revisar metal
    humedad_inicio = sensor_humedad();
    hay_metal = sensor_metal();
    if (hay_metal) {
      hay_plastico = false;
      Serial.println("Es METAL");
      abrir_compuerta(1);
    } else {
      hay_plastico = sensor_plastico();
      if (hay_plastico) {
        Serial.println("Es PLÁSTICO");
        abrir_compuerta(2);
      } else {
        delay(2000);
        humedad_final = sensor_humedad();
        if (1 <= (humedad_final - humedad_inicial)) abrir_compuerta(3);
      }
    }

    hay_humedad = sensor_humedad();
    Serial.print("Humedad detectada: ");
    Serial.println(hay_humedad);

    abrir_compuerta(0);
  } else {
    Serial.println("No hay objeto");
    hay_metal = false;
    hay_plastico = false;
    hay_humedad = 0;
  }

  Serial.println("-----------------------------");
  delay(500);
}