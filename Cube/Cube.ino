//
// LED cube variables
//
#define   C_REFRESH_RATE    1000
#define   C_LAYERS          26
#define   C_DATA            36
#define   C_CLKS            46
byte CubeMap[8][8] = {
  { B11111111, B10000001, B10000001, B10000001, B10000001, B10000001, B10000001, B11111111}, 
  { B10000001, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B10000001}, 
  { B10000001, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B10000001}, 
  { B10000001, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B10000001}, 
  { B10000001, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B10000001}, 
  { B10000001, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B10000001}, 
  { B10000001, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B10000001}, 
  { B11111111, B10000001, B10000001, B10000001, B10000001, B10000001, B10000001, B11111111}
};
boolean isSecondPackage = false;
boolean isInStreamMode = true;

//
// Entry point
//
void setup() {
  Serial.begin(115200);
  Serial.setTimeout(10);
  
  SetupRegisters();
  SetupLayers();
  
  ClearCube();
}

void SetupRegisters() {
  // Initialize Registers Clock pins
  byte endClock = C_CLKS + 8;
  for(byte i = C_CLKS; i < endClock; i++) {
    pinMode(i, OUTPUT); 
  }
  
  // Initialize Registers Data pins
  byte endData = C_DATA + 9;
  for(byte i = C_DATA; i < endData; i++) {
    pinMode(i, OUTPUT); 
  }
}

void SetupLayers() {
  // Initializes Layers pins
  byte endLayer = C_LAYERS + 8;
  for(byte i = C_LAYERS; i < endLayer; i++) {
    pinMode(i, OUTPUT);
  }
}

void ClearCube() {
  for(byte i = 0; i < 8; i++) {
    for(byte j = 0; j < 8; j++) {
      CubeMap[i][j] = 0; 
    }
  } 
}

//
// Application start
//
void loop() { 
  DrawCube();
//  RecieveSerial();  
}

void serialEvent() {
  if (Serial.available() > 0) {
    if (Serial.available() == 1) {
      byte command = Serial.read();
    
      // Software reset command
      if (command == 0x21) {
        asm volatile("jmp 0");  
      }
    }
    else if(isInStreamMode) {
      HandleSerialStream(); 
    }
    else Serial.println("Unknown command!");
  }  
}

void HandleSerialStream() {
  if (Serial.available() >= 32) {  
    int currentRow = 0;
    int currentLayer = 0;
    
    if (isSecondPackage) {
      currentLayer = 4;
    }
    
    isSecondPackage = !isSecondPackage;
    
    for(byte i = 0; i < 32; i++) {
      CubeMap[currentLayer][currentRow] = Serial.read();
      
      currentRow++;
      if (currentRow >= 8) {
        currentLayer++;
        currentRow = 0; 
      }
    }
    Serial.println("SUCC");    
   
    // Clear buffer
    while(Serial.available() > 0) { Serial.read(); }
  }
}

void DrawCube() {
  for(byte layer = 0; layer < 8; layer++) {
    // Draw layer from map
    DrawLayer(layer);
  }
}

void DrawLayer(byte layer) {
  // Set current map layer to registers
  for(byte row = 0; row < 8; row++) {
    if (row % 2 == 0) 
      SetRegDataInv(CubeMap[layer][row]);
    else SetRegData(CubeMap[layer][row]);
    ToggleClock(row);
  }
  
  // Toggles layer on and off with delay
  ToggleLayer(layer);
}

void SetRegDataInv(byte data) {
  for(byte index = 0; index < 8; index++) {
    digitalWrite(C_DATA + 7 - index, data & (1<<index));
  }
}

void SetRegData(byte data) {
  for(byte index = 0; index < 8; index++) {
    digitalWrite(C_DATA + index, data & (1<<index));
  }
}

void ToggleLayer(byte pinOffset) {
  // Get layer pin 
  byte layerPin = pinOffset + C_LAYERS;
  
  // Toggle layer with delay that cooresponds to
  // refresh rate of cube that is calculated like this:
  // 1/C_REFRESH_RATE*NUM_OF_LAYERS [us]
  digitalWrite(layerPin, HIGH); 
  delayMicroseconds(C_REFRESH_RATE);
  digitalWrite(layerPin, LOW);
}

void ToggleClock(byte pinOffset) {
  digitalWrite(C_CLKS + pinOffset, LOW);
  digitalWrite(C_CLKS + pinOffset, HIGH);
  digitalWrite(C_CLKS + pinOffset, LOW);
}
