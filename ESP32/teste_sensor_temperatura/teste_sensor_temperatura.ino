#include "DHT.h"

#define DHT_PIN 32                // PINO DO SENSOR TEMPERATURA
#define DHTTYPE DHT11


DHT dht(DHT_PIN, DHTTYPE);


void setup() {
  // put your setup code here, to run once:
  delay(500);

  // inicia monitor serial
  Serial.begin(115200);           
  Serial.println("Iniciando SETUP ");

  // inicia o monitoramento DHT
  dht.begin();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);

  // le sensor de temperatura
  float temperatura = dht.readTemperature();
  
  Serial.print("temperatura no sensor ");
  Serial.println(temperatura);
}
