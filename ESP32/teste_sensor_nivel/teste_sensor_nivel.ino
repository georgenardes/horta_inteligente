
#define NIVEL_PIN 35                // PINO DO SENSOR NIVEL


void setup() {
  // put your setup code here, to run once:
  delay(500);

  // inicia monitor serial
  Serial.begin(115200);           
  Serial.println("Iniciando SETUP ");

  pinMode(NIVEL_PIN, INPUT);  
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);

  // le sensor de temperatura
  int nivel = digitalRead(NIVEL_PIN);
  
  Serial.print("nivel no sensor ");
  Serial.println(nivel);
}
