const int NUM_MOTORS = 9;

//Data transfer
const int numChars = 2420;
char receivedChars[numChars];
char tempChars[numChars];
float receivedData[420];
boolean newData = false;

//Analog Position Scaling
int analogMax = 1023; // Maximum scale input value from the potentiometer
float target[NUM_MOTORS]; // Target position value in cm
double maxCM = 10.16; // Maximum position value in cm
double conversion = (1.0*analogMax)/(1.0*maxCM); // conversion rate from input values to cm
int moe = 20; // Acceptable error

//PINS
int analogs[NUM_MOTORS];
int PWMS[NUM_MOTORS];
int controlPin1s[NUM_MOTORS];
int controlPin2s[NUM_MOTORS];

//Motor Data
int viewMotor = 0;
int pwmVals[NUM_MOTORS];
int curPos[NUM_MOTORS];
int MAX_PWM=255;
int MIN_PWM=255;

//PID
float error[NUM_MOTORS];
float prevError[NUM_MOTORS];
float integ[NUM_MOTORS];
float deriv[NUM_MOTORS];
float control[NUM_MOTORS];

float kp = 5;
float kd = 0;
float ki = 0;

float curTime;
float prevTime=-1.0;
float deltaTime;

/* Checked ports

Good Ports
PWM: 13,12
Analog: 0,2,3

Bad Ports
PWM: 11
Analog: 1
*/

void setup() {
  int pwmPortOrder[] = {2,3,4,5,6,7,8,9,10,11,12,13};
  int analogPortOrder[] = {0,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
  //Initialize motor count dependent arrays
  for(int i=0;i<NUM_MOTORS;i++){
    target[i]=5.0;    
    PWMS[i]=pwmPortOrder[i];
    pwmVals[i]=0;
    analogs[i]=analogPortOrder[i];
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
    if(l==3 && fEqual(receivedData[0],-1.0)){ // If 3 floats are given and the f1=-1,
      if(fEqual(receivedData[1],-1.0)){ // If f2=-1, set viewMotor to f3
        viewMotor = (int)receivedData[2];
      } else { // else assign target f3 to motor f2
        int i = (int)(receivedData[1]);            // set target[f2]=f3
        for(int j=0;j<l;j++){
          Serial.print(j);
          Serial.print(": ");
          Serial.println(receivedData[j]);
        }
        if(i<NUM_MOTORS && i>=0){
          target[i]=receivedData[2];
          Serial.print("New Target for motor ");
          Serial.print(i);
          Serial.print(": ");
          Serial.println(target[i]);
        }
      }
    } else if(l==NUM_MOTORS){// If vals are given for all motors, set all targets to new values
      for(int i=0;i<l;i++){
        target[i] = receivedData[i];
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
    static int ndx = 0;
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
  for(int i=0;i<packageSize;i++){
    strtokIndx = strtok(NULL,",");
    receivedData[i] = atof(strtokIndx);
    Serial.print("received ");
    Serial.print(i);
    Serial.print(" as ");
    Serial.println(receivedData[i]);    
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
    } else if(pwmVal<MIN_PWM){
      pwmVal=0;
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
    pwmVals[i] = pwmVal;
    analogWrite(PWMS[i],pwmVal);
  }
}

bool fEqual(float f1,float f2){
  float precision = 0.00001;
  return fabs(f1-f2)<precision;
}

void debug(){
  Serial.print("target: ");
  Serial.print(int(target[viewMotor]*conversion));
  Serial.print("\t|\tpos: ");
  Serial.print(curPos[viewMotor]);
  Serial.print("\t|\tError: ");
  Serial.print(error[viewMotor]);
  Serial.print("\t|\tDeriv: ");
  Serial.print(deriv[viewMotor]);
  Serial.print("\t|\tInteg: ");
  Serial.print(integ[viewMotor]);
  Serial.print("\t|\tPWM: ");
  Serial.println(pwmVals[viewMotor]);
}
