#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include "DHT.h"
#include <Servo.h>

// ------------------------ PINES ------------------------
#define PIN_TOUCH 7
#define PIN_METAL 8
#define DHTPIN 9
#define trigPin 10
#define echoPin 11

Servo servos[5];

#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_154MS,  // Mejor para ver plástico transparente
  TCS34725_GAIN_4X
);

// ------------------------ SERVOS ------------------------
void abrir_compuerta(int numServo, int posicion = 125){
  servos[numServo].write(180);
  delay(4000);
}

void cerrar_compuerta(int numServo, int posicion = 35){
  servos[numServo].write(posicion);
}

// ------------------------ SENSORES ------------------------
long sensor_distancia(){
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracion = pulseIn(echoPin, HIGH);
  long distancia = duracion * 0.034 / 2;

  Serial.print("Distancia: ");
  Serial.println(distancia);

  return distancia;
}

bool sensor_capacitivo() {
  return digitalRead(PIN_TOUCH) == HIGH;
}

bool sensor_metal() {
  return digitalRead(PIN_METAL) == LOW;
}

void leer_color(uint16_t &r, uint16_t &g, uint16_t &b, uint16_t &c){
  tcs.getRawData(&r,&g,&b,&c);

  Serial.print("R: "); Serial.print(r);
  Serial.print("  G: "); Serial.print(g);
  Serial.print("  B: "); Serial.print(b);
  Serial.print("  C: "); Serial.println(c);
}

float sensor_humedad(){
  float h = dht.readHumidity();
  if (isnan(h)) return -1;
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

  if (!tcs.begin()) {
    Serial.println("No se encontró el TCS34725");
  } else {
    Serial.println("TCS34725 listo!");
  }

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  servos[0].attach(2);
  servos[1].attach(3);
  servos[2].attach(4);
  servos[3].attach(5);
  servos[4].attach(6);

  servos[0].write(20);
  servos[1].write(25);
  for(int i=2;i<5;i++) servos[i].write(35);

  delay(500);
  Serial.println("Sistema listo!");
}

// =======================================================
//                       LOOP
// =======================================================
void loop() {

  // ----- Lectura inicial -----
  Serial.println("Luz inicial en el tubo:");
  uint16_t r,g,b,c;
  leer_color(r,g,b,c);

  float humedad_inicio = sensor_humedad();
  long distancia = sensor_distancia();

  if (sensor_capacitivo() || distancia < 6 || distancia > 10) {
    Serial.println("OBJETO DETECTADO");
    delay(3000);

    // ------------ SI ES CONDUCTIVO → (metal u orgánico) ------------
    if(sensor_capacitivo()){

      Serial.print("Humedad inicial: ");
      Serial.println(humedad_inicio);
      delay(3000);

      if (sensor_metal()) {
        Serial.println("Es METAL");
        abrir_compuerta(1);
      }
      else {
        float humedad_final = sensor_humedad();
        Serial.print("Humedad final: ");
        Serial.println(humedad_final);
        Serial.println("Es ORGÁNICO");
        abrir_compuerta(3);
      }
    }

    // ------------ NO ES CONDUCTIVO → plástico o papel ------------
    else {

      Serial.println("Leyendo color final...");
      uint16_t r2,g2,b2,c2;
      leer_color(r2,g2,b2,c2);

      // ---- Método IR (plástico transparente) ----
      int ir = (int)c2 - ((int)r2 + (int)g2 + (int)b2);
      

      Serial.print("IR estimado: ");
      Serial.println(ir);

      // UMBRAL AJUSTABLE
      if (ir < 10) {
        Serial.println("→ PLÁSTICO");
        abrir_compuerta(2);
      }
      else {
        Serial.println("→ PAPEL / CARTÓN");
        abrir_compuerta(4,160);
      }
    }

    // ------------ REINICIAR SISTEMA ------------
    abrir_compuerta(0,160);

    cerrar_compuerta(0,20);
    cerrar_compuerta(1,25);

    for(int u=2; u<5; u++){
      cerrar_compuerta(u);
    }

    delay(4000);

  } else {
    Serial.println("No hay objeto.");
  }

  Serial.println("--------------------------");
  delay(500);
}
