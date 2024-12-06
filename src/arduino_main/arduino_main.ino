#include <Servo.h>

#define in1 3
#define in2 5
#define in3 9
#define in4 10

#define byte_inicial '_'
#define byte_final '!'

#define estado_inicial 0
#define estado_botao 1
#define estado_velocidade 2
#define estado_angulo 3
#define estado_final 4

#define bytes_pacote (1 + 3 + 3)

Servo servo;

uint8_t estado;
uint8_t recebidos, recebidosItem;
uint8_t botao;
char velocidade[4];
char angulo[4];

void setup() {
  estado = estado_inicial;
  recebidos = 0;
  botao = 0;
  
  servo.attach(11);
  
  delay(500); // aguarda o sistema estabilizar
  Serial.begin(9600); // inicia a serial
  
  servo.write(0);
  
  delay(500); // aguarda o sistema estabilizar
}


void loop() {
  if (!Serial.available())
    return;

  const uint8_t recebido = Serial.read();
  switch (recebido) {
    case byte_inicial:
      estado = estado_botao;
      recebidos = 0;
      return;
    case byte_final:
      if (recebidos != bytes_pacote) {
        estado = estado_inicial;
        recebidos = 0;
        return;
      }
      estado = estado_inicial;
      recebidos = 0;
      break;
    default:
      if (recebido < '0' || recebido > '9' || recebidos >= bytes_pacote) {
        estado = estado_inicial;
        recebidos = 0;
        return;
      }
      recebidos++;
      switch (estado) {
        case estado_botao:
          botao = (recebido != '0');
          recebidosItem = 0;
          estado = estado_velocidade;
          return;
        case estado_velocidade:
          velocidade[recebidosItem] = recebido;
          recebidosItem++;
          if (recebidosItem >= 3) {
            recebidosItem = 0;
            velocidade[3] = 0;
            estado = estado_angulo;
          }
          return;
        case estado_angulo:
          angulo[recebidosItem] = recebido;
          recebidosItem++;
          if (recebidosItem >= 3) {
            angulo[3] = 0;
            estado = estado_final;
          }
          return;
        default:
          estado = estado_inicial;
          recebidos = 0;
          return;
      }
      return;
  }

  // transforma a velocidade em um inteiro e, então, o mapeia entre 0 e 255
  int vel_int = atoi(velocidade);
  vel_int = map(vel_int, 0, 100, 0, 255);

  // transforma o angulo em um inteiro
  int ang_int = atoi(angulo);

  /*
  Serial.println("----------");
  Serial.print("b: ");
  Serial.println(botao ? "1" : "0");
  Serial.print("v: ");
  Serial.println(vel_int);
  Serial.print("a: ");
  Serial.println(ang_int);
  */
  
  uint8_t inverter = 0;
  uint8_t val_mA, val_mB;
  
  switch (ang_int) {
    case 90: // Vira para direita
      val_mA = 0;
      val_mB = (uint8_t)vel_int;
      break;
    case 180: // Move para trás
      inverter = 1;
      val_mA = (uint8_t)vel_int;
      val_mB = (uint8_t)vel_int;
      break;
    case 270: // Vira para esquerda
      val_mA = (uint8_t)vel_int;
      val_mB = 0;
      break;
    default: // Move para frente ou para
      val_mA = (uint8_t)vel_int;
      val_mB = (uint8_t)vel_int;
      break;
  }
  
  if (!inverter) {
    analogWrite(in2, val_mA); analogWrite(in1, 0);
    analogWrite(in4, val_mB); analogWrite(in3, 0);
  } else {
    analogWrite(in2, 0); analogWrite(in1, val_mA);
    analogWrite(in4, 0); analogWrite(in3, val_mB);
  }
  
  if (botao) {
    servo.write(135);
    delay(1000);
    servo.write(0);
    delay(1000);
  }
}
