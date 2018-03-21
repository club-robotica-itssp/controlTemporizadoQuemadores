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

// Modo depuracion. COMENTAR PARA DESACTIVAR LA DEPURACION.
#define DEPURACION

// Definiciones
#define BOTON_A 50 // Push para configuración hora A
#define BOTON_B 52 // Push para configuración hora B
#define BUZZER  12 // Indicador.

// Fucniones. Declaraciones.
void mostrarHora();
void setHoraA();
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
int min2,hor2; //Hora que defino yo como alarma. Solo me interesan la hora y los minutos
int min3,hor3;

// Instanciaciones.
Keypad teclado = Keypad(makeKeymap(teclas), pinesF, pinesC, filas, columnas);
RTC_DS1307 RTC;
LiquidCrystal_I2C lcd(0x3F,16,2);

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

  }
  else {
    mostrarHora();
  }
}

// Funciones. Definiciones.
void mostrarHora() {
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
  delay(1000);
}

void setHoraA() {
  //Cuando se pulse el pulsador...
  //Pediremos al usuario que escriba la hora en la que quiere hacer sonar la alarma.
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Escribe la hora");
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

 //////////////////////////////
 ////////////////////////
 comparacion();
 /////////////////////
 ///////////////////////////////////////////////////////
 //En cuanto sea la hora, saltara la alarma, se encenderan los LEDs y sonara el zumbador.

 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print("ES LA HORA!!");

 while(digitalRead(BOTON_A)) {
   //Hasta que pulsemos de nuevo alarma, no va a parar de hacer luz y ruido.
   digitalWrite(BUZZER,HIGH);
   delay(1000);
   digitalWrite(BUZZER,LOW);
   delay(1000);
 }                    //Alarma es un pulsador. Hay que quedarse un rato hasta que lea justo 1.
 delay(3000);
}

void comparacion() {
  while(min1!=min2||hor1!=hor2) {    //Mientras la hora de la alarma escrita por el usuario, y la hora real sean diferentes, //escribiremos la hora real y la de la alarma en la pantalla.
    DateTime now = RTC.now();
    lcd.setCursor(0,0);
    lcd.print("HORA");
    lcd.setCursor(0,1);
    lcd.print("Enc.");

    lcd.setCursor(6,0) ; //dibujamos la hora real
    hor1=now.hour();
    if(dia1<10) {
       lcd.print("0");
       lcd.setCursor(7,0);
    }
    lcd.print(hor1);
    lcd.setCursor(8,0);
    lcd.print(":") ;
    lcd.setCursor(9,0);
    min1=now.minute();
    if(min1<10) {
      lcd.print("0");
      lcd.setCursor(10,0);
    }
    lcd.print(min1);
    lcd.setCursor(11,0);
    lcd.print(":") ;
    lcd.setCursor(12,0);
    seg1=now.second();
    if(seg1<10) {
      lcd.print("0");
      lcd.setCursor(13,0);
    }
    lcd.print(seg1);
    lcd.setCursor(6,1) ; //dibujamos la hora de la alarma
    if(hor2<10) {
      lcd.print("0");
      lcd.setCursor(7,1);
    }
    lcd.print(hor2);
    lcd.setCursor(8,1);
    lcd.print(":") ;
    lcd.setCursor(9,1);
    if(min2<10) {
      lcd.print("0");
      lcd.setCursor(10,1);
    }
    lcd.print(min2);

    // Delay millis para sensar botones.
    stateStop = true;
    while (stateStop) {
      millisActual = millis();
      if((millisActual - millisAnterior) >= 1000) {
        millisAnterior = millisActual;
     	  stateStop = LOW;
      }

      if (!digitalRead(BOTON_A)) {
        Serial.println("Estoy en el delay de millis");
        return;
      }
    }
  }
  Serial.println("Estoy saliendo del while de comparacion");
}
