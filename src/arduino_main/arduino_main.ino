#define in1 3
#define in2 5
#define in3 9
#define in4 10

#define byte_inicial ':'
#define byte_final '\n'

#define estado_inicial 0
#define estado_botao 1
#define estado_velocidade 2
#define estado_angulo 3
#define estado_final 4

#define bytes_pacote (1 + 3 + 3)

uint8_t estado;
uint8_t recebidos, recebidosItem;
uint8_t botao;
char velocidade[4];
char angulo[4];

void setup() {
  estado = estado_inicial;
  recebidos = 0;
  botao = 0;
  delay(500); // aguarda o sistema estabilizar
  Serial.begin(9600); // inicia a serial
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

  uint8_t quadrante;
  if (ang_int < 90)
    quadrante = 1;
  else if (ang_int < 180)
    quadrante = 2;
  else if (ang_int < 270)
    quadrante = 3;
  else
    quadrante = 4;

  uint8_t val_mA, val_mB;
  if (quadrante == 1 || quadrante == 4) {                                     // se estiver na direita
    val_mB = (uint8_t) vel_int;                                               // o motor B recebe a velocidade indicada no joystick
    val_mA = (uint8_t) (vel_int * (1.0 - cos(ang_int * PI / 180.0) / 2.0));   // o motor A recebe esta velocidade reduzida proporcionalmente ao cosseno do ângulo
  } else {                                                                    // caso não
    val_mB = (uint8_t) (vel_int * (1.0 + cos(ang_int * PI / 180.0) / 2.0));   // o motor B recebe a velocidade acrescida proporcionalmente ao cosseno (que será negativo) do ângulo
    val_mA = (uint8_t) vel_int;                                               // o motor A recebe a velocidade indicada no joystick
  }
  
  if (quadrante < 3) {                                // se estiver em cima/na frente
    analogWrite(in2, val_mA); analogWrite(in1, 0);    // aciona os motores
    analogWrite(in4, val_mB); analogWrite(in3, 0);    // para frente
  } else {                                            // senão
    analogWrite(in2, 0); analogWrite(in1, val_mA);    // aciona os motores
    analogWrite(in4, 0); analogWrite(in3, val_mB);    // para trás
  }
}
