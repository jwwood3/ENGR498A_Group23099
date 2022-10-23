
int analogPin=A0;
int analogMax = 1024; // Maximum scale input currentPosue from the potentiometer
float target = 6.0; // Target position currentPosue in cm
double maxCM = 10.0; // Maximum position currentPosue in cm
double conversion = (1.0*analogMax)/(1.0*maxCM); // conversion rate from input currentPosues to cm
int moe = 15; // Acceptable error of approx 0.15 cm
void setup() {
  // Initialize output pins
  pinMode(13,OUTPUT);
  pinMode(10, OUTPUT);

  // Stop motor initially
  digitalWrite(13,HIGH);
  digitalWrite(10,HIGH);

  Serial.begin(9600);
}

void loop() {
  // Read in new target values for testing purposes
  if(Serial.available()){
    float temp = Serial.parseFloat(SKIP_WHITESPACE);
    if(temp>0){
      target=temp;
    }
  }
  // Get current position
  int currentPos = analogRead(analogPin);

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
