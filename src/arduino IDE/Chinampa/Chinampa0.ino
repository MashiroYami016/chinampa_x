//PID
#include <PID_v1.h>
double Setpoint, Input, Output;
// Parámetros de Tuning (Ajuste inicial sugerido para calentadores)
// Kp: Reacción proporcional al error actual
// Ki: Reacción a la acumulación del error (elimina el error residual)
// Kd: Reacción a la velocidad de cambio (evita el sobrepaso)
double Kp=20, Ki=5, Kd=1;

PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
//Sensor SHT30
#include <ArtronShop_SHT3x.h>
#include <Wire.h>
ArtronShop_SHT3x sht3x(0x44, &Wire); // ADDR: 0 => 0x44, ADDR: 1 => 0x45

//sensor DS18B20
#include <DallasTemperature.h>
#include <OneWire.h> 
OneWire ourWire(4);
DallasTemperature DS18B20(&ourWire);

// Definición del pines
const int pinVentiladores = 14;
const int pinPeltier = 27;
const int pinCalentador = 26;
const int pinLuz = 25;

const int pinHumidificador = 13; //  2N2222 (switch)

// 0 = apagado, 255 = velocidad máxima
int nivelVelVentiladores = 0; 
int nivelPeltier = 0; 
int nivelCalentador = 0; 
int nivelLuz = 0; 

void setup() {
  Serial.begin(115200);
  
  pinMode(pinVentiladores, OUTPUT);
  pinMode(pinPeltier, OUTPUT);
  pinMode(pinCalentador, OUTPUT);
  pinMode(pinLuz, OUTPUT);

  pinMode(pinHumidificador, OUTPUT);
  // Aplico PWM
  analogWrite(pinVentiladores, nivelVelVentiladores);
  analogWrite(pinPeltier, nivelPeltier);
  analogWrite(pinCalentador, nivelCalentador);
  analogWrite(pinLuz, nivelLuz);

  digitalWrite(pinHumidificador, LOW);

  Serial.print("Ventilador configurado a: ");
  Serial.println(nivelVelVentiladores);

  Serial.print("Placa Peltier configurada a: ");
  Serial.println(nivelPeltier);

  Serial.print("Calentador configurado a: ");
  Serial.println(nivelCalentador);

  Serial.print("Luz de cultivo configurada a: ");
  Serial.println(nivelLuz);
  
  //SHT30

  Wire.begin();
  while (!sht3x.begin()) {
    Serial.println("SHT3x not found !");
    delay(1000);
  }

  //DS18B20
  DS18B20.begin();

  //PID
  Setpoint = 30.0;
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(0, 255); // Rango de PWM del ESP32
}

void loop() {

  // --- Lectura SHT30 ---
  if (sht3x.measure()) {
    Serial.print("SHT30 - Humedad: ");
    Serial.print(sht3x.humidity());
    Serial.print("% | Temperatura: ");
    Serial.print(sht3x.temperature());
    Serial.println("°C");
  } else {
    Serial.println("Error leyendo el SHT30");
  }

  // --- Lectura DS18B20 ---  DS18B20.requestTemperatures();
  float tempLeida = DS18B20.getTempCByIndex(0);
  Serial.print("DS18B20 - Temperatura Sonda: ");
  if(tempLeida != DEVICE_DISCONNECTED_C) {
    Serial.print(tempLeida);
    Serial.println("°C");
  } else {
    Serial.println("Error: DS18B20 desconectado");
  }

  Serial.println("-----------------------");

  //PID Cama
  if (tempLeida != DEVICE_DISCONNECTED_C) {
    Input = (double)tempLeida;
    
    myPID.Compute();

    analogWrite(pinCalentador, Output);

    Serial.print("Setpoint: "); Serial.print(Setpoint);
    Serial.print(" | Actual: "); Serial.print(Input);
    Serial.print(" | PWM Out: "); Serial.println(Output);
  }
    delay(1000);
}