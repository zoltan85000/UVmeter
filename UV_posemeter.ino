/* UV and posmeter
project with:
MT8511
DS3231
SSD1306_128*64
Arduino ProMini 328P 3.3v 8MHz
*/

#include <SPI.h>
#include <DS3231.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
RTClib RTC;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

//Luxmeter
int photocellPin = 0; // the cell and 10K pulldown are connected to a0
int photocellReading; // the analog reading from the analog resistor divider


//uv posemeter
//Hardware pin definitions
int UVOUT = A0; //A1 ;//A5; //Output from the sensor
int REF_3V3 = A1; //A2 ;//A4; //3.3V power on the Arduino board
float  total_UV=0;

  int uvLevel;
 int refLevel;

 //Use the 3.3V power pin as a reference to get a very accurate output value from sensor
 float outputVoltage ;  
 float uvIntensity ;
float moy_minute;
int m_actuel=0;
float som_seconde = 0.0;



DS3231 Clock;
bool Century=false;
bool h12;
bool PM;
byte ADay, AHour, AMinute, ASecond, ABits;
bool ADy, A12h, Apm;

void setup() {

  // Start the I2C interface
  Wire.begin();
  // Start the serial interface
  Serial.begin(9600);
  Serial.print("SETup init...");
  
   pinMode(UVOUT, INPUT);
   pinMode(REF_3V3, INPUT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
    // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(500); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, WHITE);
    // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(100);

  display.clearDisplay();

  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(F("UV & luxmeter"));
  display.setCursor(0, 16);
  display.println(F("SETUP ATmega328P:"));
  display.setCursor(0, 24);
  display.println(F("DS3231 MP8511 SSD1306"));
  display.setCursor(0, 32);
  display.println(F("V1.0"));


  display.setCursor(0, 48);
  display.println(F("Rolland Philippe"));


  display.display();      // Show initial text
  delay(10000);
  ////// FIN setup
   Serial.println("...End");
}


void loop(void) {

 DateTime now = RTC.now();
    
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    //Serial.println();

    if (now.minute()!=m_actuel){
        moy_minute=som_seconde;
        som_seconde=0;
        m_actuel=now.minute();
      }
    else{
      som_seconde+=uvIntensity;
    }
    
    UV_measurment();
    display.clearDisplay();
    display.setCursor(0, 0); display.println(F("UV & Luxmeter V1.0"));
    display.setCursor(0, 8); display.println(F("Date:"));
    display.setCursor(30, 8); display.println(now.hour());
    display.setCursor(50, 8); display.println(now.minute());
    display.setCursor(70, 8); display.println(now.second());

    display.setCursor(0, 16); display.println(F("UV now:"));
    display.setCursor(60, 16); display.println(uvIntensity);

    display.setCursor(0, 24); display.println(F("UV total:"));
    display.setCursor(60, 24); display.println(total_UV);
    
    display.setCursor(0, 32); display.println(F("UV mn :"));
    display.setCursor(60, 32); display.println(moy_minute);
     
    display.display();      // Show initial text
    delay(100);
}

////////////////////////////////////
// UV
////////////////////////////////////

void UV_measurment(void){
   uvLevel = averageAnalogRead(UVOUT);
  refLevel = averageAnalogRead(REF_3V3);

 //Use the 3.3V power pin as a reference to get a very accurate output value from sensor
  outputVoltage = 3.3 / refLevel * uvLevel;  
  uvIntensity = mapfloat(outputVoltage, 0.99, 2.8, 0.0, 15.0);
 total_UV = total_UV + uvIntensity;
 Serial.print("   UV__MP8511  [output: ");
 Serial.print(uvLevel);
 Serial.print(" | voltage: ");
 Serial.print(outputVoltage);
 Serial.print(" | UV Intensity (mW/cm^2): ");
 Serial.print(uvIntensity);  
  Serial.print(" | Total UV Intensity (mW/cm^2): ");
   Serial.print(total_UV); 
     Serial.print(" ]");
 Serial.println();  
 delay(1000);
}


//Takes an average of readings on a given pin
//Returns the average
int averageAnalogRead(int pinToRead)
{
  byte numberOfReadings = 8;
  unsigned int runningValue = 0; 
  for(int x = 0 ; x < numberOfReadings ; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;
  return(runningValue);  
}
//The Arduino Map function but for floats
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

////////////////////////////////////
// posmeter no UV
////////////////////////////////////
void posmeter_v1(void){
  photocellReading = analogRead(photocellPin);
  Serial.print("Analog reading = ");
  Serial.print(photocellReading); // the raw analog reading
  // We'll have a few threshholds, qualitatively determined
  if (photocellReading < 10) {
    Serial.println(" - Noir");
  } else if (photocellReading < 200) {
    Serial.println(" - Sombre");
  } else if (photocellReading < 500) {
    Serial.println(" - Lumiere");
  } else if (photocellReading < 800) {
    Serial.println(" - Lumineux");
  } else {
    Serial.println(" - Tres lumineux");
  }
  delay(10000);
}
