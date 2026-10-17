#include <Wire.h>               // Native Hardware Wire library works flawlessly on 1624
#include <Adafruit_GFX.h>       // Layout graphics generator
#include <Adafruit_SSD1306.h>   // Core OLED display driver

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// Pin parameters -1 means shared hardware system reset pin is omitted
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int LSM6DSO_ADDR = 0x6B;  // Default I2C address for LSM6DSO line (Change to 0x6A if SA0 pin is low)

void setup() {
  // 1. Fire up standard Hardware TWI/I2C (Maps directly to Pin 4 SCL / Pin 5 SDA on megaTinyCore)
  Wire.begin();
  
  // 2. Initialize OLED module
  // 0x3C is the standard factory hardwired address for small monochrome OLED panels
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    while(true); // Loop indefinitely if I2C bus wiring to screen fails
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("SYSTEM BOOTING..."));
  display.display();
  delay(500);

  // 3. Configure LSM6DSOETR3 registers manually via I2C commands
  // Awake Accelerometer: Turn on to 104 Hz polling rate, standard +/- 2g operating scale
  Wire.beginTransmission(LSM6DSO_ADDR);
  Wire.write(0x10); // CTRL1_XL control register address
  Wire.write(0x40); // Bit values setting ODR to 104Hz
  Wire.endTransmission();

  // Awake Gyroscope: Turn on to 104 Hz polling rate, 2000 dps operating scale
  Wire.beginTransmission(LSM6DSO_ADDR);
  Wire.write(0x11); // CTRL2_G control register address
  Wire.write(0x40); // Bit values setting ODR to 104Hz
  Wire.endTransmission();
}

void loop() {
  // Raw 16-bit register signed integers variable definitions
  int16_t raw_gx, raw_gy, raw_gz;
  int16_t raw_ax, raw_ay, raw_az;

  // 1. Direct block read of 12 sequential bytes of data starting from Gyro X register
  Wire.beginTransmission(LSM6DSO_ADDR);
  Wire.write(0x22); // OUTX_L_G (Gyro X-axis low byte starting address)
  Wire.endTransmission(false); // Issue a repeated start condition to keep control of the bus
  Wire.requestFrom(LSM6DSO_ADDR, 12, true);

  if (Wire.available() >= 12) {
    // Reconstruct low and high bytes into signed integers (Little Endian format)
    raw_gx = Wire.read() | (Wire.read() << 8);
    raw_gy = Wire.read() | (Wire.read() << 8);
    raw_gz = Wire.read() | (Wire.read() << 8);
    raw_ax = Wire.read() | (Wire.read() << 8);
    raw_ay = Wire.read() | (Wire.read() << 8);
    raw_az = Wire.read() | (Wire.read() << 8);
  }

  // 2. Mathematical scaling conversions using sensor factory sensitivity specifications
  // Standard acceleration multiplier for +/- 2g scale: 0.061 mg/LSB -> scale down to G metrics
  float ax_g = (raw_ax * 0.061) / 1000.0;
  float ay_g = (raw_ay * 0.061) / 1000.0;
  float az_g = (raw_az * 0.061) / 1000.0;

  // Standard gyroscope multiplier for 2000 dps scale: 70 mdps/LSB -> scale down to degrees/sec
  float gx_dps = (raw_gx * 70.0) / 1000.0;
  float gy_dps = (raw_gy * 70.0) / 1000.0;
  float gz_dps = (raw_gz * 70.0) / 1000.0;

  // 3. Clear graphics canvas and output real-time streaming matrices to the OLED 
  display.clearDisplay();
  display.setCursor(0, 0);
  
  // Custom text readout panel
  display.println(F("--- IMU ACTIVE TEST ---"));
  display.println();
  
  display.print(F("ACC X: ")); display.print(ax_g, 2);  display.println(F(" G"));
  display.print(F("ACC Y: ")); display.print(ay_g, 2);  display.println(F(" G"));
  display.print(F("ACC Z: ")); display.print(az_g, 2);  display.println(F(" G"));
  display.println(F("--------------------"));
  display.print(F("GYR X: ")); display.print(gx_dps, 1); display.println(F(" dps"));
  display.print(F("GYR Y: ")); display.print(gy_dps, 1); display.println(F(" dps"));
  display.print(F("GYR Z: ")); display.print(gz_dps, 1); display.println(F(" dps"));
  
  display.display(); // Draw to physical glass display panel
  delay(100); // 10Hz clean polling screen update rate limit
}

//AI-generated test firmware for only design submission purposes. no time is logged. 
//I was told using AI for this is okay here: https://hackclub.slack.com/archives/C0AV7EAB9K7/p1792185026634649

