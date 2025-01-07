#include <dht.h>
#define DHT22_PIN 5
#define USE_ARDUINO_INTERRUPTS true    // Para habilitar interrupções
#include <PulseSensorPlayground.h>    // Inclui a biblioteca

dht DHT;

// Inicializa o objeto do PulseSensor
PulseSensorPlayground pulseSensor;

// Configuração do sensor
const int PULSE_INPUT = A0;          // Pino analógico onde o sensor está conectado
const int THRESHOLD = 550;          // Sensibilidade (ajuste conforme necessário)

void ler_DHT22();
void pulse_sensor();

struct {
  uint32_t total;
  uint32_t ok;
  uint32_t crc_error;
  uint32_t time_out;
  uint32_t connect;
  uint32_t ack_l;
  uint32_t ack_h;
  uint32_t unknown;
} stat = { 0, 0, 0, 0, 0, 0, 0, 0};


void setup() {
  Serial.begin(115200); // Inicializa a comunicação serial uma única vez
  Serial.println("dht22_test.ino");
  Serial.print("LIBRARY VERSION: ");
  Serial.println(DHT_LIB_VERSION);
  Serial.println();
  Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)\tTime (us)");

  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.setThreshold(THRESHOLD);

  if (pulseSensor.begin()) {
    Serial.println("Pulse Sensor inicializado!");
  }
}


void loop() {
  // put your main code here, to run repeatedly:
  ler_DHT22();
  pulse_sensor();

}

void pulse_sensor(){
  
  int bpm = pulseSensor.getBeatsPerMinute();  // Lê os batimentos por minuto
  if (pulseSensor.sawStartOfBeat()) {
    Serial.print("Batimento detectado! BPM: ");
    Serial.println(bpm);
  }
  delay(20);  // Aguarda para suavizar leituras
  
}
void ler_DHT22() {
  Serial.print("DHT22, \t");

  uint32_t start = micros();
  int chk = DHT.read22(DHT22_PIN);
  uint32_t stop = micros();

  stat.total++;
  switch (chk)
  {
    case DHTLIB_OK:
      stat.ok++;
      Serial.print("OK,\t");
      break;
    case DHTLIB_ERROR_CHECKSUM:
      stat.crc_error++;
      Serial.print("Checksum error,\t");
      break;
    case DHTLIB_ERROR_TIMEOUT:
      stat.time_out++;
      Serial.print("Time out error,\t");
      break;
    case DHTLIB_ERROR_CONNECT:
      stat.connect++;
      Serial.print("Connect error,\t");
      break;
    case DHTLIB_ERROR_ACK_L:
      stat.ack_l++;
      Serial.print("Ack Low error,\t");
      break;
    case DHTLIB_ERROR_ACK_H:
      stat.ack_h++;
      Serial.print("Ack High error,\t");
      break;
    default:
      stat.unknown++;
      Serial.print("Unknown error,\t");
      break;
  }
  // DISPLAY DATA
  Serial.print(DHT.humidity, 1);
  Serial.print(",\t");
  Serial.print(DHT.temperature, 1);
  Serial.print(",\t");
  Serial.print(stop - start);
  Serial.println();

  if (stat.total % 20 == 0)
  {
    Serial.println("\nTOT\tOK\tCRC\tTO\tCON\tACK_L\tACK_H\tUNK");
    Serial.print(stat.total);
    Serial.print("\t");
    Serial.print(stat.ok);
    Serial.print("\t");
    Serial.print(stat.crc_error);
    Serial.print("\t");
    Serial.print(stat.time_out);
    Serial.print("\t");
    Serial.print(stat.connect);
    Serial.print("\t");
    Serial.print(stat.ack_l);
    Serial.print("\t");
    Serial.print(stat.ack_h);
    Serial.print("\t");
    Serial.print(stat.unknown);
    Serial.println("\n");
  }
  delay(2000);
}
