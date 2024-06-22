#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>
const byte rxAddr[6] = "ggchtd"; //for pairing radios
const int noAnglesToSkip = 2;
const int noTriesToDiscon = 100;
const int noTriesToUnstable = 20;
const int noTriesToStable = 10;
const int delayBetweenTransfers = 25;
int prevAngle = 90;
int currAngle = 90;
int distance = 0;
int transmissionState = 0;
long counterOfFailure = 0L;
int counterOfSuccess = 0;
String data = "";
RF24 radio(9, 10);

struct Data
{
  int angle;
  int distance;
};


void setup() {
  Serial.begin(9600);
  (void)radio.begin();
  radio.openReadingPipe(0, rxAddr);
  radio.startListening();
  reciveDataAndCheckIntegrity(&currAngle, &distance, &counterOfFailure, &counterOfSuccess, &transmissionState);
  currAngle = prevAngle;
}

bool transferData(int* angle, int* distance)
{
  Data data;
  if(radio.available())
  {
    radio.read(&data, sizeof(data));
    *angle = data.angle;
    *distance = data.distance;
    return true;
  }
  else
  {
    return false;
  }
}

void reciveDataAndCheckIntegrity(int* currAngle, int* distance, long* counterOfFailure, int* counterOfSuccess, int* transmissionState)
{
  if(transferData(currAngle, distance))
  {
    //(void)Serial.print((*counterOfSuccess));
    //(void)Serial.print(' ');
    (*counterOfSuccess)++;
    *counterOfFailure = 0;
    if((*counterOfSuccess) > noTriesToStable)
    {
      (*counterOfSuccess)--;
      *transmissionState = 3;
    }
  }
  else
  {
    (*counterOfFailure)++;
    *counterOfSuccess = 0;
    if((*counterOfFailure) > noTriesToUnstable)
    {
      *transmissionState = 2;
    }
  }
}

void loop() {
  prevAngle = currAngle;
  reciveDataAndCheckIntegrity(&currAngle, &distance, &counterOfFailure, &counterOfSuccess, &transmissionState);
  if(currAngle >= prevAngle + noAnglesToSkip || currAngle <= prevAngle - noAnglesToSkip)
  {
    transmissionState = 1;
  }

  if(counterOfFailure >= noTriesToDiscon)
  {
    counterOfFailure--;
    transmissionState = 0;
  }
  data = String(currAngle) + "," + String(distance) + "," + String(transmissionState) + ";";
  if(Serial.availableForWrite())
  {  
    (void)Serial.print(data);
  }
  delay(delayBetweenTransfers);
}
