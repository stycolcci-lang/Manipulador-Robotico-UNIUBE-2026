/*
  MANIPULADOR ROBOTICO - DOIS BLOCOS EM LOOP - ARDUINO UNO
  Comandos: I habilita; N inicia loop; F solicita parada AO FIM do ciclo
  completo (Bloco 1 + pausa 2 s + Bloco 2). X retira PWM imediatamente
  apenas para contingencia; X nao corta fisicamente a energia.

  Sintaxe dos movimentos reproduz o Teach & Repeat 200 Continuo T500:
  motor 1 base D3; motor 2 ombro duplo D4/D5 invertidos;
  motor 3 cotovelo D6; motor 4 punho vertical D9;
  motor 5 giro mao D10; motor 6 garra D11.
  G75 = destino ABSOLUTO 75 graus; + ou - = 30 graus.
  LIMITES: iguais aos do arquivo Teach & Repeat 200 Continuo T500.
  Movimento fora do limite = BLOQUEADO (tal como o programa de origem).

  T1000, F e E ao fim da lista original sao comandos de configuracao,
  finalizar gravacao e gravar EEPROM, nao movimentos. Logo nao entram
  no roteiro automatico. Movimentos anteriores a T1000 permanecem T500.

  ATENCAO: ZERO[] herdado do codigo de aprendizagem: conferir antes
  de habilitar! O primeiro I pode movimentar se a pose fisica nao coincidir.
  Testar inicialmente sem carga e com area livre; GND comum e chave
  fisica de corte de energia acessivel. Nao e parada de emergencia por F.
*/
#include <Arduino.h>
#include <Servo.h>
#include <avr/pgmspace.h>
#include <string.h>

const uint8_t NG = 6, NS = 7;
const uint8_t PINOS[NS] = {3,4,5,6,9,10,11};
const uint8_t ZERO[NG] = {90,90,160,90,90,90};
const uint8_t MIN_ANG[NG] = {10,0,0,60,60,0};
const uint8_t MAX_ANG[NG] = {170,180,180,120,120,100};
const uint8_t PASSO_GRAUS = 30;
const uint16_t TEMPO_MOVIMENTO = 500; // T500 antes do T1000 final
const unsigned long PAUSA_INTERNA = 2000UL;
const unsigned long PAUSA_ENTRE_CICLOS = 4000UL;

enum Tipo : uint8_t { ABS=0, MAIS=1, MENOS=2 };
struct Passo { uint8_t grupo, tipo, valor; };

const Passo BLOCO_1[] PROGMEM = {
  {1, ABS, 75},
  {1, MENOS, 0},
  {2, ABS, 75},
  {2, MENOS, 0},
  {2, MENOS, 0},
  {4, MENOS, 0},
  {0, MENOS, 0},
  {0, MENOS, 0},
  {5, MENOS, 0},
  {5, MENOS, 0},
  {1, MENOS, 0},
  {2, MAIS, 0},
  {5, MAIS, 0},
  {5, MAIS, 0},
  {4, MENOS, 0},
  {2, MENOS, 0},
  {1, MAIS, 0},
  {0, MAIS, 0},
  {0, MAIS, 0},
  {0, MAIS, 0},
  {0, MAIS, 0},
  {4, MAIS, 0},
  {1, MENOS, 0},
  {4, MENOS, 0},
  {2, MAIS, 0},
  {5, MENOS, 0},
  {5, MENOS, 0},
  {4, MENOS, 0},
  {1, MAIS, 0},
  {0, MENOS, 0},
  {0, MENOS, 0},
  {1, MAIS, 0},
  {1, MAIS, 0},
  {2, MAIS, 0},
  {2, MAIS, 0},
  {4, MAIS, 0},
  {5, MAIS, 0},
  {5, MAIS, 0},
};
const Passo BLOCO_2[] PROGMEM = {
  {1, ABS, 75},
  {1, MENOS, 0},
  {2, ABS, 75},
  {2, MENOS, 0},
  {2, MENOS, 0},
  {4, MENOS, 0},
  {0, MAIS, 0},
  {0, MAIS, 0},
  {5, MENOS, 0},
  {5, MENOS, 0},
  {1, MENOS, 0},
  {2, MAIS, 0},
  {5, MAIS, 0},
  {5, MAIS, 0},
  {4, MENOS, 0},
  {2, MENOS, 0},
  {1, MAIS, 0},
  {0, MENOS, 0},
  {0, MENOS, 0},
  {0, MENOS, 0},
  {0, MENOS, 0},
  {4, MAIS, 0},
  {1, MENOS, 0},
  {4, MENOS, 0},
  {2, MAIS, 0},
  {5, MENOS, 0},
  {5, MENOS, 0},
  {4, MENOS, 0},
  {1, MAIS, 0},
  {0, MAIS, 0},
  {0, MAIS, 0},
  {1, MAIS, 0},
  {1, MAIS, 0},
  {2, MAIS, 0},
  {2, MAIS, 0},
  {4, MAIS, 0},
  {5, MAIS, 0},
  {5, MAIS, 0},
};


const uint8_t TAM_1 = sizeof(BLOCO_1)/sizeof(BLOCO_1[0]);
const uint8_t TAM_2 = sizeof(BLOCO_2)/sizeof(BLOCO_2[0]);

Servo servo[NS];
uint8_t atual[NG];
bool habilitado = false;
bool rodando = false;
bool pararNoFim = false;
bool emergencia = false;
uint32_t numeroCiclos = 0;
char linha[16]; uint8_t posLinha = 0;

void escreverServos(const uint8_t a[NG]) {
  servo[0].write(a[0]);
  servo[1].write(a[1]);
  servo[2].write(180 - a[1]);
  servo[3].write(a[2]);
  servo[4].write(a[3]);
  servo[5].write(a[4]);
  servo[6].write(a[5]);
}

void desligarPWM() {
  for(uint8_t i=0;i<NS;i++) servo[i].detach();
  habilitado = false;
  rodando = false;
}

void habilitar() {
  if(habilitado) {Serial.println(F("Servos ja habilitados."));return;}
  uint8_t inicial[NG]; memcpy(inicial,atual,NG);
  for(uint8_t i=0;i<NS;i++) {
    uint8_t ref = i==2 ? (uint8_t)(180-inicial[1]) :
                  i==0 ? inicial[0] : i==1 ? inicial[1] : inicial[i-1];
    servo[i].write(ref);
    servo[i].attach(PINOS[i]);
  }
  escreverServos(atual);
  habilitado=true;
  Serial.println(F("Habilitado. Envie N para iniciar o looping."));
}

void processarComando(const char* s) {
  if(!strcmp(s,"F")) {
    if(rodando) {
      if(!pararNoFim) Serial.println(F("F recebido: parar APOS concluir os dois blocos deste ciclo."));
      pararNoFim = true;
    } else Serial.println(F("Nao esta em looping."));
  } else if(!strcmp(s,"X")) {
    emergencia = true;
    desligarPWM();
    Serial.println(F("X: PWM desabilitado. Desligue a fonte fisicamente se necessario."));
  } else if(!strcmp(s,"P")) {
    Serial.print(F("Ciclos concluidos: "));Serial.println(numeroCiclos);
    Serial.print(F("Rodando: "));Serial.println(rodando?F("SIM"):F("NAO"));
  } else if(!strcmp(s,"?")) {
    Serial.println(F("I habilita | N looping | F para ao fim do ciclo | X desabilita PWM | P status"));
  } else if(!strcmp(s,"I")) {
    if(!rodando) habilitar();
  } else if(!strcmp(s,"N")) {
    if(rodando) Serial.println(F("Looping ja iniciado."));
    else if(!habilitado) Serial.println(F("Antes envie I."));
    else { rodando=true; pararNoFim=false; emergencia=false; Serial.println(F("Looping iniciado.")); }
  } else if(*s) Serial.println(F("Comando nao reconhecido."));
}

void lerSerial() {
  while(Serial.available()) {
    char c=(char)Serial.read();
    if(c=='\n' || c=='\r') {
      if(posLinha) {
        linha[posLinha]='\0';
        for(uint8_t i=0;i<posLinha;i++) {
          if(linha[i]>='a' && linha[i]<='z') linha[i]-=32;
        }
        processarComando(linha);
        posLinha=0;
      }
    } else if(c=='X'||c=='x') {  // X de emergencia nao precisa ENTER
      posLinha=0;
      emergencia=true;
      desligarPWM();
      Serial.println(F("EMERGENCIA: PWM removido. Desligue fonte fisicamente."));
    } else if(c!=' ' && c!='\t' && posLinha < sizeof(linha)-1) {
      linha[posLinha++]=c;
    }
  }
}

float suave(float t) {
  if(t<0) t=0; if(t>1) t=1;
  return t*t*t*(10.0f+t*(-15.0f+6.0f*t));
}

bool moverUm(uint8_t grupo, uint8_t destino, uint16_t ms) {
  if(!habilitado || emergencia) return false;
  uint8_t origem=atual[grupo];
  if(destino==origem) return true;
  uint8_t pose[NG]; memcpy(pose,atual,NG);
  unsigned long inicio=millis();
  while(true) {
    lerSerial();
    if(emergencia) return false;
    unsigned long dt=millis()-inicio;
    if(dt>ms) dt=ms;
    float ang=origem+((int)destino-(int)origem)*suave((float)dt/(float)ms);
    pose[grupo]=(uint8_t)(ang+0.5f);
    escreverServos(pose);
    if(dt>=ms) break;
    delay(10);
  }
  atual[grupo]=destino;
  escreverServos(atual);
  return true;
}

Passo lerPasso(const Passo* vetor, uint8_t idx) {
  Passo p;
  p.grupo=pgm_read_byte(&vetor[idx].grupo);
  p.tipo=pgm_read_byte(&vetor[idx].tipo);
  p.valor=pgm_read_byte(&vetor[idx].valor);
  return p;
}

bool executarBloco(const Passo* passos, uint8_t quantidade, uint8_t bloco) {
  Serial.print(F("Bloco "));Serial.print(bloco);
  Serial.print(F(" - "));Serial.print(quantidade);Serial.println(F(" comandos de movimento."));
  for(uint8_t i=0;i<quantidade;i++) {
    lerSerial();
    if(emergencia) return false;
    Passo p=lerPasso(passos,i);
    int alvo=(p.tipo==ABS) ? p.valor : ((int)atual[p.grupo]+(p.tipo==MAIS ? PASSO_GRAUS : -PASSO_GRAUS));
    if(alvo<MIN_ANG[p.grupo] || alvo>MAX_ANG[p.grupo]) {
      Serial.print(F("Comando ignorado (limite): bloco "));Serial.print(bloco);
      Serial.print(F(" passo "));Serial.print(i+1);
      Serial.print(F(" motor "));Serial.print(p.grupo+1);
      Serial.print(F(" alvo "));Serial.println(alvo);
      continue;
    }
    if(!moverUm(p.grupo,(uint8_t)alvo,TEMPO_MOVIMENTO)) return false;
  }
  return true;
}

bool aguardar(unsigned long ms) {
  unsigned long inicio=millis();
  while(millis()-inicio < ms) {
    lerSerial();
    if(emergencia) return false;
    delay(10);
  }
  return true;
}

void executarCicloCompleto() {
  if(!rodando || !habilitado) return;
  Serial.print(F("==== CICLO "));Serial.print(numeroCiclos+1);Serial.println(F(" ===="));
  if(!executarBloco(BLOCO_1,TAM_1,1)) return;
  if(!aguardar(PAUSA_INTERNA)) return;
  if(!executarBloco(BLOCO_2,TAM_2,2)) return;
  numeroCiclos++;
  Serial.print(F("Ciclo concluido: "));Serial.println(numeroCiclos);
  lerSerial();
  if(pararNoFim) {
    rodando=false;
    pararNoFim=false;
    Serial.println(F("F cumprido: looping encerrado depois do ciclo completo."));
    Serial.println(F("Servos permanecem sustentando posicao. Envie N para reiniciar."));
    return;
  }
  Serial.println(F("Pausa de 4 segundos ate o proximo ciclo."));
  if(!aguardar(PAUSA_ENTRE_CICLOS)) return;
  if(pararNoFim) {
    rodando=false;
    pararNoFim=false;
    Serial.println(F("F cumprido: ciclo anterior ja concluido. Looping encerrado."));
  }
}

void setup() {
  Serial.begin(9600);
  memcpy(atual,ZERO,NG);
  Serial.println(F("BRACO AUTOMATICO - 2 BLOCOS / LOOPING"));
  Serial.println(F("I habilita; N inicia; F encerra apos ciclo; X emergencia; P status"));
  Serial.println(F("Inicialmente, nenhum servo recebe PWM."));
  Serial.println(F("ATENCAO: confira angulos iniciais antes de I."));
}

void loop() {
  lerSerial();
  if(rodando && habilitado) executarCicloCompleto();
}