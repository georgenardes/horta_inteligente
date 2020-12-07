// BIBLIOTECAS PARA TRABALHAR COM FIREBASE
#include <FirebaseESP32.h>
#include <FirebaseESP32HTTPClient.h>
#include <FirebaseJson.h>
#include <jsmn.h>

// BIBLIOTECA PARA TRABALHAR COM NTP SERVER
#include <NTPClient.h>
#include <WiFiUdp.h>

#define WIFI_SSID "NET_2GNardes"   // SSID DO WIFI
#define WIFI_PASSWORD "scania112"  // SENHA DO WIFI

#define HOST "smartlamp-b443e.firebaseio.com"           // ID DE CONEXÃO COM FIREBASE
#define DBKEY "ZjBLw3DJ0fMk6rlKayVFBWiNElZ02G3B2N2pSyyG"// CHAVE DE CONEXÃO COM FIREBASE


FirebaseData firebase_data;        // INSTANCIA DO FIREBASE

void setup() {
  // put your setup code here, to run once:
  delay(500);

  // inicia monitor serial
  Serial.begin(115200);           
  Serial.println("Iniciando SETUP ");

  
  Serial.println("Iniciando conexão WIFI");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  // inicia WIFI com SSID e senha
  
  while (WiFi.status() != WL_CONNECTED) {    // aguarda conexão wifi ser estabelecida
    delay(2000);    
  }
  Serial.println("Wifi conectado!");

  Firebase.begin(HOST, DBKEY);              // inicia conexão com Firebase  
  Serial.println("Iniciado FB");

  pinMode(13, OUTPUT);  
  pinMode(33, OUTPUT);  
  pinMode(32, OUTPUT);  
  pinMode(34, OUTPUT);  
  pinMode(35, OUTPUT);  
  pinMode(12, OUTPUT);
}
int i = 0;
void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  digitalWrite(13, HIGH);
  digitalWrite(33, HIGH);
  digitalWrite(32, HIGH);
  digitalWrite(34, HIGH);
  digitalWrite(35, HIGH);
  digitalWrite(12, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
  digitalWrite(33, LOW);
  digitalWrite(32, LOW);
  digitalWrite(34, LOW);
  digitalWrite(35, LOW);
  digitalWrite(12, LOW);

  /* não acioba bomba. deve ser pq a corrente é pouca*/

  if(Firebase.getBool(firebase_data, "/acionar_manual")){ // busca o valor da variavel acender
    if(firebase_data.dataType() == "boolean"){            // se o tipo retornado estiver certo          
      boolean acender = firebase_data.boolData();                 // valor retornado
      Serial.println("Data type bool");  
    }else{
      Serial.println("Data type not bool");  
    }
  }else {
    Serial.println("Erro no retrieve");
  }

  i++; 
  if(Firebase.setInt(firebase_data, "/modo_operacao", i)){  // atualiza banco com o valor      
    Serial.println("Data type bool");  
  }


  
}
