/*  Reator: Programa feito para o controle de um reator do tipo
 *  batelada. O programa faz a supervisão de alguns sensores e 
 *  envia sinais de controle para alguns motores e uma resistência. 
 *  
 *  Autor: Julian Jose de Brito
 *  
 *  Versão 1.3 05/10/2018: -Exclusão da referência de nível
 */

//#######################################################################################################################
// CARREGAMENTO DE BIBLIOTECAS

//carrega bibliotecas para uso do sensor de temperatura
#include <OneWire.h> // ler a biblioteca
#include <DallasTemperature.h> // ler a biblioteca

//carrega a biblioteca para configurar as interrupções
//#include <TimerThree.h>

//#######################################################################################################################
// DEFINIÇÕES DOS PINOS

// Define pinos para o pwm da bomba 1 
#define velocidade_bomba1 6 //bomba 

// Define os pinos para o sensor ultrassônico
#define trigger 4
#define echo 5

// Define o pino da resistência
#define resistencia 7

// Sinal do DS18B20 (sensor de temperatura)
#define sensor_temperatura 3

// Agitador
#define step_agitador 10
#define sentido_agitador 11

// Sensor de condutividade
#define condutivimetro A3

// Sensor de nível
#define sensor_nivel A0


//#######################################################################################################################
// CONFIGURAÇÕES INICIAIS 


// Define uma instancia do oneWire para comunicacao com o sensor de temperatura
OneWire oneWire(sensor_temperatura);

DallasTemperature sensors(&oneWire);
DeviceAddress ENDERECO_SENSOR_TEMPERATURA;

//#######################################################################################################################
// PAINEL DE CONTROLE

//Define as chaves de controle
boolean CH_LIGA_BOMBA1 = 6;
boolean CH_LIBERA_SENSOR_TEMPERATURA = 0;
boolean CH_RESISTENCIA = 0;
boolean CH_HABILITA_AGITADOR = 0;
boolean CH_SENSOR_NIVEL = 1;
boolean CH_CONDUTIVIMETRO = 0;

//#######################################################################################################################
// VARIÁVEIS DO SISTEMA

float TEMPERATURA, NIVEL, CONDUTIVIDADE;
int VELOCIDADE_AGITADOR = 50, TEMPERATURA_RESISTENCIA = 50, PWM_BOMBA1 = 0;

unsigned long time;
unsigned long TEMPO_ATUAL;

int i = 0, INDEX = 0, INDEX2 = 0;

float LEITURA_NIVEL_MEDIA, NIVEL_APROXIMADO, LEITURAS_NIVEL[20], VOLUME, VOLUMES[50], LEITURAS[50], ERROS[50], META = 0;

void setup() 
{ 
  
   // Inicializa a comunicação serial
   Serial.begin(9600);

   // Configura o sensor de temperatura
   sensors.begin();
   sensors.getAddress(ENDERECO_SENSOR_TEMPERATURA, 0);


   pinMode(velocidade_bomba1, OUTPUT);
   pinMode(resistencia, OUTPUT);

   pinMode(step_agitador, OUTPUT);
   pinMode(sentido_agitador, OUTPUT);

   pinMode(sensor_nivel, INPUT);

   for(i = 0; i<20; i++)
      LEITURAS_NIVEL[i] = 0;
   for(i = 0; i<50; i++)
   {
     VOLUMES[i] = 1;
     LEITURAS[i] = 0;
     ERROS[i] = 0;
   }

   
}

void loop() 
{
  
  //verifica se algo foi digitado no canal serial
  le_informacao();

  if(CH_LIGA_BOMBA1)
  liga_motor();

  if(CH_LIBERA_SENSOR_TEMPERATURA)
  le_sensor_temperatura();

  if(CH_RESISTENCIA)
  aciona_resistencia();

  if(CH_HABILITA_AGITADOR)
  aciona_agitador();

  if(CH_SENSOR_NIVEL)
  le_nivel();
  
  if(CH_CONDUTIVIMETRO)
  le_condutividade();
  
  exibe_dados();
}

void le_informacao()
{
  while (Serial.available() > 0) 
  {
    // Lê primeiro byte digitado pelo usuário e atua no sistema
      switch (Serial.read()) 
      {
        case 'B':
                    PWM_BOMBA1 = Serial.parseInt();
                    break;
        case 'T':
                    TEMPERATURA_RESISTENCIA = Serial.parseFloat();
                    break;
        case 'A':   
                    VELOCIDADE_AGITADOR = Serial.parseInt(); //faixa recomendada 200 - 800
                    
        case 'L':   
                    VOLUME = Serial.parseFloat();
                    VOLUMES[INDEX2] = VOLUME;
                    LEITURAS[INDEX2] = LEITURA_NIVEL_MEDIA;
                    INDEX2++;
                    break;
        case 'I':  
                    Serial.println("Volumes: Leitura: Erro:");
                    for(i=0; i<INDEX2; i++){
                      Serial.print(VOLUMES[i]);
                      Serial.print(" ");
                      Serial.print(LEITURAS[i]); 
                      Serial.println(" ");
                    }
                    break;
         
        case 'F':          
                    META = Serial.parseFloat();
                    
        default:
                    break;

      }
   }

}

void liga_motor()
{
  
  if(META > 0)
  {
    if(LEITURA_NIVEL_MEDIA < META)
    analogWrite(velocidade_bomba1, 5);
    
    else
    analogWrite(velocidade_bomba1, 0);
  }
  else
  analogWrite(velocidade_bomba1, PWM_BOMBA1);
  
}


void le_sensor_temperatura()
{
  sensors.requestTemperatures();
  TEMPERATURA = sensors.getTempC(ENDERECO_SENSOR_TEMPERATURA);  
}

void aciona_resistencia()
{
  // Verifica se a temperatura desejada é maior ou menor do que a temperatura de leitura do sensor
  
  if(TEMPERATURA > TEMPERATURA_RESISTENCIA)
  digitalWrite(resistencia, LOW);

  if(TEMPERATURA < TEMPERATURA_RESISTENCIA)
  digitalWrite(resistencia, HIGH);
}

void aciona_agitador()
{
  tone(step_agitador, VELOCIDADE_AGITADOR); 
}

void le_nivel()
{
  NIVEL = analogRead(A0);
  LEITURA_NIVEL_MEDIA = calculaMediaNivel(LEITURA_NIVEL_MEDIA);

  if(LEITURA_NIVEL_MEDIA > 60 && LEITURA_NIVEL_MEDIA < 75)
   NIVEL_APROXIMADO = 1;
  else
  {
    NIVEL_APROXIMADO = LEITURA_NIVEL_MEDIA*0,05;
  }
}

void le_condutividade()
{
  CONDUTIVIDADE = analogRead(A3);
}

float calculaMediaNivel(float leitura_media_anterior)
{
  // Calcula média das leituras do sensor de nível
  
  float total = 0, media;
  
  LEITURAS_NIVEL[INDEX] = NIVEL;
  INDEX++;

  if(INDEX > 20)
    INDEX = 0;

  for(i = 0; i<20; i++)
    total+= LEITURAS_NIVEL[i];

  media = total/20;

  //filtra o valor da média com base em uma variação de valor = 6
  if(!((media >= leitura_media_anterior+1) ||  (media <= leitura_media_anterior-1)))
    media = leitura_media_anterior;
  
  return media;
}

void exibe_dados()
{
  
  if(CH_HABILITA_AGITADOR)
  {
    Serial.print(" Velocidade do agitador: ");
    Serial.print(VELOCIDADE_AGITADOR);
  }
  
  if(CH_LIGA_BOMBA1)
  {
    Serial.print(" PWM Bomba 1: ");
    Serial.print(PWM_BOMBA1);
  }  

  if(CH_RESISTENCIA)
  {
    Serial.print(" Temperatura Resistência: ");
    Serial.print(TEMPERATURA_RESISTENCIA);
  }

  if(CH_LIBERA_SENSOR_TEMPERATURA)
  {
    Serial.print(" Temp ºC: ");
    Serial.print(TEMPERATURA);
  }

  if(CH_SENSOR_NIVEL)
  {
    Serial.print(" Nivel Médio: ");
    Serial.print(LEITURA_NIVEL_MEDIA);

//    Serial.print(" Referência: ");
//    Serial.print(NIVEL_REFERENCIA);

    Serial.print(" Nivel apoximado: ");
    Serial.print(NIVEL_APROXIMADO);
  }
  
  if(CH_CONDUTIVIMETRO)
  {
    Serial.print(" Condutividade: ");
    Serial.print(CONDUTIVIDADE);
  }

  Serial.print("\n");
}
