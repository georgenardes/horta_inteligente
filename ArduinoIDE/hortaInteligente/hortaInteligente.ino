#include <FirebaseESP32.h>
#include <FirebaseESP32HTTPClient.h>
#include <FirebaseJson.h>

// BIBLIOTECAS PARA TRABALHAR COM FIREBASE
#include <jsmn.h>
#include "DHT.h"      // biblioteca sensor de temperatura

// BIBLIOTECA PARA TRABALHAR COM NTP SERVER
#include <NTPClient.h>
#include <WiFiUdp.h>

#define WIFI_SSID "NET_2GNardes"   // SSID DO WIFI
#define WIFI_PASSWORD "scania112"  // SENHA DO WIFI

#define HOST "horta-inteligente-5231c.firebaseio.com"        // ID DE CONEXÃO COM FIREBASE
#define DBKEY "1F09QKX7E6RjdCZTSxvaINjKob86JF03XbbFCJIu"     // CHAVE DE CONEXÃO COM FIREBASE

#define LED_PIN 14                // PINO DO LED
#define BOMBA_PIN 13              // PINO DO BOMBA
#define VENTOINHA_PIN 12          // PINO DO VENTOINHA

#define LDR_PIN 35                // PINO DO SENSOR LDR
#define UM_PIN 32                 // PINO DO SENSOR UMIDADE
#define TEMP_PIN 33               // PINO DO SENSOR TEMPERATURA
#define NIVEL_PIN 26              // PINO DO SENSOR NIVEL

#define DHTTYPE DHT11
DHT dht(TEMP_PIN, DHTTYPE);       // biblioteca para monitoramento temperatura

FirebaseData firebaseData;        // INSTANCIA DO FIREBASE
bool alterado = false;            // variavel para saber se houve alteração no banco

int modo_operacao = 0;            // variavel que define o modo de operacao

int ldr_value = 0;                // valor limite de luminosidade definido pelo usuário
int um_value = 0;                 // valor limite de umidade definido pelo usuário
float temp_value = 0;             // valor limite de temperatura definido pelo usuário

int ldr_lido = 0;                // valor de luminosidade lido do sensor
int um_lido = 0;                 // valor de umidade lido do sensor
float temp_lido = 0;             // valor de temperatura lido do sensor
int nivel_lido = 0;            // nivel de agua lido do sensor 0 = tem agua; 1 = sem agua

int acender_lamp = 0;             // variavel que define se a lampada dever ser acendida
int acionar_bomba = 0;            // variavel que define se a bomba dever ser acionada
int acionar_ventoinha = 0;        // variavel que define se a ventoinha dever ser acionada

int estado_lamp = 0;              // variavel que armazena o estado atual da lampada
int estado_bomba = 0;             // variavel que armazena o estado atual da bomba
int estado_ventoinha = 0;         // variavel que armazena o estado atual da ventoinha

WiFiUDP ntpUDP;                   // variavel para conexão via udp com o servidor NTP
NTPClient timeClient(ntpUDP);     // instancia do NTP client

String hora_atual = "";           // variavel para armazenar a hora atual


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

  // inicia o monitoramento DHT
  dht.begin();

  timeClient.begin();                       // inicia client
  timeClient.setTimeOffset(-10800);         // timeset para operar no horario de brasilia   

  while(!timeClient.update()){              // atualiza hora atual
    timeClient.forceUpdate();               // atualiza hora atual
  }      
  hora_atual = split_hora(timeClient.getFormattedTime());  // busca hora atual do serviro NTP

  alterado = true;  // para forçar atualização dos dados
}


/* 
  * Loop de verificação de variaveis de configuração, sensores e atuadores. 
  * Primeiro é verificado as variaveis de configuração que definem o funcionamento, e
  * depois é realizado o controle dos atuadores. 
  */
void loop() {
  if(Firebase.getBool(firebaseData, "/alterado")){  // busca o valor da variavel alterado
    if(firebaseData.dataType() == "boolean"){       // se o tipo retornado estiver certo          
      alterado = firebaseData.boolData();           // valor retornado

      if(alterado == true){                         // verifica se houve alteração

        /* seção que atualiza as variaveis locais com os novos dados do banco */
        if(Firebase.getInt(firebaseData, "/modo")){      // busca o valor da variavel modo_operacao
          if(firebaseData.dataType() == "int"){          // se o tipo retornado estiver certo          
            modo_operacao = firebaseData.intData();      // valor retornado
          } else {
            Serial.println(firebaseData.dataType());        
          }
        }else{
          Serial.println("Erro ao dar retrieve no modo_operacao");        
        }

        if(Firebase.getInt(firebaseData, 
                            "/configuracoes/luminosidade")){ // busca o valor definido para luminosidade
          if(firebaseData.dataType() == "int"){              // se o tipo retornado estiver certo          
            ldr_value = firebaseData.intData();              // valor retornado
          }
        }

        if(Firebase.getInt(firebaseData, 
                            "/configuracoes/temperatura")){ // busca o valor definido para temperatura
          if(firebaseData.dataType() == "int"){             // se o tipo retornado estiver certo          
            temp_value = firebaseData.intData();            // valor retornado
          }
        }

        if(Firebase.getInt(firebaseData, 
                            "/configuracoes/umidade")){    // busca o valor definido para umidade
          if(firebaseData.dataType() == "int"){            // se o tipo retornado estiver certo          
            um_value = firebaseData.intData();             // valor retornado
          }
        }
        
        alterado = false;                                       // seta variavel de alterado como false                                
        
        if(Firebase.setBool(firebaseData, "/alterado", alterado)){  // atualiza banco com o valor
          Serial.println("variaveis atualizadas");
        }
      }
    }
  }else {
    Serial.println(firebaseData.errorReason());    
  }

  // leitura do nível de agua no reservatório
  nivel_lido = !digitalRead(NIVEL_PIN);

  // leitura de temperatura
  temp_lido = dht.readTemperature();             

  // leitura de umidade do solo
  um_lido = analogRead(UM_PIN);

  // leitura da luminosidade no ambiente
  ldr_lido = analogRead(LDR_PIN);

  /* seção de controle dos atuadores */
  switch (modo_operacao){
    
    case 0:               // modo de operação por controle manual
              
      Serial.println("modo operação manual");  
      
      if(Firebase.getInt(firebaseData, "/controle/lampada")){    // verifica a variavel de controle da lampada
        if(firebaseData.dataType() == "int"){                    // se o tipo retornado estiver certo          
          acender_lamp = firebaseData.intData();                 // valor retornado
          digitalWrite(LED_PIN, acender_lamp);                   // ativa/desativa led conforme comando acender
        }else{
          Serial.println("Data type not int");  
        }
      }else {
        Serial.println("Erro no retrieve");
      }

      if(Firebase.getInt(firebaseData, "/controle/bomba_agua")){ // verifica a variavel de controle da bomba_agua
        if(firebaseData.dataType() == "int"){                    // se o tipo retornado estiver certo         
          acionar_bomba = firebaseData.intData();                // valor retornado
          if(nivel_lido == 1){                                   // verifica o nivel da agua            
            digitalWrite(BOMBA_PIN, acionar_bomba);              // ativa/desativa bomba conforme comando acender
          }else{                                                 // se não tiver água
            acionar_bomba = 0;                                   // seta variavel para atualizar banco
            digitalWrite(BOMBA_PIN, 0);                          // desativa a bomba
          }
        }else{
          Serial.println("Data type not int");  
        }
      }else {
        Serial.println("Erro no retrieve");
      }

      if(Firebase.getInt(firebaseData, "/controle/ventilador")){ // verifica a variavel de controle do ventilador
        if(firebaseData.dataType() == "int"){                    // se o tipo retornado estiver certo          
          acionar_ventoinha = firebaseData.intData();            // valor retornado
          digitalWrite(VENTOINHA_PIN, acionar_ventoinha);        // ativa/desativa ventoinha conforme comando acender
        }else{
          Serial.println("Data type not int");  
        }
      }else {
        Serial.println("Erro no retrieve");
      }
      break;
    
    case 1:       // modo de operação automatico

      // ===========================================================
      // acionamento lampada            
      if (estado_lamp == 0){                                      // Se a lampada estiver apagada
        if (hora_atual == "08:00"){                             // Se hora atual for maior que as 8 da manha
          acender_lamp =  (ldr_lido > ldr_value);                 // verifica se o a itensidade de luz está acima
          digitalWrite(LED_PIN, acender_lamp);                    // ativa led conforme comando acender_lamp
        }
      } else {                                                    // Se lampada estiver acesa
        if (hora_atual == "18:00"){                             // Se hora atual passar das 18h
          acender_lamp = 0;                                       // seta variavel para atualizar o banco
          digitalWrite(LED_PIN, acender_lamp);                    // Desativa led 
        }
      }
      // ===========================================================

      // ===========================================================
      // acionamento ventoinha
      if(temp_lido > temp_value){                            // Se temperatura atual for maior que a delimitada
        acionar_ventoinha = 1;                               // seta variavel para atualizar o banco
        digitalWrite(VENTOINHA_PIN, acionar_ventoinha);      // Aciona ventoinha
      }else{
        acionar_ventoinha = 0;                               // seta variavel para atualizar o banco
        digitalWrite(VENTOINHA_PIN, acionar_ventoinha);      // Desativa ventoinha
      }
      // ===========================================================

      // ===========================================================
      // acionamento bomba
      if(nivel_lido){                                       // se tem agua
        if(um_lido > um_value){                             // Se umidade atual for maior que a delimitada
          acionar_bomba = 1;                                // seta variavel para salvar no banco
          digitalWrite(BOMBA_PIN, acionar_bomba);           // Aciona bomba
        }else{
          acionar_bomba = 0;                                // seta variavel para salvar no banco
          digitalWrite(BOMBA_PIN, acionar_bomba);           // Aciona bomba
        }
      }else{
        acionar_bomba = 0;                                  // seta variavel para salvar no banco
        digitalWrite(BOMBA_PIN, acionar_bomba);             // Aciona bomba
      }
      // =========================================================== 
      
      break;  
  }


  /**
  
  Envia estados dos atuadores para o Firebase
  
  */
  // ======================================================================
  if (estado_lamp != acender_lamp){
    if(Firebase.setInt(firebaseData, "/estado/atuadores/lampada", acender_lamp)){   // atualiza banco com o valor      
      estado_lamp = acender_lamp;
    }
  }
  if (estado_bomba != acionar_bomba){
    if(Firebase.setInt(firebaseData, "/estado/atuadores/bomba_agua", acionar_bomba)){ // atualiza banco com o valor      
      estado_bomba = acionar_bomba;
    }
  }
  if (estado_ventoinha != acionar_ventoinha){
    if(Firebase.setInt(firebaseData, "/estado/atuadores/ventoinha", acionar_ventoinha)){     // atualiza banco com o valor      
      estado_ventoinha = acionar_ventoinha;
    }
  }
  // ======================================================================


   /**
  
  Envia estados dos sensores para o Firebase
  
  */
  // =====================================================================
  
  if(Firebase.setInt(firebaseData, "/estado/sensores/luminosidade", ldr_lido)){ // atualiza banco com o valor      
    
  } else {
    Serial.println("Erro no envio");
  }
    
  if(Firebase.setInt(firebaseData, "/estado/sensores/nivel", nivel_lido)){      // atualiza banco com o valor      
  
  } else {
    Serial.println("Erro no envio");
  }
    
  if(Firebase.setInt(firebaseData, "/estado/sensores/temperatura", temp_lido)){ // atualiza banco com o valor      
  
  } else {
    Serial.println("Erro no envio");
  }

  if(Firebase.setInt(firebaseData, "/estado/sensores/umidade", um_lido)){      // atualiza banco com o valor      
  
  } else {
    Serial.println("Erro no envio");
  }
  // =====================================================================  

  // =====================================================================  
  // log de conexao
  while(!timeClient.update()){                                 // atualiza hora atual
    timeClient.forceUpdate();                                  // atualiza hora atual
  }      
  hora_atual = split_hora(timeClient.getFormattedTime());      // busca hora atual do serviro NTP
  
  if(Firebase.setString(firebaseData, "/ultima_conexao", hora_atual)){      // atualiza banco com o valor    
  }
  // =====================================================================  
}


String split_hora(String hora){
  String hora_minuto = "";

  for(int i = 0; i < hora.length()-3; i++){
    hora_minuto += hora.charAt(i);
  }

  Serial.println(hora_minuto);
  return hora_minuto;
}
