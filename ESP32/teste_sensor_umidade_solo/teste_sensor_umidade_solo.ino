#define UMS_PIN 33                // PINO DO SENSOR UMIDADE SOLO

void setup() {
  // put your setup code here, to run once:
  delay(500);

  // inicia monitor serial
  Serial.begin(115200);           
  Serial.println("Iniciando SETUP ");

  pinMode(UMS_PIN, INPUT);                  // pino para ler umidade do solo  
  
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);

  // le sensor de umidade
  int umidade_solo = analogRead(UMS_PIN);
  
  Serial.print("Umidade no sensor ");
  Serial.println(umidade_solo);
}
