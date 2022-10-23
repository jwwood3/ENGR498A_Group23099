const int NUM_MOTORS = 1;

//Data transfer
const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];
float* receivedData;
boolean newData = false;

//Analog Position Scaling
int analogMax = 1023; // Maximum scale input value from the potentiometer
float target[NUM_MOTORS]; // Target position value in cm
double maxCM = 10.16; // Maximum position value in cm
double conversion = (1.0*analogMax)/(1.0*maxCM); // conversion rate from input values to cm
int moe = 8; // Acceptable error

//PINS
int analogs[NUM_MOTORS];
int PWMS[NUM_MOTORS];
int controlPin1s[NUM_MOTORS];
int controlPin2s[NUM_MOTORS];

//Motor Data
int curPos[NUM_MOTORS];
int MAX_PWM=255;
int MIN_PWM=130;

//PID
float error[NUM_MOTORS];
float prevError[NUM_MOTORS];
float integ[NUM_MOTORS];
float deriv[NUM_MOTORS];
float control[NUM_MOTORS];

float kp = 3;
float kd = 0.02;
float ki = 4.0;

float curTime;
float prevTime=-1.0;
float deltaTime;

void setup() {
  //Initialize motor count dependent arrays
  for(int i=0;i<NUM_MOTORS;i++){
    target[i]=5.0;    
    PWMS[i]=2+i;
    analogs[i]=i;
    controlPin1s[i]=52-(2*i);
    controlPin2s[i]=53-(2*i);
    integ[i]=0.0;
    prevError[i]=-1.0;

    pinMode(PWMS[i],OUTPUT);
    pinMode(controlPin1s[i],OUTPUT);
    pinMode(controlPin2s[i],OUTPUT);
  }

  Serial.begin(9600);
  while(!Serial){
    ;
  }
  Serial.println("START");
}

void loop() {
  recvWithStartEndMarkers();
  // Read in new target values for testing purposes
  if(newData == true){
    strcpy(tempChars, receivedChars);
    int l=parseData();
    if(l==NUM_MOTORS){// If vals are given for all motors, set all targets to new values
      for(int i=0;i<l;i++){
        target[i] = receivedData[i];
        Serial.print("New Target for motor ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(target[i]);
      }
    } else if(l==3 && fEqual(receivedData[0],-1.0)){ // If 3 floats are given and the f1=-1,
      int i = (int)(receivedData[1]+0.5);            // set target[f2]=f3
      if(i<NUM_MOTORS){
        target[i]=receivedData[2];
        Serial.print("New Target for motor ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(target[i]);
      }
    }
    newData = false;
  }
  // Get current motor positions
  getPos();
  // Run motors up or down to reach the target position
  PID();
  motors();
  debug();
}
/*
*
* Written by Robin2, taken from
* https://forum.arduino.cc/t/serial-input-basics-updated/382007/3
*
*/
void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

/*
*
* Adapted from code by Robin2 from
* https://forum.arduino.cc/t/serial-input-basics-updated/382007/3
*
*/
int parseData() {
  char * strtokIndx;

  strtokIndx = strtok(tempChars,",");
  int packageSize = atoi(strtokIndx);
  float dat[packageSize];
  receivedData = dat;
  for(int i=0;i<packageSize;i++){
    strtokIndx = strtok(NULL,",");
    receivedData[i] = atof(strtokIndx);
  }
  return packageSize;
}

void getPos(){
  for(int i=0;i<NUM_MOTORS;i++){
    curPos[i] = analogRead(analogs[i]);
  }
}

void PID(){
  curTime = micros();
  if(fEqual(prevTime,-1.0)){
    prevTime=curTime-1000;
  }
  deltaTime = (curTime - prevTime)/1000000.0;
  prevTime = curTime;
  for(int i=0;i<NUM_MOTORS;i++){
    error[i] = float(curPos[i]) - float(target[i]*conversion);
    if(fEqual(prevError[i],-1.0)){
      prevError[i]=error[i];
    }    
    deriv[i] = (error[i] - prevError[i])/deltaTime;
    integ[i] = integ[i] + (error[i] * deltaTime);
    control[i] = (kp*error[i])+(kd*deriv[i])+(ki*integ[i]);
    prevError[i] = error[i];
  }
}

void motors(){
  for(int i=0;i<NUM_MOTORS;i++){
    int dir=(control[i]>0)-(control[i]<0);
    int pwmVal = (int)fabs(control[i]);
    if(pwmVal>MAX_PWM){
      pwmVal = MAX_PWM;
    }
    if(pwmVal<MIN_PWM && fabs(error[i])>moe){
      pwmVal = MIN_PWM;
    }
    if(dir==-1){
      digitalWrite(controlPin1s[i],LOW);
      digitalWrite(controlPin2s[i],HIGH);
    } else if(dir==1){
      digitalWrite(controlPin1s[i],HIGH);
      digitalWrite(controlPin2s[i],LOW);
    } else {
      digitalWrite(controlPin1s[i],LOW);
      digitalWrite(controlPin2s[i],LOW);
      pwmVal = 0;
    }
    analogWrite(PWMS[i],pwmVal);
  }
}

bool fEqual(float f1,float f2){
  float precision = 0.00001;
  return fabs(f1-f2)<precision;
}

void debug(){
  Serial.print("target: ");
  Serial.print(int(target[0]*conversion));
  Serial.print("\t|\tpos: ");
  Serial.println(curPos[0]);
  Serial.print("Error: ");
  Serial.print(error[0]);
  Serial.print("\t|\tDeriv: ");
  Serial.print(deriv[0]);
  Serial.print("\t|\tInteg: ");
  Serial.println(integ[0]);
}
