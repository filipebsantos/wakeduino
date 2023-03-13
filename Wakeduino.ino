/*  WakeDuino v1.0 - 28/02/2023
 *  Elaborado por Filipe Bezerra - www.filipebezerra.dev.br
 *  O objetivo deste projeto é ligar um computador remotamente, usando o PowerSW da placa-mãe e monitorando se o PC está ligado
 *  pela tensão no Power LED. Projeto feito originalmente para contornar o fato da placa X99-QD4 da Huananzhi não funcionar o Wake-on-Lan
 *  mesmo estando ativado no firmware.
 */

#include <SPI.h>
#include <Ethernet.h>

// Define as portas usadas pelo PowerSW e PowerLED
const int PowerSW = 2;
const int PowerLED = A0;

// Define os atributos da rede
byte mac[] = {0xE7, 0x78, 0x15, 0xF6, 0x8D, 0x27};
byte ip [] = {10, 1, 0, 10};
byte gateway[] = {10, 1, 0, 1};
byte subnet[] = {255, 255, 255, 0}; 

// Variável para fazer a busca pelo comando na URL
String readString = String(30);

// Servidor irá "escutar" na porta 80
EthernetServer server(80);

void setup() {
  // Define o PowerSW como saída e o PowerLED como entrada
  pinMode(PowerSW, OUTPUT);
  pinMode(PowerLED, INPUT);

  // PowerSW inicia desligado
  digitalWrite(PowerSW, LOW);

  // Inicia a comunicação serial
  Serial.begin(9600);
  Serial.println("WakeDuino - v1.0");

  // Inicia a conexão Ethernet
  Ethernet.begin(mac, ip, gateway, subnet);

  // Checa se o módulo Ethernet está disponível
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("O módulo Ethernet não está presente. Inicialização interrompida!");
    while (true) {
      delay(1); // Não faz nada sem o módulo Ethernet presente
    }
  }
  
  // Verifica se o cabo de rede está conectado  
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Cabo Ethernet não está conectado. Inicialização interrompida!");
    while (true) {
      delay(1); // Não faz nada sem o módulo Ethernet presente
    }
  }
  
  // Inicia o servidor
  server.begin();
  Serial.print("Servidor iniciado no ip ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  // Faz a leitura da tensão no pino do PowerLED
  float tensao = analogRead(PowerLED)*5.0/1023.0;
  
  // Escuta pela conexão dos clientes
  EthernetClient client = server.available();
  if (client) {
    Serial.print("Conexao recebida do ip ");
    Serial.println(client.remoteIP());
    while (client.connected()){
      if (client.available()){
        char c = client.read();
        Serial.write(c);

        // Salva a requisição na variável readString
        if (readString.length() < 100) {
          readString += c;
        }

        if (c == '\n') {
          if (readString.indexOf("?") < 0) { // Busca pelo inicio dos comandos
            } else if (readString.indexOf("start") > 0){ // Comando de ligar
              if (tensao < 2.00) { // Verifica se tensão do Power LED é menor que 2 volts, o que indica que o computado está desligado.
                Serial.print("Tensão do PowerLED: ");
                Serial.println(tensao);
                Serial.println("Ligando o computador...");
                digitalWrite(PowerSW, HIGH);
                delay(250);
                digitalWrite(PowerSW, LOW); 
              } else {
                Serial.print("Tensão do PowerLED: ");
                Serial.println(tensao);
                Serial.print("O computador ja esta ligado.");
              }
            } else if (readString.indexOf("stop") > 0) { //Comando de desligar
              if (tensao > 2.00) {
                Serial.print("Tensão do PowerLED: ");
                Serial.println(tensao);
                Serial.println("Desligando o computador...");
                digitalWrite(PowerSW, HIGH);
                delay(250);
                digitalWrite(PowerSW, LOW); 
              } else {
                Serial.print("Tensão do PowerLED: ");
                Serial.println(tensao);
                Serial.println("O computador ja esta desligado.");
              }
             }
           // Envia resposta padrão
           client.println("HTTP/1.1 200 OK");
           client.println("Content-Type: text/html");
           client.println("Connection: close");
           client.println();
           client.println("<!DOCTYPE HTML>");
           client.println("<html>");
           client.println("<head>");
           client.println("<title>WakeDuino v1.0</title>");
           client.println("</head>");
           client.println("<body style=background-color:#C9EAF5>");
           client.println("<center><h1>WakeDuino v1.0</h1></center>");
           client.println("<br><br>");
           client.println("<center><h3>Uso: http://ip-do-wakeduino/?start - Liga o computador</h3></center>");
           client.println("<center><h3>http://ip-do-wakeduino/?stop - Desliga o computador</h3></center>");
           client.println("</body>");
           client.println("</html>");

            // Reinicia a varável readString
            readString = "";

            delay(1);

            // Fecha a conexão
            client.stop();
            Serial.println("Cliente desconectado");
          } // Fim do if (c == '\n')
        } // Fim do if (client.available()) 
      } // Fim do while (client.connected())
    } // Fim do if (client)
}
