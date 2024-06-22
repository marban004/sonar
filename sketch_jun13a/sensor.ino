#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>
const byte rxAddr[6] = "ggchtd"; //for pairing radios
const int trigger = 3;
const int echo = 4;
const int pwm = 2;
const int maxConeWidth = 135;
const int minConeWidth = 45;
const int timeBetweenAngleUpdate = 500;
const int timeBetweenPWMGeneration = 20;
const int timer1Max = 20;   //for servo PWM generation
const int timer2Max = 50;  //for angle update
const int timeToAck = 15;
const int noRetries = 0;
RF24 radio(9, 10);  //pins connected to CE and CS
int distanceCM = 1;
int angle = 90;
bool direction = true;
int timer1 = 0;
int timer2 = 0;

struct Data
{
  int angle;
  int distance;
};

void setup() 
{
  Serial.begin(9600);
  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(pwm, OUTPUT);

  TCCR0A = (1<<WGM01);        //Set the CTC (Clear Timer on Compare) mode, Timer/Counter Control Register A for timer 0 (TCCR0A)
  OCR0A = 249;                //Value for ORC0A for 1ms, Output Compare Register A for timer 0 (OCR0A), 16 MHz / 64 (from prescaler) = 250 000, 
                              //250 000 / 249 = +/- 1004, but counter starts from 0 instead of 1 so it actually is 250, which gives 1000 Hz (1 ms)
  
  TIMSK0|=(1<<OCIE0A);        //Set  the interrupt request
  sei();                      //Enable interrupt
  
  TCCR0B|=(1<<CS01);          //Set the prescale 1/64 clock, Timer/Counter Control Register B for timer 0 (TCCR0B)
  TCCR0B|=(1<<CS00);

  (void)radio.begin();
  radio.setRetries(timeToAck, noRetries);
  radio.openWritingPipe(rxAddr);
  radio.stopListening();
}

int UpdateDistanceCm(int trigger, int echo)
{
  digitalWrite(trigger, LOW);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  unsigned long duration = pulseIn(echo, HIGH);

  //(void)Serial.print( duration);
  //(void)Serial.print(' ');
  
  return ((duration * (0.034)) / 2);
}

void generateServoPWM(int angle, int pwm, int minConeWidth, int maxConeWidth)
{
  long timeInMicroseconds = map(angle, minConeWidth, maxConeWidth, 1000, 2000);
  digitalWrite(pwm, HIGH);
  delayMicroseconds(timeInMicroseconds);
  digitalWrite(pwm, LOW);
  //(void)Serial.print(timeInMicroseconds);
  //(void)Serial.print(' ');
}

void updateAngle(int* angle, int minConeWidth, int maxConeWidth)
{
  if(*angle <= minConeWidth || *angle >= maxConeWidth)
  {
    direction = !direction;
  }

  if(direction)
  {
    *angle = *angle + 1;
  }
  else
  {
    *angle = *angle - 1;
  }
  //(void)Serial.print(*angle);
  //(void)Serial.print(' ');
}

bool transferData(int angle, int distance)
{
  Data data;
  data.angle = angle;
  data.distance = distance;
  (void)radio.write(&data, sizeof(data));
}

ISR(TIMER0_COMPA_vect) //triggered every time OCR0A reaches 0 i. e. every millisecond
{
  timer1++;
  timer2++;
}

void loop() 
{
  if(timer2 >= timer2Max)
  {
    timer2 = 0; 
    updateAngle(&angle, minConeWidth, maxConeWidth);
  }
  distanceCM = UpdateDistanceCm(trigger, echo);
  if(timer1 >= timer1Max)
  {
    timer1 = 0;
    generateServoPWM(angle, pwm, minConeWidth, maxConeWidth);
  }
  (void)transferData(angle, distanceCM);
  (void)Serial.print(String(angle) + "," + String(distanceCM) + ",3;");
}

