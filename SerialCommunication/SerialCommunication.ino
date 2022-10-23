const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];
float* receivedData;
int analogs[] = {A0,A1,A2,A3,A4,A5,A6,A7,A8};
boolean newData = false;
int analogMax = 1024; // Maximum scale input currentPosue from the potentiometer
float target = 6.0; // Target position currentPosue in cm
double maxCM = 10.0; // Maximum position currentPosue in cm
double conversion = (1.0*analogMax)/(1.0*maxCM); // conversion rate from input currentPosues to cm
int moe = 15; // Acceptable error of approx 0.15 cm
int rec = 0;
void setup() {
  // Initialize output pins
  pinMode(13,OUTPUT);
  pinMode(10, OUTPUT);

  // Stop motor initially
  digitalWrite(13,HIGH);
  digitalWrite(10,HIGH);

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
    parseData();
    target = receivedData[0];
    Serial.print("New Target: ");
    Serial.println(target);
    newData = false;
  }
  // Get current position
  int currentPos = analogRead(analogs[0]);

  // Run motor up or down to reach the target position
  if(currentPos<(target*conversion)-moe){
    // Set motor to go up
    digitalWrite(13,LOW);
    digitalWrite(10,HIGH);
    Serial.print(currentPos);
    Serial.print(" : Need to go UP to ");
    Serial.println(target);
  }else if(currentPos>(target*conversion)+moe){
    // Set motor to go down
    digitalWrite(13,HIGH);
    digitalWrite(10,LOW);
    Serial.print(currentPos);
    Serial.print(" : Need to go DOWN to ");
    Serial.println(target);
  } else {
    // Set motor to stop
    digitalWrite(13,HIGH);
    digitalWrite(10,HIGH);
    Serial.print(currentPos);
    Serial.print(" : Need to go STOP to ");
    Serial.println(target);
  }
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
void parseData() {
  char * strtokIndx;

  strtokIndx = strtok(tempChars,",");
  int packageSize = atoi(strtokIndx);
  float dat[packageSize];
  receivedData = dat;
  for(int i=0;i<packageSize;i++){
    strtokIndx = strtok(NULL,",");
    receivedData[i] = atof(strtokIndx);
  }
}
