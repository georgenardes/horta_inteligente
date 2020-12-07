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

#define LED_PIN 27                // PINO DO LED
#define BOMBA_PIN 28              // PINO DO BOMBA
#define VENTOINHA_PIN 29          // PINO DO VENTOINHA


#define LDR_PIN 32                // PINO DO SENSOR LDR
#define UM_PIN 33                 // PINO DO SENSOR PIR
#define TEMP_PIN 34               // PINO DO SENSOR PIR
#define NIVEL_PIN 35              // PINO DO SENSOR PIR


#define THRESHOLD_NIVEL 2000

FirebaseData firebaseData;        // INSTANCIA DO FIREBASE

int ldr_value = 0;                // variaveis de entrada de sensor
int um_value = 0;                 // variaveis de entrada de sensor
int temp_value = 0;               // variaveis de entrada de sensor

String hora_atual = "";           // variavel para armazenar a hora atual

bool alterado = false;            // variavel para saber se houve alteração no banco

int modo_operacao = 0;            // variavel que define o modo de operacao

bool acender_lamp = false;        // variavel que define se a lampada dever ser acendida
bool acionar_bomba = false;       // variavel que define se a lampada dever ser acendida
bool acionar_ventoinha = false;   // variavel que define se a lampada dever ser acendida


bool estado_lamp = false;         // variavel que armazena o estado atual da lampada
bool estado_bomba = false;        // variavel que armazena o estado atual da lampada
bool estado_ventoinha = false;    // variavel que armazena o estado atual da lampada


WiFiUDP ntpUDP;                   // variavel para conexão via udp com o servidor NTP
NTPClient timeClient(ntpUDP);     // instancia do NTP client


// FUNCAO DE SETUP 
void setup() {
  delay(500);
  
  Serial.begin(115200);                    // inicia monitor serial
  Serial.println("Iniciando SETUP ");
  
  Serial.println("Iniciando conexão WIFI");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);    // inicia WIFI com SSID e senha
  
  while (WiFi.status() != WL_CONNECTED) {  // aguarda conexão wifi ser estabelecida
    delay(500);
  }
  Serial.println("Wifi conectado!");
    
  Firebase.begin(HOST, DBKEY);              // inicia conexão com Firebase  
  Serial.println("Iniciado FB");
  
  pinMode(LED_PIN, OUTPUT);                 // pino para ativar LED
  pinMode(BOMBA_PIN, OUTPUT);               // pino para ativar LED
  pinMode(VENTOINHA_PIN, OUTPUT);           // pino para ativar LED
    
  pinMode(LDR_PIN, INPUT);                  // pino para ler LDR  
  pinMode(UM_PIN, INPUT);                   // pino para ler pir
  pinMode(TEMP_PIN, INPUT);                 // pino para ler pir
  pinMode(NIVEL_PIN, INPUT);                // pino para ler pir

  timeClient.begin();                       // inicia client
  timeClient.setTimeOffset(-10800);         // timeset para operar no horario de brasilia   
}


/* 
  * Loop de verificação de variaveis de configuração, sensores e controle da lampada. 
  * Primeiro é verificado as variaveis de configuração que definem o funcionamento, e
  * depois é realizado o controle de iluminação. 
  */
void loop() {
  
  if(Firebase.getBool(firebaseData, "/alterado")){  // busca o valor da variavel alterado
    if(firebaseData.dataType() == "boolean"){       // se o tipo retornado estiver certo          
      alterado = firebaseData.boolData();           // valor retornado

      if(alterado == true){                         // verifica se houve alteração

        /* seção que atualiza as variaveis locais com os novos dados do banco */
        if(Firebase.getInt(firebaseData, "/modo_operacao")){ // busca o valor da variavel modo_operacao
          if(firebaseData.dataType() == "int"){               // se o tipo retornado estiver certo          
            modo_operacao = firebaseData.intData();           // valor retornado
          } else {
            Serial.println(firebaseData.dataType());        
          }
        }else{
          Serial.println("Erro ao dar retrieve no modo_operacao");        
        }

        if(Firebase.getInt(firebaseData, 
                            "/configuracoes/luminosidade")){ // busca o valor da variavel tempo_desligar
          if(firebaseData.dataType() == "int"){              // se o tipo retornado estiver certo          
            ldr_value = firebaseData.intData();              // valor retornado
          }
        }

        if(Firebase.getInt(firebaseData, 
                            "/configuracoes/temperatura")){ // busca o valor da variavel tempo_desligar
          if(firebaseData.dataType() == "int"){             // se o tipo retornado estiver certo          
            temp_value = firebaseData.intData();            // valor retornado
          }
        }

        if(Firebase.getInt(firebaseData, 
                            "/configuracoes/umidade")){    // busca o valor da variavel tempo_desligar
          if(firebaseData.dataType() == "int"){            // se o tipo retornado estiver certo          
            um_value = firebaseData.intData();             // valor retornado
          }
        }
        
        alterado = false;                                       // seta variavel de alterado como false                                
        
        if(Firebase.setBool(firebaseData, "/alterado", alterado)){  // atualiza banco com o valor
          Serial.println("data sended");
        }
      }
    }
  }else {
    Serial.println(firebaseData.errorReason());    
  }


  /* seção de controle dos atuadores */
  switch (modo_operacao){
    case 0:               // modo de operação por controle manual
      Serial.println("modo operação manual");  
      if(Firebase.getBool(firebaseData, "/controle/lampada")){ // busca o valor da variavel acender
        if(firebaseData.dataType() == "boolean"){                 // se o tipo retornado estiver certo          
          acender_lamp = firebaseData.boolData();                 // valor retornado
          digitalWrite(LED_PIN, acender_lamp);                    // ativa/desativa led conforme comando acender
        }else{
          Serial.println("Data type not bool");  
        }
      }else {
        Serial.println("Erro no retrieve");
      }

      if(Firebase.getBool(firebaseData, "/controle/bomba_agua")){ // busca o valor da variavel acender
        if(firebaseData.dataType() == "boolean"){                 // se o tipo retornado estiver certo          
          acionar_bomba = firebaseData.boolData();                // valor retornado
          digitalWrite(BOMBA_PIN, acionar_bomba);                 // ativa/desativa led conforme comando acender
        }else{
          Serial.println("Data type not bool");  
        }
      }else {
        Serial.println("Erro no retrieve");
      }

      if(Firebase.getBool(firebaseData, "/controle/ventilador")){ // busca o valor da variavel acender
        if(firebaseData.dataType() == "boolean"){                 // se o tipo retornado estiver certo          
          acionar_ventoinha = firebaseData.boolData();            // valor retornado
          digitalWrite(VENTOINHA_PIN, acionar_ventoinha);         // ativa/desativa led conforme comando acender
        }else{
          Serial.println("Data type not bool");  
        }
      }else {
        Serial.println("Erro no retrieve");
      }
      
      break;
-------------------------------------------------------------------------------------------------------------------------
    case 1:               // modo de operação automatico
      acender_lamp = digitalRead(PIR_PIN);      // le o estado do sensor PIR      
      digitalWrite(LED_PIN, acender);           // ativa/desativa led conforme comando acender      
      if (acender == true)                      // se o comando for de acender
        delay(tempo_desligar*1000);             // mantem aceso por pelo menos 'tempo_desligar' segundos

      acender = (analogRead(LDR_PIN) > THRESHOLD_LDR);  // le o estado do sensor LDR e compara com o threshold      
      digitalWrite(LED_PIN, acender);                   // ativa/desativa led conforme comando acender
      break;

      
      break;
    
    case 2:               // modo de operação por sensor LDR      
      acender = (analogRead(LDR_PIN) > THRESHOLD_LDR);  // le o estado do sensor LDR e compara com o threshold      
      digitalWrite(LED_PIN, acender);                   // ativa/desativa led conforme comando acender
      break;
    
    case 3:                // modo operação por tempo definido
      while(!timeClient.update()){                // atualiza hora atual
        timeClient.forceUpdate();                 // atualiza hora atual
      }
      hora_atual = timeClient.getFormattedTime(); // busca hora atual do serviro NTP


      if (estado_lamp == true){
        if (hora_atual > h_apagar){
          digitalWrite(LED_PIN, false);                   // ativa/desativa led conforme comando acender
        }
      } else {
        if (hora_atual > h_acender){
          digitalWrite(LED_PIN, true);                    // ativa/desativa led conforme comando acender
        }
      }
    break;
    
    default:
    break;
  }
  
---------------------------------------------------------------------------------------------------------------
  
  if (estado_lamp != acender_lamp){
    if(Firebase.setBool(firebaseData, "/atuadores/lampada", acender)){   // atualiza banco com o valor      
      estado_lamp = acender_lamp;
    }
  }
  if (estado_bomba != acionar_bomba){
    if(Firebase.setBool(firebaseData, "/atuadores/bomba_agua", acender)){ // atualiza banco com o valor      
      estado_bomba = acionar_bomba;
    }
  }
  if (estado_ventoinha != acionar_ventoinha){
    if(Firebase.setBool(firebaseData, "/estado_ventoinha", acender)){     // atualiza banco com o valor      
      estado_ventoinha = acionar_ventoinha;
    }
  }

  delay(1000);
}
