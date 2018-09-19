/*  Reator: Programa feito para o controle de um reator do tipo
 *  batelada. O programa faz a supervisão de alguns sensores e 
 *  envia sinais de controle para alguns motores e uma resistência. 
 *  
 *  Autor: Julian Jose de Brito
 *  
 *  Versão 0.1 15/03/2018: -Controle PWM do motore DC (bomba).
 *                         -Implementação dos sensores ultrassônicos.
 *                         
 *  Versão 0.3 19/03/2018: -Implementação do sensor de temperatura
 *                         incluindo a exibição do sinal. 
 *                        
 *  Versão 0.4 20/03/2018: -Controle PWM da resistência                     
 *                         -Modificação no nome de algumas variávies.
 *           
 *  Versão 0.5 20/03/2018: -Correções no controle do módulo de acionamento
 *                          do motor de passo.
 *                         -Modificações no nome de variáveis para facilitar 
 *                          a legibilidade do código.
 *         
 *  Versão 0.6 20/03/2018: -Controle simples da temperatura, usando a leitura do sensor       
 *                          de temperatura
 *                         -Melhora na interface com o usuário via Serial. 
 * 
 *  Versão 0.7 26/03/2018: -Implementação da medida indireta do volume (Ainda não funcionou... mas funcionará!)                        
 *  
 *  
 *  Verção 0.8 27/03/2018: -Função de exibição de dados.
 *         
 *         
 *  Versão 0.9 16/08/2018: -Impressão de Tempo/Temperatura                       
 *                             
 *  Versão 1   11/09/2018: -Remoção de partes que não deram certo, como as relacionadas ao sendo ultrassonico.                       
 *                         -Melhoria na legibilidade do código
 *                         -Acrescentado um código simples para leitura da condutividade 
 *                         -Leitura do sensor de nível 
 *                         -Modificação na exibição de dados
 *                         
 *  Versão 1.1 18/09/2018: -Ajuste do código                        
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
#define sensor_nivel_referencia A1


//#######################################################################################################################
// CONFIGURAÇÕES INICIAIS 


// Define uma instancia do oneWire para comunicacao com o sensor de temperatura
OneWire oneWire(sensor_temperatura);

DallasTemperature sensors(&oneWire);
DeviceAddress ENDERECO_SENSOR_TEMPERATURA;

//#######################################################################################################################
// PAINEL DE CONTROLE

//Define as chaves de controle
boolean CH_LIGA_BOMBA1 = 0;
boolean CH_LIBERA_SENSOR_TEMPERATURA = 0;
boolean CH_RESISTENCIA = 0;
boolean CH_HABILITA_AGITADOR = 1;
boolean CH_SENSOR_NIVEL = 1;
boolean CH_CONDUTIVIMETRO = 0;

//#######################################################################################################################
// VARIÁVEIS DO SISTEMA

float TEMPERATURA, NIVEL, NIVEL_REFERENCIA, CONDUTIVIDADE;
int VELOCIDADE_AGITADOR = 50, TEMPERATURA_RESISTENCIA = 50, PWM_BOMBA1 = 0;

unsigned long time;
unsigned long TEMPO_ATUAL;
unsigned long VOLUME;

float TempoAtual = 0;
int aux = 0, passo = 5000;

float NIVEL_CALCULADO;

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
   pinMode(sensor_nivel_referencia, INPUT);

   
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
                      
        default:
                    break;

      }
   }

}

void liga_motor()
{
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
  NIVEL_REFERENCIA = analogRead(A1);
  NIVEL_CALCULADO = calculaNivel();
}

void le_condutividade()
{
  CONDUTIVIDADE = analogRead(A3);
}

float calculaNivel()
{
  return float((NIVEL-370)/60);
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
    Serial.print(" Nivel: ");
    Serial.print(NIVEL);

    Serial.print(" Referência: ");
    Serial.print(NIVEL_REFERENCIA);

    Serial.print(" Nivel apoximado: ");
    Serial.print(NIVEL_CALCULADO);
  }
  
  if(CH_CONDUTIVIMETRO)
  {
    Serial.print(" Condutividade: ");
    Serial.print(CONDUTIVIDADE);
  }

  Serial.print("\n");
}
