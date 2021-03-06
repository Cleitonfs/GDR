/*****************************************************************************************************
 * ------------------------------------------GDR - Guide Dog Robot---------------------------------------
 * Versão do Software: 2.1
 ******************************************************************************************************/
//--- BIBLIOTECAS ---    
#include <Servo.h>                 //Biblioteca de manipulação do servo motor.    
#include <AFMotor.h>               //Biblioteca de manipulação dos motores DCs.  
#include <SoftwareSerial.h>        //Biblioteca para bluetooth

//--- DEFINICAO HARDWARE ---  
#define trigPinF    A8                //Pino TRIG do sensor frontal no pino analógico A8
#define echoPinF    A9                //Pino ECHO do sensor frontal no pino analógico A9
//#define trigPinS A10               //Pino TRIG do sensor superior no pino analógico A10
//#define echoPinS A11               //Pino ECHO do sensor superior no pino analógico A11
#define BUZZER   A12               //Pino BUZZER no pino analgi A12 
#define IR       A13               //Sensor INFRA-VERMELHO no pino analógico A13 
#define serv     10                //Pino de controle do SERVO MOTOR  <------------------------ATENÇAO
#define batPin   A15               //Pino de leitura do nível da tensão de alimentação do arduino no pino analógico A15



SoftwareSerial bluetooth(14, 15);  //Adaptador BLUETOOTH portas RX/FX

//--- SELECAO MOTORES ---
AF_DCMotor motor1(3);              //Define MOTOR 1 ligado ao pino M1 do SHIELD  
AF_DCMotor motor2(4);              //Define MOTOR 2 ligado ao pino M2 do SHIELD 
Servo servo;                       //Define nome do SERVO MOTOR


// --- PROTOTIPO FUNCOES AUXILIARES ---
float measureDistance();                //Função para medir, calcular e retornar a distância em cm
void trigPulse();                       //Função que gera o pulso de trigger de 10µs
void decision();                        //Função para tomada de decisão. Qual melhor caminho?
void robot_forward(int v);              //Função para movimentar robô para frente
void robot_backward(int v);             //Função para movimentar robô para trás
void robot_left(int v);                 //Função para movimentar robô para esquerda
void robot_right(int v);                //Função para movimentar robô para direita
void robot_stop(int v);                 //Função para parar o robô
void verificaPiso();                    //Função para verificar condiçao do piso
void bipa(int tempo);                   //Função para robo bipar
void verificaTensao();           //Função para robo veirificar o nível da tensão de entrada (alimentação);
 
// --- VARIAVEIS GLOBAIS ---
int velocidade;                        //Armazena a velocidade dos motores (8 bits)
float dist_cm;                         //Armazena a distância em centímetros entre o robô e o obstáculo
float dist_right;                      //Armazena a distância em centímetros da direita
float dist_left;                       //Armazena a distância em centímetros da esquerda
int status_IR_piso_ok = 1;             //Armazena 1 enquanto o piso estiver OK e 0 quando NAO OK   
int contador;
//const nivelBat_min  = 4;               //Armazena o valor mínimo da tensão de alimentação
//const nivelBat_max = 6;                //Armazena o valor máximo da tensão de alimentação
float nivelBat;                        //Armazena o valor lido no circuito divisor de tensão na placa do protoboard 
         
// --- CONFIGURACOES INICIAIS ---
void setup()
{  
 Serial.begin(9600);
 bluetooth.begin(9600);                           //Inicializa a comunicação bluetooth 
 pinMode(trigPinF, OUTPUT);                       //Saída para o pulso de trigger
 pinMode(echoPinF, INPUT);                        //Entrada para o pulso de echo
 pinMode(serv, OUTPUT);                           //Saída para o servo motor
 pinMode(BUZZER,OUTPUT);                          //Saída para o buzzer
 pinMode(batPin, INPUT);
 servo.attach(serv);                              //Objeto servo no pino de saída do servo
 
 digitalWrite(trigPinF, LOW);                     //Pino de trigger inicia em low
 servo.write(90);                                 //Centraliza servo
 delay(500);                                      //Aguarda meio segundo antes de iniciar
 velocidade = 255;                                //Inicia velocidade no valor máximo
 
 bipa(1500);                                      //Robo bipa por 1,5s 
 bluetooth.println("Bluetooth OK!");              //Envia mensagem para receptor bluetooth
}


 
// --- LOOP INFINITO ---
void loop()
{
  verificaTensao();
  verificaPiso();
  robot_forward(velocidade);
  delay(80);                               //Faz verificacao a cada 80ms de obstaculos a frente
  dist_cm = measureDistance();             //Verifica distancia de algum obstaculo a frente
  Serial.println(dist_cm);
  if(dist_cm < 20)
  {
    decision();
  }
  contador += 1;
  Serial.println(contador);
  if (contador == 20)
  {
    bipa(300);
    contador = 0;
  }
}
 
// --- DESENVOLVIMENTO DAS FUNCOES AUXILIARES ---
 
float measureDistance()                       //Função que retorna a distância em centímetros
{
  float pulse;                                //Armazena o valor de tempo em µs que o pino echo fica em nível alto
  trigPulse();                                //Envia pulso de 10µs para o pino de trigger do sensor
  pulse = pulseIn(echoPinF, HIGH);            //Mede o tempo em que echo fica em nível alto e armazena na variável pulse
  /*
    >>> Cálculo da Conversão de µs para cm:
   Velocidade do som = 340 m/s = 34000 cm/s
   1 segundo = 1000000 micro segundos
   
      1000000 µs - 34000 cm/s
            X µs - 1 cm
            
                1000000
            X = ------- = 29.41
                 34000
                 
    Para compensar o ECHO (ida e volta do ultrassom) multiplica-se por 2
    
    X' = 29.41 x 2 = 58.82
 */
  
  return (pulse/58.82);                      //Calcula distância em centímetros e retorna o valor
}
 
void trigPulse()                             //Função para gerar o pulso de trigger para o sensor HC-SR04
{
   digitalWrite(trigPinF,HIGH);              //Saída de trigger em nível alto
   delayMicroseconds(10);                    //Por 10µs ...
   digitalWrite(trigPinF,LOW);               //Saída de trigger volta a nível baixo
}
 
void decision()                              //Compara as distâncias e decide qual melhor caminho a seguir
{
   robot_stop(velocidade);                   //Para o robô
   bipa(1000);                               //Bipa 1s para indicar obstaculo identificado
   delay(500);                               //Aguarda 500ms
   servo.write(35);                          //Move sensor para direita através do servo
   delay(500);                               //Aguarda 500ms
   dist_right = measureDistance();           //Mede distância e armazena em dist_right
   delay(2000);                              //Aguarda 2000ms
   servo.write(145);                         //Move sensor para esquerda através do servo
   delay(500);                               //Aguarda 500ms
   dist_left = measureDistance();            //Mede distância e armazena em dis_left
   delay(2000);                              //Aguarda 2000ms
   servo.write(90);                          //Centraliza servo
   delay(500);
   if(dist_right > dist_left)                //Distância da direita maior que da esquerda?
   {                                         //Sim...
     bluetooth.println("Vai para direita!");
     robot_backward(velocidade);            //Move o robô para trás
     delay(600);                            //Por 600ms
     robot_right(velocidade);               //Move o robô para direita
     delay(700);                           //Por 2000ms
     robot_forward(velocidade);             //Move o robô para frente
   }
   else                                      //Não...
   {
     bluetooth.println("Vai para esquerda!");
     robot_backward(velocidade);            //Move o robô para trás
     delay(600);                            //Por 600ms
     robot_left(velocidade);                //Move o robô para esquerda
     delay(700);                           //Por 2000ms
     robot_forward(velocidade);             //Move o robô para frente
   }
 
}
 
void robot_forward(int v)
{
     motor1.setSpeed(v);
     motor1.run(FORWARD);
     motor2.setSpeed(v);
     motor2.run(FORWARD);
}
 
void robot_backward(int v)
{
     motor1.setSpeed(v);
     motor1.run(BACKWARD);
     motor2.setSpeed(v);
     motor2.run(BACKWARD);
}
 
void robot_left(int v)
{
     motor1.setSpeed(v);
     motor1.run(FORWARD);
     motor2.setSpeed(v);
     motor2.run(BACKWARD);
}
 
void robot_right(int v)
{
     motor1.setSpeed(v);
     motor1.run(BACKWARD);
     motor2.setSpeed(v);
     motor2.run(FORWARD);
}
 
void robot_stop(int v)
{
     motor1.setSpeed(v);
     motor1.run(RELEASE);
     motor2.setSpeed(v);
     motor2.run(RELEASE);
}

void verificaPiso()
{
   status_IR_piso_ok = digitalRead(IR);        //Ler sensor IR
   if(status_IR_piso_ok == 1){                   //Verifica condiçao do piso
      bluetooth.println("Buraco detectado!");  //Exibe no serial monitor   
      decision();                              //Para o robô e decide o lado a seguir sem obstaculo  
   }
}

void verificaTensao()
{
  nivelBat = analogRead(batPin);
  if ((nivelBat >= 1000) && (nivelBat <=1200)){
    Serial.print("Bateria normal com: ");
    Serial.print(nivelBat);
    Serial.println("mV");
    //bluetooth.println("Baterial normal");
    
    } else if (nivelBat < 1000){
      while (nivelBat < 1000){
      Serial.print("Bateria baixa com: ");
      Serial.print(nivelBat);
      Serial.println("mV");
      robot_stop(velocidade);
      //bluetooth.println("Baterial baixa");
      bipa(100);
      delay(950);
      bipa(200);
      delay(950);
      bipa(100);
      delay(950);
      }
        } else {
           Serial.print("Bateria sobrecaregada com: ");
           Serial.print(nivelBat);
           Serial.println("mV");
           //bluetooth.println("Sobrecarga");
        } 
  
}
  
void bipa(int tempo)
{
     digitalWrite(BUZZER, HIGH);               //Liga buzzer 
     delay(tempo);                             //Tempo buzzer ligado
     digitalWrite(BUZZER, LOW);                //Desliga buzzer
} 
