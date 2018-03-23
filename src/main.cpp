/*
  Control temporizado para los quemadores de una estufa
  utilizando servomotores como actuadores.

  Autores:
  Heri (Alumno)
  Jesús David (Asesor)

  16/03/2018


  Sobre las librerias.
    En cada liinea de adición de las librerias utilizadas en el proyecto se
  al final de la linea se muestra el numero correspondiente al ID utilizao para
  su instalación via "pio lib install", las librerias que no con tengan este
  numero se incluyen en el proyecto.

*/

// Librerias.
#include <Arduino.h>
#include <Keypad.h>   // 165
#include <Wire.h>     // 2603
#include "LiquidCrystal_I2C.h"
#include <TimeLib.h>  // 44
#include <RTClib.h>   // 83
#include <Servo.h>    // 883

// Modo depuracion. COMENTAR PARA DESACTIVAR LA DEPURACION.
//#define DEPURACION

// Definiciones
#define BOTON_A 50 // Push para configuración hora A
#define BOTON_B 52 // Push para configuración hora B
#define BUZZER  12 // Indicador.
#define SERVO_A 10 // Servomotor A
#define SERVO_B 11 // Servomotor B

// Fucniones. Declaraciones.
void mostrarHora();
void setHoraA();
void setHoraB();
void comparacion();

// Constantes para Keypad.
const byte filas = 4;
const byte columnas = 4;
byte pinesF[filas] = {9,8,7,6};
byte pinesC[columnas] = {5,4,3,2};
char teclas[filas][columnas] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Variables.
char tecla;
unsigned long millisAnterior = 0;
unsigned long millisActual  = 0;
bool stateStop = false;
bool T_A = false;
int i;
int tecla1;
int seg1,min1,hor1,ano1,mes1,dia1; //Hora que trae el reloj DS1307RTC
int min2 = 0;
int hor2 = 0;
int min3 = 0;
int hor3 = 0;

// Banderas.
bool primerInicio = true;

// Instanciaciones.
Keypad teclado = Keypad(makeKeymap(teclas), pinesF, pinesC, filas, columnas);
RTC_DS1307 RTC;
LiquidCrystal_I2C lcd(0x3F,16,4);
Servo quemadorA;
Servo quemadorB;

void setup() {
  // Elementos para depuracion serial.
  #ifdef DEPURACION
    Serial.begin(9600);
    delay(1000);
    Serial.println("Iniciando configuracion...");
  #endif

  // Inicializando el puerto i2c.
  Wire.begin();
  #ifdef DEPURACION
    delay(1000);
    Serial.println("...  Inicializado i2c ...");
  #endif

  // Configurando modulo de reloj.
  RTC.begin();
  RTC.adjust(DateTime(__DATE__, __TIME__));
  #ifdef DEPURACION
    delay(1000);
    Serial.println("...  Configurado reloj (RTC) ...");
  #endif

  // Configurando LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  #ifdef DEPURACION
    delay(1000);
    Serial.println("...  Inicializada LCD ...");
  #endif

  // Inicializando servos.
  quemadorA.attach(SERVO_A);
  quemadorB.attach(SERVO_B);
  for(int i = quemadorA.read(); i>=10; i--) {
    quemadorA.write(i);
    delay(50);
  }
  #ifdef DEPURACION
    delay(1000);
    Serial.println("...  Inicializados servos ...");
  #endif

  // Inicialización de contadores para retardo millis.
  millisAnterior = 0;
  millisActual  = 0;

  // Configurando entradas y salidas.
  pinMode(BOTON_A, INPUT_PULLUP); // Activando entrada para BOTON_A con PULL-UP.
  pinMode(BOTON_B, INPUT_PULLUP); // Activando entrada para BOTON_A con PULL-UP.
  pinMode(BUZZER,OUTPUT);

  #ifdef DEPURACION
    delay(1000);
    Serial.println("... finanlizando configuracion ...");
    delay(100);
  #endif
}

void loop() {
  #ifdef DEPURACION
    delay(1000);
    Serial.println("En loop principal.");
  #endif

  lcd.clear();
  // Esperando por una pulsación. NOTA: BONTONES A Y B CON PULL-UP.
  if(!digitalRead(BOTON_A)) {
    #ifdef DEPURACION
      delay(1000);
      Serial.println("Se pulso BOTON_A");
    #endif

    setHoraA();
  }
  else if(!digitalRead(BOTON_B)) {
    #ifdef DEPURACION
      delay(1000);
      Serial.println("Se pulso BOTON_B");
    #endif

    setHoraB();
  }
  else {
    // Mostrando las horas real y programadas.
    mostrarHora();

    // Realizando la comparacion entre hora real y programada.
    if((min2==0)||(hor2==0)||(min3==0)||(hor3==0)) {
      primerInicio = false;
    }
    else {
      primerInicio = true;
    }
    if(((min1 >= min2)&&(hor1 >= hor2)) && primerInicio) {
      for(int i = 5; i<=175; i++) {
        quemadorA.write(i);
        delay(50);
      }
      #ifdef DEPURACION
        Serial.println("Listo tiempo A");
      #endif
    }
    if(((min1 >= min3)&&(hor1 >= hor3)) && primerInicio) {
      #ifdef DEPURACION
        Serial.println("Listo tiempo B");
      #endif
    }
  }
}

// Funciones. Definiciones.
void mostrarHora() {
  #ifdef DEPURACION
    delay(1000);
    Serial.println("Se muestra hora.");
  #endif

  DateTime now = RTC.now();
  // En la anterior linea ya suponemos que el reloj ha sido SETeado en otro momento
  // y que la hora ya esta en la memoria. Esa hora esta en now().
  //De aqui en adelante nos vamos a dedicar a escribir la hora en la pantalla.
  lcd.setCursor(0,0);
  lcd.print("FECHA:");
  lcd.setCursor(6,0);
  dia1=now.day();
  if(dia1<10) {
    lcd.print("0");
    lcd.setCursor(7,0);
  }
  lcd.print(dia1);
  lcd.setCursor(8,0);
  lcd.print("/") ;
  lcd.setCursor(9,0);
  mes1=now.month();
  if(mes1<10) {
    lcd.print("0");
    lcd.setCursor(10,0);
  }
  lcd.print(mes1);
  lcd.setCursor(11,0);
  lcd.print("/") ;
  lcd.setCursor(12,0);
  ano1=now.year();
  if(ano1<10) {
    lcd.print("0");
    lcd.setCursor(13,0);}
  lcd.print(ano1);
  lcd.setCursor(0,1);
  hor1=now.hour();
  if(hor1<10) {
    lcd.print("0");
    lcd.setCursor(1,1);
  }
  lcd.print(hor1);
  lcd.setCursor(2,1);
  lcd.print(":") ;
  lcd.setCursor(3,1);
  min1=now.minute();
  if(min1<10) {
    lcd.print("0");
    lcd.setCursor(4,1);
  }
  lcd.print(min1);
  lcd.setCursor(5,1);
  lcd.print(":") ;
  lcd.setCursor(6,1);
  seg1=now.second();
  if(seg1<10) {
    lcd.print("0");
    lcd.setCursor(7,1);
  }
  lcd.print(seg1);

  // Tiempos de acción A
  lcd.setCursor(0,2);
  lcd.print("Tiempo A: ");
  lcd.setCursor(10,2);
  lcd.print(hor2);
  lcd.setCursor(12,2);
  lcd.print(":");
  lcd.setCursor(13,2);
  lcd.print(min2);

  // Tiempos de acción B
  lcd.setCursor(0,3);
  lcd.print("Tiempo B: ");
  lcd.setCursor(10,3);
  lcd.print(hor3);
  lcd.setCursor(12,3);
  lcd.print(":");
  lcd.setCursor(13,3);
  lcd.print(min3);


  delay(1000);
}

void setHoraA() {
  #ifdef DEPURACION
    delay(1000);
    Serial.println("Se esta configurando la hora A");
  #endif
  //Cuando se pulse el pulsador...
  //Pediremos al usuario que escriba la hora en la que quiere hacer sonar la alarma.
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Escribe la hora A");
  delay(1000);

  //La hora, constara de cuatro numeros ( 2 para los minutos y 2 para los segundos )
  // Por ello, daremos 5 vueltas dentro de un for, esperando a que el usuario pulse una tecla.
  // En la tercera vuelta (i=2), no debemos pedir al usuario que nos de un valor, sino que escribiremos " : " que separan las horas de los minutos.
  for(i=0; i<5; i++){
    char tecla;
    lcd.setCursor(i,1);
    if(i==2){lcd.print(":");
  }

  else {
    tecla=teclado.waitForKey(); // Esperamos a que pulse un boton.
    lcd.print(tecla);            //Escribimos el el valor de pulsacion en el LCD.

    switch(tecla) {              // Pulsacion es de tipo char, por eso hemos definido al principio de programa "pulsacion1", de tipo int, Que tendra un valor numerico que sera el de la hora en la que el usuario ha definido la alarma.Este valor, sera dependiendo el boton pulsado en el teclado matricial, que hemos guardado en
      case '1': tecla1=1;
      break;
      case '2': tecla1=2;
      break;
      case '3': tecla1=3;
      break;
      case '4': tecla1=4;
      break;
      case '5': tecla1=5;
      break;
      case '6': tecla1=6;
      break;
      case '7': tecla1=7;
      break;
      case '8': tecla1=8;
      break;
      case '9': tecla1=9;
      break;
      case '0': tecla1=0;
    }

    switch(i) {
      case 0: hor2=tecla1*10;       //Sabemos que el primer valor de los minutos sera un valor en decimas
      break;
      case 1: hor2=hor2+tecla1;     // El segundo valor de los minutos que sera una unidades
      break;
      case 3: min2=tecla1*10;       //Primer valor de los segundos que sera un valor en decimas
      break;
      case 4: min2=min2+tecla1;     //Segundo valor de los segundos que sera en unidades
      break;
    }
  }
  delay(500);
 }

 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print("Encendido 1");
 lcd.setCursor(0,1);
 lcd.print("Configurada");
 delay(3000);
 lcd.clear();

 return;
}

void setHoraB() {
  #ifdef DEPURACION
    delay(1000);
    Serial.println("Se esta configurando la hora B");
  #endif
  //Cuando se pulse el pulsador...
  //Pediremos al usuario que escriba la hora en la que quiere hacer sonar la alarma.
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Escribe la hora B");
  delay(1000);

  //La hora, constara de cuatro numeros ( 2 para los minutos y 2 para los segundos )
  // Por ello, daremos 5 vueltas dentro de un for, esperando a que el usuario pulse una tecla.
  // En la tercera vuelta (i=2), no debemos pedir al usuario que nos de un valor, sino que escribiremos " : " que separan las horas de los minutos.
  for(i=0; i<5; i++){
    char tecla;
    lcd.setCursor(i,1);
    if(i==2){lcd.print(":");
  }

  else {
    tecla=teclado.waitForKey(); // Esperamos a que pulse un boton.
    lcd.print(tecla);            //Escribimos el el valor de pulsacion en el LCD.

    switch(tecla) {              // Pulsacion es de tipo char, por eso hemos definido al principio de programa "pulsacion1", de tipo int, Que tendra un valor numerico que sera el de la hora en la que el usuario ha definido la alarma.Este valor, sera dependiendo el boton pulsado en el teclado matricial, que hemos guardado en
      case '1': tecla1=1;
      break;
      case '2': tecla1=2;
      break;
      case '3': tecla1=3;
      break;
      case '4': tecla1=4;
      break;
      case '5': tecla1=5;
      break;
      case '6': tecla1=6;
      break;
      case '7': tecla1=7;
      break;
      case '8': tecla1=8;
      break;
      case '9': tecla1=9;
      break;
      case '0': tecla1=0;
    }

    switch(i) {
      case 0: hor3=tecla1*10;       //Sabemos que el primer valor de los minutos sera un valor en decimas
      break;
      case 1: hor3=hor3+tecla1;     // El segundo valor de los minutos que sera una unidades
      break;
      case 3: min3=tecla1*10;       //Primer valor de los segundos que sera un valor en decimas
      break;
      case 4: min3=min3+tecla1;     //Segundo valor de los segundos que sera en unidades
      break;
    }
  }
  delay(500);
 }

 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print("Encendido 2");
 lcd.setCursor(0,1);
 lcd.print("Configurado");
 delay(3000);
 lcd.clear();

 return;
}
