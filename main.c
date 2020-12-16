//Jenaro Vega
//jvega008@ucr.edu
//google doc: https://docs.google.com/document/d/1QbNZyImxCE3jiFuNk7_nI6JWl5ZePrax1Algd2QYSAo/edit?usp=sharing
//Youtube Video: https://youtu.be/Gjqo00mEAyI

#include <LiquidCrystal.h>
#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <HX711.h>
#include "pitches.h"

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
const int buttonPin = 10;  //button pin

#define redpin 3//
#define greenpin 5//
#define bluepin 6//


//led set
int led1 = 13;
int led2 = 9;

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

const byte LOADCELL_DOUT_PIN = 7;
const byte LOADCELL_SCK_PIN = 6;

float y1 = 355.0; // calibrated mass to be added
long x1 = 0L;
long x0 = 0L;
float avg_size = 10.0; // amount of averages for each mass measurement

HX711 hx711; // prep hx711

typedef struct Object{// Feature definitions
 char* category; // Object's category
 float weight; // A feature -- weight
 float r; // A feature -- red color
 float g; // A feature -- green color
 float b; // A feature -- blue color
};

//speaker setup
int melody[] = {
  
  // Hedwig's theme fromn the Harry Potter Movies
  // Socre from https://musescore.com/user/3811306/scores/4906610
  
  REST, 2, NOTE_D4, 4,
  NOTE_G4, -4, NOTE_AS4, 8, NOTE_A4, 4,
  NOTE_G4, 2, NOTE_D5, 4,
  NOTE_C5, -2, 
  NOTE_A4, -2,
  NOTE_G4, -4, NOTE_AS4, 8, NOTE_A4, 4,
  NOTE_F4, 2, NOTE_GS4, 4,
  NOTE_D4, -1, 
  NOTE_D4, 4,

  NOTE_G4, -4, NOTE_AS4, 8, NOTE_A4, 4, //10
  NOTE_G4, 2, NOTE_D5, 4,
  NOTE_F5, 2, NOTE_E5, 4,
  NOTE_DS5, 2, NOTE_B4, 4,
  NOTE_DS5, -4, NOTE_D5, 8, NOTE_CS5, 4,
  NOTE_CS4, 2, NOTE_B4, 4,
  NOTE_G4, -1,
  NOTE_AS4, 4,
     
  NOTE_D5, 2, NOTE_AS4, 4,//18
  NOTE_D5, 2, NOTE_AS4, 4,
  NOTE_DS5, 2, NOTE_D5, 4,
  NOTE_CS5, 2, NOTE_A4, 4,
  NOTE_AS4, -4, NOTE_D5, 8, NOTE_CS5, 4,
  NOTE_CS4, 2, NOTE_D4, 4,
  NOTE_D5, -1, 
  REST,4, NOTE_AS4,4,  

  NOTE_D5, 2, NOTE_AS4, 4,//26
  NOTE_D5, 2, NOTE_AS4, 4,
  NOTE_F5, 2, NOTE_E5, 4,
  NOTE_DS5, 2, NOTE_B4, 4,
  NOTE_DS5, -4, NOTE_D5, 8, NOTE_CS5, 4,
  NOTE_CS4, 2, NOTE_AS4, 4,
  NOTE_G4, -1, 
};
// change this to make the song slower or faster
int tempo = 400;

// change this to whichever pin you want to use
int buzzer = 8;


// sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int notes = sizeof(melody) / sizeof(melody[0]) / 2;

// this calculates the duration of a whole note in ms (60s/tempo)*4 beats
int wholenote = (60000 * 4) / tempo;

int divider = 0, noteDuration = 0;

 
  Object inputObject;
  char *closest_object_category;
  
void setup() {
 pinMode(buttonPin, INPUT);
 pinMode(led1, OUTPUT);
 pinMode(led2, OUTPUT);
 digitalWrite(led2, HIGH);//red
 digitalWrite(led1, LOW);//green
  lcd.begin(16, 2);
  hx711.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  Serial.begin(9600); // prepare serial port
  delay(1000); // allow load cell and hx711 to settle
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); // halt!
  }
  // tare procedure
  for (int ii=0;ii<int(avg_size);ii++){
    delay(10);
    x0+=hx711.read();
  }
  x0/=long(avg_size);
  Serial.println("Add Calibrated Mass");
   lcd.clear();
   lcd.setCursor(0, 0);
   lcd.print("Add Calibrated");
   lcd.setCursor(0, 1);
   lcd.print("Mass");
  // calibration procedure (mass should be added equal to y1)
  int ii = 1;
  while(true){
    if (hx711.read()<x0+10000){
    } else {
      ii++;
      delay(2000);
      for (int jj=0;jj<int(avg_size);jj++){
        x1+=hx711.read();
      }
      x1/=long(avg_size);
      break;
    }
  }
   lcd.clear();
   lcd.setCursor(0, 0);
   lcd.print("Calibration");
   lcd.setCursor(0, 1);
   lcd.print(" Complete");
  Serial.println("Calibration Complete");
  delay(5000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wait Please");
  PopulateKnownObjects();
///
 for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 95% of the duration, leaving 5% as a pause
    tone(buzzer, melody[thisNote], noteDuration*0.95);

    // Wait for the specief duration before playing the next note.
    delay(noteDuration);
    
    // stop the waveform generation before the next note.
    noTone(buzzer);
  }


  
}

void rgb(){
uint16_t clear, red, green, blue;
  tcs.getRawData(&red, &green, &blue, &clear);
 Serial.print("C:\t"); Serial.print(clear);
  Serial.print("\tR:\t"); Serial.print(red);
  Serial.print("\tG:\t"); Serial.print(green);
  Serial.print("\tB:\t"); Serial.print(blue);
 Serial.print("\n");
 
 inputObject.r = red; // input "B" is r
 inputObject.g = green; // input "C" is g
 inputObject.b = blue; // input "D" is b
}


float getWeight(){
  // averaging reading
  long reading = 0;
  for (int jj=0;jj<int(avg_size);jj++){
    reading+=hx711.read();
  }
  reading/=long(avg_size);
  // calculating mass based on calibration and linear fit
  float ratio_1 = (float) (reading-x0);
  float ratio_2 = (float) (x1-x0);
  float ratio = ratio_1/ratio_2;
  float mass = y1*ratio;

  Serial.print(mass);
  Serial.print("grams \n ");

  return mass;
}

bool checkItem(){ //checks if there is item to be classified
float weight1 = 0.0;
weight1 = getWeight();
if(weight1 > 1.5){
  return true;
}
else{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Place item");
  return false;
}

  
}

#define NUM_OF_CATEGORIES 3
int NUM_OF_KNOWN_OBJECTS = 16;
char* ObjectCategories[NUM_OF_CATEGORIES] = {"Aluminum Can", "Plastic bottle", "Glass bottle"};


// Classification definitions
#define K_Parameter 3 // Used in KNN algorithm
#define TRAINING_SET_SIZE 16

Object knownObjects[TRAINING_SET_SIZE];

void AddToKnownObjects(int i, char* category, float weight, float r, float g, float b) { // Add new object to the known objects array
 knownObjects[i].category = category;
 knownObjects[i].weight = weight;
 knownObjects[i].r = r;
 knownObjects[i].g = g;
 knownObjects[i].b = b;
 Serial.print("Added ");
 Serial.print(i);
 Serial.print("th object \n");
 

}

void PopulateKnownObjects() { // Insert all known objects into the known objects array ("training data")
 AddToKnownObjects(0, "Aluminum Can", 14.5, 65, 40, 37);
 AddToKnownObjects(1, "Aluminum Can", 15, 63, 41, 35);
 AddToKnownObjects(2, "Aluminum Can", 14.5, 61, 40, 37);
 AddToKnownObjects(3, "Aluminum Can", 16.1, 62, 42, 32);
 AddToKnownObjects(4, "Aluminum Can", 15.6, 60, 42, 35);
 AddToKnownObjects(5, "Aluminum Can", 14.9, 61, 41, 34);
 AddToKnownObjects(6, "Plastic bottle", 8.2, 30, 39, 34);
 AddToKnownObjects(7, "Plastic bottle", 8.9, 27, 36, 24);
 AddToKnownObjects(8, "Plastic bottle", 8.2, 28, 35, 30);
 AddToKnownObjects(9, "Plastic bottle", 8.5, 29, 37, 33);
 AddToKnownObjects(10, "Plastic bottle", 8.4, 25, 34, 29);
 AddToKnownObjects(11, "Glass bottle", 182.5, 16, 22, 17);
 AddToKnownObjects(12, "Glass bottle", 181.9, 31, 40, 37);
 AddToKnownObjects(13, "Glass bottle", 183.9, 32, 41, 35);
 AddToKnownObjects(14, "Glass bottle", 181.9, 32, 42, 38);
 AddToKnownObjects(15, "Glass bottle", 180.9, 30, 41, 39);
}

/* PHASE 1: FEATURE EXTRACTION */
// Extract features from sensors and create a new object with those features, default example below uses sensor values as features
void FeatureExtraction(Object &inputObject);
void FeatureExtraction(Object &inputObject) {
// Object inputObject;
 inputObject.weight = getWeight(); 
rgb();
Serial.print("Debug r: ");
Serial.print(inputObject.r);
Serial.print("\n");
Serial.print("Debug g: ");
Serial.print(inputObject.g);
Serial.print("\n");
Serial.print("Debug b: ");
Serial.print(inputObject.b);
Serial.print("\n");
 
}

/* PHASE 2: CLASSIFICATION */
float ComputeDistanceofObjects(Object object1, Object object2) { // Computes Euclidean distance between two objects for any # of dimensions.
 float weight = (object1.weight - object2.weight);
 float r = (object1.r - object2.r);
 float g = (object1.g - object2.g);
 float b = (object1.b - object2.b);
 float dist = sqrt(weight*weight + r*r + g*g + b*b);
 return dist;
}

void Sort(float *distances, char** categories) { // Sorts the provided distances from small to large// we are using bubble sort
 int i, j, n;
 float *temp1;
 char *temp2;
 n = NUM_OF_KNOWN_OBJECTS;  
    for (i = 0; i < n-1; i++){      
      
    // Last i elements are already in place  
    for (j = 0; j < n-i-1; j++){  
        if (distances[j] > distances[j+1]){  
          
            temp1 = &distances[j];
            distances[j] = distances[j+1];
            distances[j+1] = *temp1;

            temp2 = categories[j];
            categories[j] = categories[j+1];
            categories[j+1] = *temp2;
          }  
      }
    }
    
    Serial.print("Distances :");
    for(int k = 0; k <n; k++){
      Serial.print(distances[k]);
      Serial.print(" ");

    }
}

char* Classification(Object inputObject, Object knownObjects[]) { // KNN classification: Predicts the input object's category given known objects
 int count = 0, max_count = 0;
 char* most_frequent_category;
 
 Object kNearestObjects[K_Parameter]; // Maintains K nearest knownObjects
 float distances[NUM_OF_KNOWN_OBJECTS];
 char* categories[NUM_OF_KNOWN_OBJECTS];
 for(int i=0; i<NUM_OF_KNOWN_OBJECTS; ++i) { // Compute the distance of each known object to the input object
 distances[i] = ComputeDistanceofObjects(inputObject, knownObjects[i]);
 Serial.print("Distance between object ");
 Serial.print(i);
 Serial.print(": ");
 Serial.print(distances[i]);
 Serial.print("\n");
 categories[i] = knownObjects[i].category;
 }
 Serial.print("Before sort \n");
Sort(distances, categories); // Sort distances in ascending order
Serial.print("After sort \n");
 // For each category, determine if it's the most frequent among the K closest known objects
 for(int i=0; i<NUM_OF_CATEGORIES; ++i) {
 count = 0;
 for (int j=0; j<K_Parameter; ++j) { // Count frequency of this category in K closest objects
 if (categories[j] == ObjectCategories[i])
 count++;
 }
 if (count > max_count) { // Most frequent category so far
 max_count = count;
most_frequent_category = ObjectCategories[i];
 }
 }
 return most_frequent_category;
}

int Actuation(char* category) { // Turns on corresponding output bit to show category of the input object.
  if(category != "") {
    for (int i=0; i<NUM_OF_CATEGORIES; ++i) {
      if (category == ObjectCategories[i]) {
        return i;
      }
    }
  }
}



void loop() {
  buttonState = digitalRead(buttonPin);
  int index = 0;
  if((buttonState == HIGH) || checkItem()){
    digitalWrite(led2, HIGH);//red
    digitalWrite(led1, LOW);//green
    lcd.clear();
    FeatureExtraction(inputObject);
    closest_object_category = Classification(inputObject, knownObjects);
    index = Actuation(closest_object_category);
    lcd.setCursor(0,0);
    lcd.print(ObjectCategories[index]);
    delay(200);
  }
  else{
    digitalWrite(led2, LOW);//red
 digitalWrite(led1, HIGH);//green
  }
  
}

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST 0
