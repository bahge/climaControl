#include <SD.h>
#include <SPI.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;                                                    // Cria o objeto bme
float temperature, humidity, pressure, altitude;                        // Define as variáveis de temperatura, humidade, pressão e altitude
bool bmeOk = true;                                                      // Inicia a variável do bme

const int sensor_ldr = 2;                                               // D4 - Sensor LDR

/*Dados da conexão*/
const char* ssid = "climaControl";                                      // Nome da rede criada
const char* password = "UFMT2020";                                      // Senha da wifi

const int chipSelect = 15;                                              // D8 - Data SD
File dataFile;                                                          // Objeto responsável por escrever/Ler do cartão SD
bool cartaoOk = true;                                                   // Inicia a variavel do cartão

ESP8266WebServer server(80);              
 
void setup() {
  Serial.begin(9600);                                                   // Inicialização da porta de comunicação Serial
  delay(100);
  Serial.println("\n Verificando cartão");
  /* Verificação do cartão */
  if (!SD.begin(chipSelect)) {
    Serial.println("Erro na leitura do arquivo não existe um cartão SD ou o módulo está conectado corretamente ?");
    cartaoOk = false;
  }
  /* Verificação do bme */
  if (!bme.begin(0x76)){
    Serial.println("Erro no sensor bme");
    bmeOk = false; 
  }
  /* Abrindo conexão */
  WiFi.mode(WIFI_AP); 
  WiFi.softAP(ssid, password);                                          // Criando a conexão com a senha da wifi
  IPAddress myIP = WiFi.softAPIP();                                     // Pegando o IP

  Serial.println(myIP);

  server.on("/", leitura);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("Servidor inicializado");
  
}

void loop() {
  server.handleClient();
}

void leitura() {
  if (bmeOk) {
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0F;
    altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  }

  if (cartaoOk){
      dataFile = SD.open("datalog.csv", FILE_WRITE); 
      Serial.println("Cartão SD Inicializado para escrita");
  }

  int luz = digitalRead(sensor_ldr);
  String stLuz = "";
  if (luz == HIGH) {
    stLuz = "Noite";
  } else {
    stLuz = "Dia";
  }

  String leitura = "";
  if ( bmeOk == true && cartaoOk == true ) {
    leitura = String(millis()) + ";" + String(temperature) + ";" + String(humidity) + ";" + String(pressure) + ";" + String(altitude) + ";" + stLuz + ";";
  }

  if (dataFile) {   
    Serial.println(leitura);    // Mostramos no monitor a linha que será escrita
    dataFile.println(leitura);  // Escrevemos no arquivos e pulamos uma linha
    dataFile.close();           // Fechamos o arquivo
  }
   
  server.send(200, "text/html", SendHTML(temperature,humidity,pressure,altitude,stLuz));
  
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}


String SendHTML(float temperature,float humidity,float pressure,float altitude, String ilumicacao){
  String ptr = "<!DOCTYPE html>\n<html lang=\"pt-br\">\n<head>\n<meta charset=\"UTF-8\">\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
  ptr += "<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">\n<meta http-equiv=\"refresh\" content=\"30\"/>\n<title>ClimaControl</title>\n</head>";
  ptr += "<body>\n<svg height=\"150\" width=\"100%\" viewBox=\"0 0 128 128\" style=\"vertical-align:middle;\">\n<g transform=\"translate(0,-220)\">\n";
  ptr += "<g transform=\"matrix(0.4906917,0,0,0.4906917,375.51248,375.39467)\">\n";
  ptr += "<g style=\"fill:none;stroke:#002255;stroke-width:1;stroke-linecap:round;stroke-linejoin:miter;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\" transform=\"matrix(0.33163069,-2.0234131,-2.0234131,-0.33163069,33.916394,-169.2153)\">\n";
  ptr += "<path style=\"fill:none;fill-opacity:1;stroke:#002255;stroke-width:1.12313223;stroke-linecap:butt;stroke-linejoin:round;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"\n";
  ptr += "d=\"m -1.8224955,284.30645 c -1.9874079,-2.82343 -3.8533912,2.43975 -4.9289249,0.372 -2.5329779,-4.86975 -4.9056586,-1.11861 -4.9056586,-1.11861 0,0 2.7133599,-3.30013 4.5394754,-1.11149 1.588043,1.90331 3.3611731,-3.47881 6.021513,0.18296 1.83518699,2.29399 3.9163477,0.39647 3.9163477,0.39647 l 0.090566,-0.62736 4.273124,1.59394 -2.7946285,2.27259 c -0.182023,-0.24879 -0.3686418,-0.73705 -0.5932623,-1.04408 0,0 -4.02471751,1.55136 -5.6185521,-0.91641 z m 2.26197643,6.15023 c -1.98740793,-2.82343 -3.85339123,2.43975 -4.92892493,0.372 -2.5329779,-4.86975 -4.9056584,-1.11861 -4.9056584,-1.11861 0,0 2.7133597,-3.30013 4.5394752,-1.11149 1.588043,1.90331 3.3611731,-3.47881 6.021513,0.18296 1.835187,2.29399 3.9163477,0.39647 3.9163477,0.39647 l 0.090566,-0.62736 4.273124,1.59394 -2.7946285,2.27259 c -0.182023,-0.24879 -0.3686418,-0.73705 -0.5932623,-1.04408 0,0 -4.0247175,1.55136 -5.61855207,-0.91641 z m -16.57919993,7.51401 0.570472,-3.48068 m 3.760908,4.19058 0.570471,-3.48068 m 3.760892,4.19058 0.5704714,-3.48068 m 3.7609078,4.19057 0.5704714,-3.48067 m 20.6513564,9.15892 -5.474372,0.6437 z m -2.459564,9.055 -4.31621,-3.42881 z m -3.075131,-6.62317 c -0.455352,2.937 -2.8135572,5.20978 -5.7653692,5.55654 -3.6284475,0.42607 -6.9153409,-2.16987 -7.34157491,-5.79831 -0.11836914,-1.0155 2.7164e-4,-2.04462 0.34659683,-3.00655 -0.31067842,0.0884 -0.62680807,0.11655 -0.93749292,0.0656 l -23.6227828,-3.8717 c -2.300165,2.80948 -6.200137,3.72373 -9.509218,2.22909 -3.309077,-1.49466 -5.202197,-5.02569 -4.614927,-8.60886 0.235654,-1.43782 0.852191,-2.73609 1.73327,-3.80348 -13.487083,-6.32199 -12.345813,-18.25494 -2.012654,-23.42523 -2.215776,4.4677 -1.400167,9.8456 2.040167,13.45575 3.440401,3.61012 8.770949,4.6828 13.340076,2.68463 4.569098,-1.9982 7.399806,-6.64123 7.084847,-11.61822 2.083569,3.0315 2.43409,6.04407 1.953082,8.66075 -0.618544,3.36429 -2.609906,6.07358 -4.055621,7.32558 -1.643198,1.42295 -3.61119,2.54249 -5.698719,3.27874 0.421933,0.57576 0.775599,1.21142 1.03728,1.90417 l 23.62278298,3.8717 c 1.65711772,0.27159 2.74873962,2.6898 2.52916952,4.37689 0.7858269,-0.46013 1.6581954,-0.75309 2.562472,-0.86048 3.6284474,-0.42607 6.9153405,2.16987 7.3415905,5.79831 0.06973,0.59377 0.05865,1.19426 -0.03294,1.78505 z m 1.024322,-10.53035 -3.429639,4.31609 z M 7.4367577,319.38947 6.7940156,313.91443 Z M 5.2151749,300.47883 4.5714903,295.00448 Z m -3.3431614,11.91893 -3.4288158,4.31622 z m -2.46481691,-4.426 -5.47517989,0.64356 z\"/>\n";
  ptr += "</g>\n<g style=\"font-style:normal;font-weight:normal;font-size:32px;line-height:1.25;font-family:sans-serif;letter-spacing:0px;word-spacing:0px;fill:#000000;fill-opacity:1;stroke:none\" transform=\"matrix(2.7866521,0,0,3.7219883,778.20338,86.982004)\" aria-label=\"CLIMA \">\n";
  ptr += "<path style=\"font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-family:'Yanone Kaffeesatz';-inkscape-font-specification:'Yanone Kaffeesatz';fill:#d4aa00;fill-opacity:1\"\n";
  ptr += "d=\"m -544.84342,-96.78006 c -2.88,0 -6.304,1.6 -6.304,12.544 0,9.312 2.272,10.688 5.312,10.688 2.368,0 3.968,-1.056 3.968,-2.304 0,-0.544 -0.192,-1.152 -0.416,-1.568 -0.576,0.48 -1.696,0.992 -2.944,0.992 -1.856,0 -3.2,-1.44 -3.2,-7.968 0,-8.416 2.08,-9.856 4.096,-9.856 0.992,0 1.76,0.512 2.08,0.8 0.192,-0.384 0.416,-0.96 0.416,-1.568 0,-0.832 -0.64,-1.76 -3.008,-1.76 z\"/>\n";
  ptr += "<path style=\"font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-family:'Yanone Kaffeesatz';-inkscape-font-specification:'Yanone Kaffeesatz';fill:#d4aa00;fill-opacity:1\"\n";
  ptr += "d=\"m -537.22748,-76.39606 v -19.968 c -2.56,0 -2.624,0 -2.624,0.736 v 21.664 h 7.616 c 0.288,0 0.512,-0.096 0.512,-0.544 v -1.888 z\"/>\n";
  ptr += "<path style=\"font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-family:'Yanone Kaffeesatz';-inkscape-font-specification:'Yanone Kaffeesatz';fill:#d4aa00;fill-opacity:1\"\n";
  ptr += "d=\"m -529.8035,-95.18006 v 21.216 c 2.624,0 2.656,0 2.656,-0.544 v -21.856 c -2.592,0 -2.656,0 -2.656,1.184 z\"/>\n";
  ptr += "<path style=\"font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-family:'Yanone Kaffeesatz';-inkscape-font-specification:'Yanone Kaffeesatz';fill:#d4aa00;fill-opacity:1\"\n";
  ptr += "d=\"m -513.64355,-96.36406 c -0.32,0 -0.416,0.224 -0.512,0.512 l -2.816,9.248 c -0.384,1.216 -0.928,3.456 -0.928,3.456 0,0 -0.608,-2.24 -1.024,-3.552 l -3.04,-9.664 h -1.888 c -0.288,0 -0.544,0.032 -0.544,0.512 v 21.888 c 2.432,0 2.592,0 2.592,-0.544 l -0.064,-7.36 c -0.032,-3.424 -0.608,-9.344 -0.608,-9.344 l 0.032,-0.032 c 0.672,2.464 1.792,6.528 2.048,7.232 l 1.664,5.12 h 1.12 c 0.128,0 0.288,-0.032 0.352,-0.256 l 1.696,-4.864 c 0.256,-0.768 1.408,-4.896 2.08,-7.296 -0.064,0.736 -0.512,6.4 -0.608,9.568 l -0.096,7.776 c 2.656,0 2.656,0 2.656,-0.544 v -21.856 z\"/>\n";
  ptr += "<path style=\"font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-family:'Yanone Kaffeesatz';-inkscape-font-specification:'Yanone Kaffeesatz';fill:#d4aa00;fill-opacity:1\"\n";
  ptr += "d=\"m -496.3316,-74.44406 -5.728,-21.92 c -0.288,-0.064 -0.768,-0.096 -1.216,-0.096 -0.576,0 -0.928,0.128 -0.992,0.448 l -5.696,22.016 c 0.608,0.064 1.28,0.096 1.632,0.096 1.024,0 1.12,-0.16 1.216,-0.512 l 1.216,-5.248 h 5.536 l 1.376,5.696 c 0.64,0.064 1.152,0.064 1.536,0.064 1.024,0 1.216,-0.192 1.12,-0.544 z m -8.992,-7.584 1.088,-4.544 c 0.48,-2.144 0.928,-4.608 1.12,-6.144 0.16,1.536 0.608,4.032 1.12,6.144 l 1.088,4.544 z\"/>\n";
  ptr += "</g>\n";
  ptr += "<g style=\"font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-size:7.36837196px;line-height:1.25;font-family:'Yanone Kaffeesatz';-inkscape-font-specification:'Yanone Kaffeesatz';letter-spacing:0px;word-spacing:0px;fill:#d4aa00;fill-opacity:1;stroke:none;stroke-width:0.3947342\" transform=\"matrix(2.7197768,0,0,7.0521134,97.510394,-1344.256)\" aria-label=\"ONTROL\">\n";
  ptr += "<path\n";
  ptr += "style=\"font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-family:'Yanone Kaffeesatz';-inkscape-font-specification:'Yanone Kaffeesatz';fill:#d4aa00;fill-opacity:1;stroke-width:0.3947342\"\n";
  ptr += "d=\"m -224.68145,162.66284 c 0,2.12209 0.37579,2.67472 1.33368,2.67472 0.96526,0 1.38525,-0.67053 1.38525,-2.69683 0,-1.98946 -0.35368,-2.59367 -1.3042,-2.59367 -0.95789,0 -1.41473,0.65579 -1.41473,2.61578 z m 0.67053,0.14736 c 0,-1.59156 0.20631,-2.15156 0.7221,-2.15156 0.53052,0 0.67789,0.52315 0.67789,1.99683 0,1.50315 -0.18421,2.07051 -0.71473,2.07051 -0.52316,0 -0.68526,-0.54526 -0.68526,-1.91578 z\"/>\n";
  ptr += "<path\n";
  ptr += "style=\"font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-family:'Yanone Kaffeesatz';-inkscape-font-specification:'Yanone Kaffeesatz';fill:#d4aa00;fill-opacity:1;stroke-width:0.3947342\"\n";
  ptr += "d=\"m -219.38359,160.23127 v 1.73894 c 0,0.34631 0.0147,1.7463 0.0663,2.18841 v 0.0221 c -0.17684,-0.55263 -0.55263,-1.69472 -0.69999,-2.07788 l -0.82526,-1.98946 h -0.45684 c -0.0884,0 -0.12526,0.0368 -0.12526,0.11789 v 5.03997 c 0.59684,0 0.6042,0 0.6042,-0.12526 v -1.78315 c 0,-0.31684 -0.0368,-1.7242 -0.0811,-2.20314 0.18421,0.5821 0.56737,1.75367 0.7,2.08525 l 0.83999,2.0263 h 0.44947 c 0.0811,0 0.12526,-0.0442 0.12526,-0.12526 v -5.0326 c -0.58947,0 -0.59683,0 -0.59683,0.11789 z\"/>\n";
  ptr += "<path\n";
  ptr += "style=\"font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-family:'Yanone Kaffeesatz';-inkscape-font-specification:'Yanone Kaffeesatz';fill:#d4aa00;fill-opacity:1;stroke-width:0.3947342\"\n";
  ptr += "d=\"m -218.28572,160.11338 c -0.0884,0 -0.12526,0.0368 -0.12526,0.11789 v 0.44211 h 1.0021 v 4.59786 c 0.58947,0 0.59684,0 0.59684,-0.11789 v -4.47997 h 0.90631 c 0.0884,0 0.12526,-0.0295 0.12526,-0.12527 v -0.43473 z\"/>\n";
  ptr += "<path\n";
  ptr += "style=\"font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-family:'Yanone Kaffeesatz';-inkscape-font-specification:'Yanone Kaffeesatz';fill:#d4aa00;fill-opacity:1;stroke-width:0.3947342\"\n";
  ptr += "d=\"m -213.1426,165.13124 c -0.0737,-1.12736 -0.45684,-1.80525 -0.69263,-2.12209 0.38316,-0.21368 0.64105,-0.65579 0.64105,-1.43683 0,-1.02421 -0.42736,-1.45894 -1.35578,-1.45894 h -0.37579 -0.35368 c -0.12526,0 -0.12526,0.11789 -0.12526,0.11789 v 5.03997 c 0.58947,0 0.60421,0 0.60421,-0.12526 v -1.96736 h 0.25789 c 0.0442,0 0.0958,0 0.14,-0.007 0.17684,0.24316 0.53052,0.85474 0.6042,2.09999 0.13263,0.007 0.2579,0.0147 0.35369,0.0147 0.27999,0 0.3021,-0.0368 0.3021,-0.15474 z m -1.65788,-2.50525 v -1.98946 h 0.36105 c 0.43473,0 0.5821,0.33895 0.5821,0.90631 0,0.78105 -0.21369,1.08315 -0.62631,1.08315 z\"/>\n";
  ptr += "<path\n";
  ptr += "style=\"font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-family:'Yanone Kaffeesatz';-inkscape-font-specification:'Yanone Kaffeesatz';fill:#d4aa00;fill-opacity:1;stroke-width:0.3947342\"\n";
  ptr += "d=\"m -212.75945,162.66284 c 0,2.12209 0.37579,2.67472 1.33368,2.67472 0.96525,0 1.38525,-0.67053 1.38525,-2.69683 0,-1.98946 -0.35368,-2.59367 -1.3042,-2.59367 -0.95789,0 -1.41473,0.65579 -1.41473,2.61578 z m 0.67052,0.14736 c 0,-1.59156 0.20632,-2.15156 0.72211,-2.15156 0.53052,0 0.67789,0.52315 0.67789,1.99683 0,1.50315 -0.18421,2.07051 -0.71474,2.07051 -0.52315,0 -0.68526,-0.54526 -0.68526,-1.91578 z\"/>\n";
  ptr += "<path\n";
  ptr += "style=\"font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-family:'Yanone Kaffeesatz';-inkscape-font-specification:'Yanone Kaffeesatz';fill:#d4aa00;fill-opacity:1;stroke-width:0.3947342\"\n";
  ptr += "d=\"m -208.89843,164.71124 v -4.59786 c -0.58947,0 -0.6042,0 -0.6042,0.16947 v 4.98839 h 1.75367 c 0.0663,0 0.11789,-0.0221 0.11789,-0.12526 v -0.43474 z\"/>\n";
  ptr += "</g>\n";
  ptr += "</g>\n";
  ptr += "</g>\n";
  ptr += "</svg>\n";
  ptr += "<svg height=\"64\" width=\"64\" viewBox=\"0 0 32 32\" style=\"vertical-align:middle;\">\n";
  ptr += "<g transform=\"translate(0,-265)\">\n";
  ptr += "<path\n";
  ptr += "style=\"fill:none;stroke:#12ad54;stroke-width:1;stroke-linecap:round;stroke-linejoin:miter;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"\n";
  ptr += "d=\"m 18.128261,272.11793 h -2.267857 m 2.267857,2.64585 h -2.267857 m 2.267857,2.64585 h -2.267857 m 2.267857,2.64584 h -2.267857 m 2.639685,4.739 c 1.959839,1.13151 2.915245,3.4383 2.329531,5.62422 -0.585717,2.18592 -2.566603,3.7059 -4.82963,3.7059 -2.263027,0 -4.243911,-1.51998 -4.829628,-3.7059 -0.585714,-2.18592 0.369888,-4.49271 2.329727,-5.62422 0,0 0,0 0,-14.91906 0,-1.06719 1.432813,-2 2.5,-2 1.067187,0 2.5,0.93281 2.5,2 0,14.91906 0,14.91906 0,14.91906 z\"/>\n";
  ptr += "</g>\n";
  ptr += "</svg>Temperatura: <span>";
  ptr += temperature;
  ptr += " &deg;C</span><br>";
  ptr += "<svg height=\"64\" width=\"64\" viewBox=\"0 0 32 32\" style=\"vertical-align:middle;\">\n";
  ptr += "<g transform=\"translate(0,-265)\">\n";
  ptr += "<g transform=\"translate(48.818413,17.37252)\">\n";
  ptr += "<path\n";
  ptr += "d=\"m -31.500425,265.62792 c -2.29457,-3.79619 2.95277,-8.17677 4.82122,-12.45791 0.35763,-0.81942 7.38059,8.87371 4.88704,12.59787 -1.83468,2.74012 -7.04796,4.26129 -9.70826,-0.13996 z\"\n";
  ptr += "style=\"fill:#afdde9;stroke:#afdde9;stroke-width:1;stroke-linecap:round;stroke-linejoin:round;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"/>\n";
  ptr += "<path\n";
  ptr += "style=\"fill:#aaffcc;fill-opacity:1;stroke:#aaffcc;stroke-width:1;stroke-linecap:round;stroke-linejoin:round;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"\n";
  ptr += "d=\"m -35.395937,275.83102 c -4.616415,-1.4442 -8.041451,-5.52824 -8.819927,-10.51696 -0.778475,-4.98872 1.226346,-10.00586 5.162683,-12.91982 l -1.256936,-1.51992 c 8.140069,0.70345 8.23905,0.3021 4.91369,8.22098 l -1.221341,-1.94823 c -3.356211,2.2111 -5.065568,6.01809 -4.401821,9.80351 0.663745,3.78542 3.584009,6.88438 7.520071,7.98023 3.936061,1.09585 8.196571,-0.004 10.971383,-2.83195 -3.254443,3.72706 -8.251387,5.17636 -12.867802,3.73216 z\"/>\n";
  ptr += "</g>\n";
  ptr += "</g>\n";
  ptr += "</svg>Humidade: <span>";
  ptr +=humidity;
  ptr += " %</span><br>";
  ptr += "<svg height=\"64\" width=\"64\" viewBox=\"0 0 32 32\" style=\"vertical-align:middle;\">\n";
  ptr += "<g transform=\"translate(0,-265)\">\n";
  ptr += "<rect style=\"opacity:1;fill:#b7c4c8;fill-opacity:1;stroke:#000000;stroke-width:1;stroke-linecap:round;stroke-linejoin:round;stroke-miterlimit:4;stroke-dasharray:none;stroke-dashoffset:0;stroke-opacity:1\" width=\"6.7090774\" height=\"28.348215\" x=\"19.204824\" y=\"266.8259\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.3px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,274.73854 h 4\"/>\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.3px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,281 h 4\"/>\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.3px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,273.42994 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.3px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,279.73595 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,272.14361 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,278.44963 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\" \n";
  ptr += "d=\"m 19.204826,270.85729 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,277.1633 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,269.57096 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,275.87698 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,287.60179 h 4\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,293.86325 h 4\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,286.29319 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,292.59921 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\" \n";
  ptr += "d=\"m 19.204826,285.00686 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,291.31288 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,283.72054 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,290.02656 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,282.43421 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,288.74023 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.3px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,268.13675 h 4\" />\n";
  ptr += "<text>\n";
  ptr += "<tspan x=\"5.7866716\" y=\"285.28571\"\n";
  ptr += "style=\"font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-size:14.11111069px;font-family:Dyuthi;-inkscape-font-specification:Dyuthi;fill:#2ca089;\">H</tspan></text>\n";
  ptr += "<path style=\"fill:none;stroke:#000000;stroke-width:3.77952761;stroke-linecap:round;stroke-linejoin:miter;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"\n";
  ptr += "d=\"M 23.029297 6.9003906 L 57.044922 6.9003906 L 23.029297 6.9003906 z M 40.037109 8.4003906 L 40.037109 34.857422 L 40.037109 8.4003906 z \"\n";
  ptr += "transform=\"matrix(0.26458333,0,0,0.26458333,0,265)\" />\n";
  ptr += "<path style=\"fill:none;stroke:#000000;stroke-width:3.77952761;stroke-linecap:round;stroke-linejoin:miter;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"\n";
  ptr += "d=\"M 40.037109 86.087891 L 40.037109 112.54297 L 40.037109 86.087891 z M 23.029297 114.04297 L 57.044922 114.04297 L 23.029297 114.04297 z \"\n";
  ptr += "transform=\"matrix(0.26458333,0,0,0.26458333,0,265)\" />\n";
  ptr += "</g>\n";
  ptr += "</svg>Altitude: <span>";
  ptr +=altitude;
  ptr +=" m </span><br>"; 
  ptr += "<svg height=\"64\" width=\"64\" viewBox=\"0 0 32 32\" style=\"vertical-align:middle;\">\n";
  ptr += "<g transform=\"translate(0,-265)\">\n";
  ptr += "<rect style=\"opacity:1;fill:#b7c4c8;fill-opacity:1;stroke:#000000;stroke-width:1;stroke-linecap:round;stroke-linejoin:round;stroke-miterlimit:4;stroke-dasharray:none;stroke-dashoffset:0;stroke-opacity:1\" width=\"6.7090774\" height=\"28.348215\" x=\"19.204824\" y=\"266.8259\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.3px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,274.73854 h 4\"/>\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.3px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,281 h 4\"/>\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.3px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,273.42994 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.3px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,279.73595 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,272.14361 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,278.44963 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\" \n";
  ptr += "d=\"m 19.204826,270.85729 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,277.1633 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,269.57096 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,275.87698 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,287.60179 h 4\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,293.86325 h 4\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,286.29319 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,292.59921 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\" \n";
  ptr += "d=\"m 19.204826,285.00686 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,291.31288 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,283.72054 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,290.02656 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,282.43421 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,288.74023 h 3\" />\n";
  ptr += "<path style=\"fill:#b7c4c8;stroke:#000000;stroke-width:0.3px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\"\n";
  ptr += "d=\"m 19.204826,268.13675 h 4\" />\n";
  ptr += "<path\n";
  ptr += "d=\"m 12.453515,286.77004 c -2.097129,-1.7832 0.504749,-3.73827 0.504749,-3.73827 0,0 2.424318,-2.05787 0.246804,-4.02986 -2.177503,-1.97198 0.618801,-4.09901 0.618801,-4.09901 0.05746,0.0585 -0.432183,-0.43973 -0.648257,-0.6596 l 3.332547,-0.47662 -0.806686,2.93624 -0.705023,-0.88154 c 0,0 -2.561281,1.16093 -0.594315,3.34298 1.966955,2.18205 -0.653006,3.98426 -0.653006,3.98426 0,0 -2.786234,1.68197 -0.880139,3.66807 -0.415475,-0.0466 0.32689,0.64381 -0.415475,-0.0466 z\"\n";
  ptr += "style=\"fill:#0088aa;stroke:none;stroke-width:0.2;stroke-linecap:butt;stroke-linejoin:round;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"\n";
  ptr += "/>\n";
  ptr += "<path\n";
  ptr += "style=\"fill:#006680;stroke:none;stroke-width:0.2;stroke-linecap:butt;stroke-linejoin:round;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"\n";
  ptr += "d=\"m 6.7320441,289.39775 c -2.330151,-1.98133 0.560818,-4.15363 0.560818,-4.15363 0,0 2.6936865,-2.28653 0.274236,-4.47762 -2.419451,-2.1911 0.687475,-4.55447 0.687475,-4.55447 0.06385,0.065 -0.480198,-0.48859 -0.720287,-0.73289 l 3.7028279,-0.52958 -0.896323,3.26249 -0.7833514,-0.9795 c 0,0 -2.8458725,1.28993 -0.6603625,3.71443 2.1855009,2.4245 -0.725562,4.42695 -0.725562,4.42695 0,0 -3.095817,1.86886 -0.977935,4.07564\"\n";
  ptr += "/>\n";
  ptr += "</g>\n";
  ptr += "</svg> Pressão: <span>";
  ptr +=pressure;
  ptr +=" hPa</span><br>";
  ptr += "<svg height=\"64\" width=\"64\" viewBox=\"0 0 32 32\" style=\"vertical-align:middle;\">\n";
  ptr += "<g transform=\"translate(0,-265)\">\n";
  ptr += "<path\n";
  ptr += "style=\"fill:#ffcc00;fill-opacity:1;stroke:#ffcc00;stroke-width:0.68716764;stroke-linecap:round;stroke-linejoin:miter;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"\n";
  ptr += "d=\"m 13.746494,282.51036 -1.674112,-3.0004 M 7.9641951,272.14717 6.29008,269.14678 m 0.1129543,8.69898 -3.0003879,1.67411 m 13.2312806,-7.3826 -3.000388,1.67411 m 3.73954,4.12736 -3.302595,-0.94755 M 5.9660881,274.66595 2.6634937,273.7184 m 6.2130614,6.08956 -0.9475488,3.30259 m 4.1785637,-14.56397 -0.947552,3.3026 m 2.821382,5.11645 a 4.1230059,4.1230059 0 0 1 -5.1001731,2.82605 4.1230059,4.1230059 0 0 1 -2.8260539,-5.10017 4.1230059,4.1230059 0 0 1 5.100176,-2.82606 4.1230059,4.1230059 0 0 1 2.826051,5.10018 z\" />\n";
  ptr += "<g transform=\"rotate(-38.847604,26.165939,280.79219)\">\n";
  ptr += "<g transform=\"matrix(0.08070356,0.55841388,-0.55841388,0.08070356,177.67864,255.26652)\">\n";
  ptr += "<path\n";
  ptr += "style=\"opacity:1;fill:#373748;fill-opacity:1;stroke:none;stroke-width:1;stroke-linecap:round;stroke-linejoin:round;stroke-miterlimit:4;stroke-dasharray:none;stroke-dashoffset:0;stroke-opacity:1\"\n";
  ptr += "d=\"m 13.156958,286.82442 c -3.3120937,-0.58401 -5.9052665,-3.17719 -6.4892765,-6.48928 -0.584012,-3.31209 0.9658557,-6.6358 3.8784615,-8.31739 -5.669666,-0.0871 -7.902183,4.87646 -8.1923931,7.15097 -0.4306698,3.37535 0.7036073,7.12484 3.0350726,9.4247 3.257507,3.3582 6.1168795,3.91738 8.8342195,3.42947 3.234301,-0.58074 8.136919,-3.38993 7.251305,-9.07694 -1.681591,2.91261 -5.005294,4.46248 -8.317389,3.87847 z\" />\n";
  ptr += "<path\n";
  ptr += "style=\"opacity:1;fill:#ccaaff;fill-opacity:1;stroke:none;stroke-width:0.5;stroke-linecap:round;stroke-linejoin:round;stroke-miterlimit:4;stroke-dasharray:none;stroke-dashoffset:0;stroke-opacity:1\"\n";
  ptr += "d=\"m 14.631371,282.17656 c -0.527102,0.27453 -1.330041,-1.31697 -1.917841,-1.40469 -0.587801,-0.0877 -1.820425,1.19999 -2.244405,0.78353 -0.423979,-0.41647 0.841508,-1.67191 0.743296,-2.25805 -0.09821,-0.58614 -1.70381,-1.36051 -1.4387416,-1.89243 0.2650686,-0.53193 1.8501206,0.28367 2.3772226,0.009 0.527102,-0.27454 0.767413,-2.04084 1.355214,-1.95312 0.5878,0.0877 0.30193,1.84723 0.725909,2.2637 0.423979,0.41647 2.178097,0.0992 2.276309,0.68533 0.09821,0.58614 -1.663518,0.85798 -1.928586,1.38991 -0.265069,0.53192 0.578725,2.10214 0.05162,2.37668 z\"\n";
  ptr += "/>\n";
  ptr += "<path\n";
  ptr += "style=\"opacity:1;fill:#e3d7f4;fill-opacity:1;stroke:none;stroke-width:0.5;stroke-linecap:round;stroke-linejoin:round;stroke-miterlimit:4;stroke-dasharray:none;stroke-dashoffset:0;stroke-opacity:1\"\n";
  ptr += "d=\"m 9.1987572,295.17769 c -0.1976633,0.30747 -1.1121729,-0.29727 -1.4528157,-0.1647 -0.3406429,0.13257 -0.6057854,1.1964 -0.9592939,1.10343 -0.3535085,-0.093 -0.060966,-1.1496 -0.292312,-1.4326 -0.2313459,-0.28301 -1.3250431,-0.20643 -1.3458602,-0.57137 -0.020817,-0.36494 1.0744937,-0.41323 1.272157,-0.7207 0.1976632,-0.30748 -0.2131363,-1.32398 0.1275066,-1.45655 0.3406428,-0.13257 0.7250397,0.89421 1.0785482,0.98718 0.3535084,0.093 1.1933176,-0.61184 1.4246635,-0.32883 0.2313459,0.283 -0.6263945,0.96588 -0.6055775,1.33081 0.020817,0.36494 0.9506472,0.94585 0.752984,1.25333 z\" />\n";
  ptr += "</g>\n";
  ptr += "</g>\n";
  ptr += "<path\n";
  ptr += "style=\"fill:#d7d7f4;stroke:#a56060;stroke-width:3;stroke-linecap:round;stroke-linejoin:miter;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:0\"\n";
  ptr += "d=\"m 9.9995196,290.61652 c 3.6808054,1.89212 7.8441824,-11.70344 16.9333344,-16.99948\"/>\n";
  ptr += "</g>\n";
  ptr += "</svg>";
  ptr += ilumicacao;
  ptr += "<br>\n";
  ptr += "<div style=\"width: 100%; height: 20px; border: 1px solid #000; border-radius: 10px;text-align: center;\">\n";
  ptr += "<span>Atualização a cada 5 segundos</span>\n";
  ptr += "</div>\n";
  ptr += "<div style=\"width: 100%; text-align: center; position: fixed; bottom: 20px;margin-bottom:-10px;\"><small>CLIMACONTROL - 2020 - UFMT -IC</small></div>\n";
  ptr += "<script>";
  ptr += "setTimeout(function(){ window.location.reload(1);}, 5000);\n";
  ptr += "</script>\n</body>\n</html>";

  return ptr;
}

void json(){
  String ptr = "";
  dataFile = SD.open("datalog.csv");
  if (dataFile) {
    Serial.println("datalog.csv:\n");
    
    while (dataFile.available()) {
      Serial.write(dataFile.read());
      ptr += dataFile.read();
    }
    // close the file:
    dataFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("Erro ao abrir o arquivo.");
  }
  server.send(200, "text/html", ptr);
}