#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include "DHT.h"
#include <Servo.h>

// ------------------------ PINES ------------------------
#define PIN_TOUCH 7        // TTP223 capacitivo
#define PIN_METAL 8        // Sensor inductivo LJ12A3
#define DHTPIN 9     // DHT22
#define trigPin 10
#define echoPin 11
//SERVOS
//SERVO 0 - COMPUERTA TUBO
//SERVO 1 - COMPUERTA METAL
//SERVO 2 - COMPUERTA PLASTICO
//SERVO 3 - COMPUERTA ORGANICOS
//SERVO 4 - COMPUERTA PAPEL
Servo servos[5];
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
void abrir_compuerta(int numServo, int posicion = 125){  
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
  long distancia = duracion * 0.034 / 2;
  Serial.print("Distancia: ");
  Serial.println(distancia);

  // Convertir tiempo → distancia (cm)
  return distancia;
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
uint16_t sensor_plastico() {
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);
  Serial.print("R: ");
  Serial.print(r/c);
  Serial.print("G: ");
  Serial.print(g/c);
  Serial.print("B: ");
  Serial.print(b/c);

  return c;
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
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  servos[0].attach(2);
  servos[1].attach(3);
  servos[2].attach(4);
  servos[3].attach(5);
  servos[4].attach(6);

  // --- POSICIÓN INICIAL (evita los movimientos al inicio) ---
  servos[0].write(20);
  servos[1].write(25);

  for (int i = 2; i < 5; i++) {
    servos[i].write(35);  // o la posición que tú quieras como inicial
  }

  delay(500);
  Serial.println("Sistema listo!");
}

void loop() {

  uint16_t luz_inicial= sensor_plastico();
  Serial.println("esta es la luz inicial");
  Serial.println(luz_inicial);
  float humedad_inicio = sensor_humedad();
  long distancia=sensor_distancia();
  if (sensor_capacitivo() || distancia < 6 || distancia > 10) {
    Serial.println("Objeto detectado");
    delay(2000);
    if(sensor_capacitivo()){
    Serial.println(humedad_inicio);
      delay(3000);
      if (sensor_metal()) {
        Serial.println("Es METAL");
        abrir_compuerta(1);
      }
      else {
        delay(14000);
        float humedad_final = sensor_humedad();
        Serial.println(humedad_final);
        abrir_compuerta(3);
        } 

    }
    else {
      uint16_t luz_final= sensor_plastico();
      Serial.println("esta es la luz final");
      Serial.println(luz_final);
      uint16_t suma= (luz_final - luz_inicial);
      Serial.println(suma);
      if (70 >= (luz_final - luz_inicial)) {
        Serial.println("Es PLÁSTICO");
        abrir_compuerta(2);
      } 
      else{
        // if es papel entonces
        abrir_compuerta(4, 160);
      }
    }
      


      abrir_compuerta(0,160);
      // delay(6000);
      cerrar_compuerta(0,20);
      cerrar_compuerta(1,25);

      for(int u=2; u<5;u++){
        cerrar_compuerta(u);
      }
      delay(4000);
  } else {
      Serial.println("No hay objeto");
      
  }

  Serial.println("-----------------------------");
  delay(500);
}