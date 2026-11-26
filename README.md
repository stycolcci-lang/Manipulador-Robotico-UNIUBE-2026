# Manipulador Robótico Autônomo — UNIUBE 2026

Projeto acadêmico da disciplina de Robótica da Universidade de Uberaba (UNIUBE), desenvolvido em 2026.

## Objetivo
Desenvolver um manipulador robótico capaz de executar movimentos automáticos de coleta, transporte e deposição de um objeto por meio de servomotores comandados por Arduino Uno.

## Arquivo de programação
- [Braco_Robotico_Looping_2_Blocos_Parada_F_UNO.ino](Braco_Robotico_Looping_2_Blocos_Parada_F_UNO.ino): **versão de looping contínuo desenvolvida posteriormente aos testes relatados**, a ser validada no protótipo antes de ser identificada como a versão da demonstração final.

O código executa, em ordem, o Bloco 1, uma pausa de 2 segundos e o Bloco 2. Depois aguarda 4 segundos e repete a sequência. Ao receber `F`, termina os dois blocos do ciclo em andamento antes de encerrar o looping. Essa parada é lógica: os servomotores permanecem habilitados.

## Hardware e conexões
- Controlador: Arduino Uno.
- Sete servomotores distribuídos em seis grupos de comando.
- Grupo 1: base — D3.
- Grupo 2: ombro, dois servos sincronizados e com comandos opostos — D4 e D5.
- Grupo 3: cotovelo — D6.
- Grupo 4: punho vertical — D9.
- Grupo 5: grupo de movimentação da mão no programa — D10.
- Grupo 6: garra — D11.
- Fonte externa utilizada nos testes: 5 Vcc / 20 A para os servomotores.
- Arduino alimentado pela porta USB do computador, com referência GND comum à fonte.

## Instruções básicas
1. Abra o arquivo `.ino` na Arduino IDE, selecione a placa Arduino Uno e a porta correta.
2. Verifique e alinhe os ângulos físicos dos servomotores com a matriz `ZERO[]` do código **antes** de habilitá-los. O acionamento sem alinhamento pode causar movimentos bruscos ou colisões.
3. Faça o primeiro teste sem carga, com a área de movimentação livre e um interruptor físico para corte de energia acessível.
4. Abra o Monitor Serial em **9600 baud**, com **Nova linha**.
5. Envie `I` para habilitar os servos e `N` para iniciar o looping.

| Comando | Função |
| --- | --- |
| `I` | Habilita servomotores |
| `N` | Inicia execução em looping |
| `F` | Solicita parada **após concluir o ciclo atual** |
| `P` | Exibe andamento/status |
| `X` | Remove imediatamente os sinais PWM; **não corta a alimentação elétrica** |
| `?` | Exibe ajuda |

A configuração da rotina preserva o passo de 30° e os limites programados no código de origem. Movimentos fora dos limites são ignorados e identificados no Monitor Serial. A duração nominal de cada movimento é de 500 ms; pausas são de 2 s entre os dois blocos e 4 s entre ciclos.

## Projeto eletrônico e simulação
O projeto eletrônico foi desenvolvido e simulado na plataforma **Cirkit Designer**:

[Consultar o projeto no Cirkit Designer](https://app.cirkitdesigner.com/project/4491e687-1fa9-4162-b77a-02f6f4b94dfa)

## Registros da demonstração acadêmica
Na apresentação da 19ª ENTEC da UNIUBE, em 25/11/2026, o manipulador realizou quatro ciclos de coleta, transporte e deposição de uma miniatura de tambor de 200 L, impressa em PETG, com massa de 30 g, segundo os registros da equipe. O deslocamento horizontal de transporte foi de aproximadamente 300 mm.

**Atenção à versão do código:** a sequência de looping infinito presente neste repositório foi solicitada após essa demonstração. Portanto, não é correto afirmar que esse arquivo é exatamente o programa executado na ENTEC sem confirmação adicional. Para cumprir literalmente o roteiro da N2, publicar também a versão efetivamente apresentada na demonstração final, identificada de modo inequívoco.

## Integrantes
- Ana Paula Arraes dos Santos
- Eduardo Henrique Fernandes Duarte
- Felipe Tomazini
- Rogério Cyrillo Rodrigues
- Sthenio Alves Resende
- Styve Bonome

## Segurança
Os servos de posição não fornecem automaticamente ao Arduino medida confiável do ângulo mecânico real. A parada por `F` é apenas uma parada programada no fim do ciclo. O comando `X` remove PWM, mas não corta fisicamente a energia e pode liberar o torque de sustentação; mantenha um sistema físico de desligamento disponível.
