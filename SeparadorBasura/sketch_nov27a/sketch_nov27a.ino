#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include "DHT.h"
#include <Servo.h>

// ------------------------ PINES ------------------------
#define PIN_TOUCH 5        // TTP223 capacitivo
#define PIN_METAL 6        // Sensor inductivo LJ12A3
#define PIN_HUMEDAD 7      // DHT22
#define trigPin 8
#define echoPin 9
//SERVOS
//SERVO 0 - COMPUERTA TUBO
//SERVO 1 - COMPUERTA METAL
//SERVO 2 - COMPUERTA PLASTICO
//SERVO 3 - COMPUERTA ORGANICOS
//SERVO 4 - COMPUERTA PAPEL
Servo servos[5];
// ------------------------ DHT11 cambiar a 22 ------------------------
#define DHTTYPE DHT22
DHT dht(PIN_HUMEDAD, DHTTYPE);

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
void abrir_compuerta(int numServo, int posicion = 125){
  if (numServo < 0 || numServo > 5) return; // Validación de rango
  
  servos[numServo].write(180);
  delay(4000);
}

void cerrar_compuerta(int numServo, int posicion = 35){  
  servos[numServo].write(posicion);
}


//                FUNCIONES DE SENSORES
// ---- SENSOR DE DISTANCIA ULTRASONICO------
long sensor_distancia(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Recibir señal
  long duracion = pulseIn(echoPin, HIGH);

  // Convertir tiempo → distancia (cm)
  return duracion * 0.034 / 2;
}

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

  servos[0].attach(0);
  servos[1].attach(1);
  servos[2].attach(2);
  servos[3].attach(3);
  servos[4].attach(4);

  // --- POSICIÓN INICIAL (evita los movimientos al inicio) ---
  servos[1].write(25);

  for (int i = 2; i < 4; i++) {
    servos[i].write(35);  // o la posición que tú quieras como inicial
  }

  delay(500);
  Serial.println("Sistema listo!");
}

void loop() {


  if (sensor_capacitivo() || sensor_distancia() < 8) {
    Serial.println("Objeto detectado");
    // revisar metal
    float humedad_inicio = sensor_humedad();

  if (sensor_metal()) {
      hay_plastico = false;
      Serial.println("Es METAL");
      abrir_compuerta(1);
    } else {
      if (sensor_plastico()) {
        Serial.println("Es PLÁSTICO");
        abrir_compuerta(2);
      } else {
        delay(8000);
        float humedad_final = sensor_humedad();
        if (1 <= (humedad_final - humedad_inicio)){
          abrir_compuerta(3);
        } 
        else{
          // if es papel entonces
          abrir_compuerta(4, 160);
        }
      }
    }

    abrir_compuerta(0);
    delay(6000);
    cerrar_compuerta(1,25);

    for(int u=2; u<6;u++){
      cerrar_compuerta(u);
    }
  } else {
    Serial.println("No hay objeto");
    hay_metal = false;
    hay_plastico = false;
    hay_humedad = 0;
  }

  Serial.println("-----------------------------");
  delay(500);
}