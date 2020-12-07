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
#define LDR_PIN 33                // PINO DO SENSOR LDR
#define PIR_PIN 32                // PINO DO SENSOR PIR

#define THRESHOLD_LDR 2000        // threshold de acionamento

FirebaseData firebase_data;        // INSTANCIA DO FIREBASE

int ldr_value = 0;                // variaveis de entrada de sensor
int pir_value = 0;                // variaveis de entrada de sensor
String hora_atual = "";           // variavel para armazenar a hora atual

bool alterado = false;            // variavel para saber se houve alteração no banco
int modo_operacao = 0;            // variavel que define o modo de operacao
int tempo_desligar = 0;           // variavel que define o tempo para desligar
String h_acender = "";            // variavel que define o horário de acionar
String h_apagar = "";             // variavel que define o horário de desligar
bool acender = false;             // variavel que define se a lampada dever ser acendida
bool estado_lamp = false;         // variavel que armazena o estado atual da lampada

WiFiUDP ntp_UDP;                   // variavel para conexão via udp com o servidor NTP
NTPClient time_client(ntp_UDP);     // instancia do NTP client


// FUNCAO DE SETUP 
void setup() {
  delay(500);
  
  Serial.begin(115200);           // inicia monitor serial
  Serial.println("Iniciando SETUP ");
  
  Serial.println("Iniciando conexão WIFI");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  // inicia WIFI com SSID e senha
  
  while (WiFi.status() != WL_CONNECTED) {    // aguarda conexão wifi ser estabelecida
    delay(2000);    
  }
  Serial.println("Wifi conectado!");
    
  Firebase.begin(HOST, DBKEY);              // inicia conexão com Firebase  
  Serial.println("Iniciado FB");
  
  pinMode(LED_PIN, OUTPUT);                 // pino para ativar LED  
  pinMode(LDR_PIN, INPUT);                  // pino para ler LDR  
  pinMode(PIR_PIN, INPUT);                  // pino para ler pir

  time_client.begin();                       // inicia client
  time_client.setTimeOffset(-10800);         // timeset para operar no horario de brasilia   
}


/* 
  * Loop de verificação de variaveis de configuração, sensores e controle da lampada. 
  * Primeiro é verificado as variaveis de configuração que definem o funcionamento, e
  * depois é realizado o controle de iluminação. 
  */
void loop() {
  
  if(Firebase.getBool(firebase_data, "/alterado")){  // busca o valor da variavel alterado
    if(firebase_data.dataType() == "boolean"){       // se o tipo retornado estiver certo          
      alterado = firebase_data.boolData();           // valor retornado

      if(alterado == true){                         // verifica se houve alteração

        /* seção que atualiza as variaveis locais com os novos dados do banco */
        if(Firebase.getInt(firebase_data, "/modo_operacao")){ // busca o valor da variavel modo_operacao
          if(firebase_data.dataType() == "int"){               // se o tipo retornado estiver certo          
            modo_operacao = firebase_data.intData();           // valor retornado
          } else {
            Serial.println(firebase_data.dataType());        
          }
        }else{
          Serial.println("Erro ao dar retrieve no modo_operacao");        
        }
        
        if(Firebase.getInt(firebase_data, 
                            "/configuracoes/tempo_desligar")){ // busca o valor da variavel tempo_desligar
          if(firebase_data.dataType() == "int"){                // se o tipo retornado estiver certo          
            tempo_desligar = firebase_data.intData();           // valor retornado
          }
        }
        
        if(Firebase.getString(firebase_data, 
                            "/configuracoes/horarios/acender")){ // busca o valor da variavel acender
          if(firebase_data.dataType() == "string"){               // se o tipo retornado estiver certo          
            h_acender = firebase_data.stringData();               // valor retornado
          }
        }
        
        if(Firebase.getString(firebase_data, 
                            "/configuracoes/horarios/apagar")){ // busca o valor da variavel apagar
          if(firebase_data.dataType() == "string"){              // se o tipo retornado estiver certo          
            h_apagar = firebase_data.stringData();               // valor retornado
          }
        }

        alterado = false;                                       // seta variavel de alterado como false                                
        
        if(Firebase.setBool(firebase_data, "/alterado", alterado)){  // atualiza banco com o valor
          Serial.println("dado enviado");
        }
      }
    }
  }else {
    Serial.println(firebase_data.errorReason());    
  }

  /* seção de controle da lampada */
  switch (modo_operacao){
    case 0:               // modo de operação por controle manual
      if(Firebase.getBool(firebase_data, "/acionar_manual")){ // busca o valor da variavel acender
        if(firebase_data.dataType() == "boolean"){            // se o tipo retornado estiver certo          
          acender = firebase_data.boolData();                 // valor retornado
          digitalWrite(LED_PIN, acender);                     // ativa/desativa led conforme comando acender
        }else{
          Serial.println("Data type not bool");  
        }
      }else {
        Serial.println("Erro no retrieve");
      }
      
      break;
    
    case 1:               // modo de operação por sensor de presença
      acender = digitalRead(PIR_PIN);       // le o estado do sensor PIR      
      digitalWrite(LED_PIN, acender);       // ativa/desativa led conforme comando acender      
      if (acender == true)                  // se o comando for de acender
        delay(tempo_desligar*1000);         // mantem aceso por pelo menos 'tempo_desligar' segundos
      
      break;
    
    case 2:               // modo de operação por sensor LDR      
      acender = (analogRead(LDR_PIN) > THRESHOLD_LDR);  // le o estado do sensor LDR e compara com o threshold      
      digitalWrite(LED_PIN, acender);                   // ativa/desativa led conforme comando acender
      break;
    
    case 3:               // modo operação por tempo definido
      while(!time_client.update()){                // atualiza hora atual
        time_client.forceUpdate();                 // atualiza hora atual
      }
      
      hora_atual = time_client.getFormattedTime(); // busca hora atual do serviro NTP
      hora_atual = split_time(hora_atual);        // retorna apenas hora e minuto
      
      if (hora_atual == h_apagar){
        digitalWrite(LED_PIN, false);             // ativa/desativa led conforme comando acender
      }
      if (hora_atual == h_acender) {
        digitalWrite(LED_PIN, true);              // ativa/desativa led conforme comando acender
      }
      break;
    
    default:
    break;
  }
  
  if (estado_lamp != acender){                            // se o estado  da lampada estiver diferente 
                                                          // do comando acender, atualiza o estado da lamp
                                                          // no banco
    if(Firebase.setBool(firebase_data, "/estado_lamp", acender)){  // atualiza banco com o valor      
      estado_lamp = acender;
    }
  }  
}


/*
  Função para remover os segundos da string retornada do servidor NTP
*/
String split_time (String str){
  String hora_min = "";
  for(int i = 0; i < str.length()-3; i++){
    hora_min.concat(str.charAt(i));    
  }
  return hora_min;
}
