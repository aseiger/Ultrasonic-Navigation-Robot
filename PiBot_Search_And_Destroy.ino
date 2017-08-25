const int pingPin_R = 6;
const int pingPin_L = 9;
const int echoPin_C = 7;
const int trigPin_C = 8;
const int bumperPin_L = 10;
const int bumperPin_R = 11;
const int motorPWM_L = 3;
const int motorDIR_L = 2;
const int motorPWM_R = 5;
const int motorDIR_R = 4;

typedef enum states{
  FORWARD,
  SPIN_L,
  SPIN_R,
  REV
} states;

int state = FORWARD;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);
}

void loop() {
  int leftDuration, centerDuration, rightDuration, bumperL, bumperR;
  int power_L = 150;
  int power_R = 150;
  double x_dur[32], y_dur[32];
  double x_mean = 0;
  double y_mean = 0;
  
  leftDuration = readPing(pingPin_L);
  centerDuration = readCenter(trigPin_C, echoPin_C);
  rightDuration = readPing(pingPin_R);
  bumperL = readBumper(bumperPin_L);
  bumperR = readBumper(bumperPin_R);
  
  //left bumper hit
  if(bumperL == 1){
    setMotors(-150, -150);
    delay(350);
    setMotors(-150, 150);
    delay(300);
    setMotors(150, 150);
  //right bumper hit
  }else if(bumperR == 1){
    setMotors(-150, -150);
    delay(350);
    setMotors(150, -150);
    delay(300);
    setMotors(150, 150);
  } else {
    //go forwards, avoiding with pings
    //move the filters down one tap
    for(int i = 0; i<30; i++){
      x_dur[i+1] = x_dur[i];
      y_dur[i+1] = y_dur[i];
    }
    
    x_dur[0] = -.707*(double)leftDuration+.707*(double)rightDuration;
    y_dur[0] = .707*(double)leftDuration+.707*(double)rightDuration+centerDuration;
    
    //calculate the mean of the delay line
    for(int i = 0; i<31; i++){
      x_mean = (x_mean + x_dur[i])/2;
      y_mean = (y_mean + y_dur[i])/2;
    }
    
    power_R = 80-(x_mean/30)+(y_mean/100);
    power_L = 80+(x_mean/30)+(y_mean/100);
    
    //institute bounds so overflow conditions do not cause erratic driving
    if(power_R > 255){
      power_R = 255;
    }else if(power_R < -255){
      power_R = -255;
    }
    
    if(power_L > 255){
      power_L = 255;
    }else if(power_L < -255){
      power_L = -255;
    }
    
    //setMotors(0, 0);
    setMotors(power_R, power_L);
  }
  
  Serial.print(x_mean/20);
  Serial.print("X, ");
  Serial.print(y_mean/100);
  Serial.println("Y, ");
  
//  Serial.print(leftDuration);
//  Serial.print("L, ");
//  Serial.print(centerDuration);
//  Serial.print("C, ");
//  Serial.print(rightDuration);
//  Serial.print("R, ");
//  Serial.print(bumperL);
//  Serial.print("BumperL, ");
//  Serial.print(bumperR);
//  Serial.print("BumperR");
//  Serial.println();
//  
//  delay(100);  
}

void setMotors(int powerL, int powerR){
  int pwmL, pwmR, dirL, dirR;
  pinMode(motorPWM_L, OUTPUT);
  pinMode(motorPWM_R, OUTPUT);
  pinMode(motorDIR_L, OUTPUT);
  pinMode(motorDIR_R, OUTPUT);
  
  if(powerL > 0){
    pwmL = 255 - powerL;
    dirL = 1;
  }else if(powerL < 0){
    pwmL = -powerL;
    dirL = 0;
  }else{
    pwmL = 0;
    dirL = 0;
  }
  
  if(powerR > 0){
    pwmR = 255 - powerR;
    dirR = 1;
  }else if(powerR < 0){
    pwmR = -powerR;
    dirR = 0;
  }else{
    pwmR = 0;
    dirR = 0;
  }
  
  analogWrite(motorPWM_L, pwmL);
  analogWrite(motorPWM_R, pwmR);
  digitalWrite(motorDIR_L, dirL);
  digitalWrite(motorDIR_R, dirR);
  
}

long readPing(int pingPin){
  long duration;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH);
  
  return duration;
}

long readCenter(int trigPin, int echoPin){
  long duration;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(trigPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  
  duration = pulseIn(echoPin, HIGH);
  
  return duration;
}

int readBumper(int bumperPin){
  int bumperState;
  pinMode(bumperPin, INPUT);
  bumperState = digitalRead(bumperPin);
  return bumperState;
}
  

long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}
