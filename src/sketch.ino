/* ############################################# ESP32 JUKEBOX ############################################# 
 * ESP32 Jukebox Project
 * 
 * A music player with LED visualization, LCD display, and volume control
 * Uses shift registers for LED bar graph display
 * 
 * Author: PfJn
 * Version: 2.0
 */

/* ############################################# LIBRARIES ############################################# */
#include <LiquidCrystal_I2C.h>

/* ############################################# CONSTANTS ############################################# */
// Pin Definitions
#define LED_PIN         15
#define LED_BTN_PIN     23
#define BUZZER_PIN      18
#define BUZZER_BTN_PIN  19
#define DATA_PIN        2  // Pin connected to DS of 74HC595
#define LATCH_PIN       4  // Pin connected to STCP of 74HC595
#define CLOCK_PIN       5  // Pin connected to SHCP of 74HC595
#define POT_PIN         13 // Analog pin for potentiometer

// LCD Configuration
#define I2C_ADDR        0x27
#define LCD_COLUMNS     16
#define LCD_LINES       2
#define CHAR_DELAY      50 // milliseconds

// LED Bar Graph Configuration
#define NUM_SHIFT_REGS  2   // Number of shift registers
#define LED_COUNT       10  // Number of LEDs in the bar graph

// Timing Constants
#define DEBOUNCE_DELAY  50  // Button debounce delay in milliseconds
#define DISPLAY_REFRESH 100 // Display refresh rate in milliseconds

// Musical notes for buzzer
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
#define REST     0

/* ############################################# CLASS DECLARATIONS ################################### */

/**
 * @brief Button class for managing button input with debouncing
 */
class Button {
private:
  uint8_t pin;
  uint8_t lastState;
  uint8_t currentState;
  unsigned long lastDebounceTime;
  unsigned long debounceDelay;
  bool stateChanged;

public:
  /**
   * @brief Constructor for Button class
   * @param buttonPin GPIO pin for the button
   * @param debounceMs Debounce delay in milliseconds
   */
  Button(uint8_t buttonPin, unsigned long debounceMs = DEBOUNCE_DELAY) 
    : pin(buttonPin), 
      lastState(HIGH), 
      currentState(HIGH), 
      lastDebounceTime(0),
      debounceDelay(debounceMs),
      stateChanged(false) {
  }

  /**
   * @brief Initialize the button
   */
  void begin() {
    pinMode(pin, INPUT_PULLUP);
    currentState = digitalRead(pin);
    lastState = currentState;
  }

  /**
   * @brief Update button state and handle debouncing
   * @return true if button state has changed
   */
  bool update() {
    stateChanged = false;
    
    // Read current state
    uint8_t reading = digitalRead(pin);
    
    // Check if state has changed
    if (reading != lastState) {
      lastDebounceTime = millis();
    }
    
    // Check if debounce time has elapsed
    if ((millis() - lastDebounceTime) > debounceDelay) {
      // If state has actually changed
      if (reading != currentState) {
        currentState = reading;
        stateChanged = true;
      }
    }
    
    lastState = reading;
    return stateChanged;
  }

  /**
   * @brief Get current button state
   * @return Current state (HIGH or LOW)
   */
  uint8_t getState() {
    return currentState;
  }

  /**
   * @brief Check if button is pressed
   * @return true if button is pressed (LOW)
   */
  bool isPressed() {
    return currentState == LOW;
  }
};

/**
 * @brief LED class for controlling a single LED
 */
class Led {
private:
  uint8_t pin;
  bool state;

public:
  /**
   * @brief Constructor for LED class
   * @param ledPin GPIO pin for the LED
   */
  Led(uint8_t ledPin) : pin(ledPin), state(false) {
  }

  /**
   * @brief Initialize the LED
   */
  void begin() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    state = false;
  }

  /**
   * @brief Turn the LED on
   */
  void on() {
    digitalWrite(pin, HIGH);
    state = true;
  }

  /**
   * @brief Turn the LED off
   */
  void off() {
    digitalWrite(pin, LOW);
    state = false;
  }

  /**
   * @brief Toggle the LED state
   */
  void toggle() {
    state = !state;
    digitalWrite(pin, state ? HIGH : LOW);
  }

  /**
   * @brief Set LED state directly
   * @param newState New state to set (true = on, false = off)
   */
  void setState(bool newState) {
    if (state != newState) {
      state = newState;
      digitalWrite(pin, state ? HIGH : LOW);
    }
  }

  /**
   * @brief Get current LED state
   * @return Current state (true = on, false = off)
   */
  bool getState() {
    return state;
  }
};

/**
 * @brief Display class for managing LCD display
 */
class Display {
private:
  LiquidCrystal_I2C* lcd;
  uint8_t columns;
  uint8_t rows;

public:
  /**
   * @brief Constructor for Display class
   * @param address I2C address of LCD
   * @param cols Number of columns
   * @param rows Number of rows
   */
  Display(uint8_t address, uint8_t cols, uint8_t rows) 
    : columns(cols), rows(rows) {
    lcd = new LiquidCrystal_I2C(address, cols, rows);
  }

  /**
   * @brief Initialize the display
   */
  void begin() {
    lcd->init();
    lcd->backlight();
    lcd->clear();
    lcd->home();
  }

  /**
   * @brief Clear the display
   */
  void clear() {
    lcd->clear();
  }

  /**
   * @brief Print text at specific position
   * @param text Text to display
   * @param col Column (0-based)
   * @param row Row (0-based)
   */
  void print(const String& text, uint8_t col = 0, uint8_t row = 0) {
    lcd->setCursor(col, row);
    lcd->print(text);
  }

  /**
   * @brief Scroll text on a specific row
   * @param text Text to scroll
   * @param delayTime Delay between each scroll step in milliseconds
   * @param row Row to scroll text on (0-based)
   */
  void scrollText(const String& text, uint8_t delayTime, uint8_t row) {
    String paddedText = text;
    
    // Add padding at beginning and end
    for (uint8_t i = 0; i < columns; i++) {
      paddedText = " " + paddedText;
    }
    paddedText += " ";
    
    // Scroll the text
    for (uint8_t pos = 0; pos < paddedText.length() - columns; pos++) {
      lcd->setCursor(0, row);
      lcd->print(paddedText.substring(pos, pos + columns));
      delay(delayTime);
    }
  }
};

/**
 * @brief ShiftRegister class for controlling shift registers
 */
class ShiftRegister {
private:
  uint8_t dataPin;
  uint8_t clockPin;
  uint8_t latchPin;
  uint8_t numRegisters;
  uint8_t numPins;
  bool* registers;

public:
  /**
   * @brief Constructor for ShiftRegister class
   * @param data Data pin (DS)
   * @param clock Clock pin (SHCP)
   * @param latch Latch pin (STCP)
   * @param numRegs Number of daisy-chained shift registers
   */
  ShiftRegister(uint8_t data, uint8_t clock, uint8_t latch, uint8_t numRegs) 
    : dataPin(data), 
      clockPin(clock), 
      latchPin(latch), 
      numRegisters(numRegs),
      numPins(numRegs * 8) {
    registers = new bool[numPins]();  // Initialize all to 0
  }

  /**
   * @brief Destructor to clean up dynamic memory
   */
  ~ShiftRegister() {
    delete[] registers;
  }

  /**
   * @brief Initialize the shift registers
   */
  void begin() {
    pinMode(dataPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(latchPin, OUTPUT);
    clearAll();
    write();
  }

  /**
   * @brief Clear all shift register outputs
   */
  void clearAll() {
    for (uint8_t i = 0; i < numPins; i++) {
      registers[i] = LOW;
    }
  }

  /**
   * @brief Set a specific pin on the shift register
   * @param pin Pin number (0-based)
   * @param value Value to set (HIGH or LOW)
   */
  void setPin(uint8_t pin, bool value) {
    if (pin < numPins) {
      registers[pin] = value;
    }
  }

  /**
   * @brief Set multiple consecutive pins on the shift register
   * @param startPin First pin to set (0-based)
   * @param numPins Number of pins to set
   * @param value Value to set (HIGH or LOW)
   */
  void setPins(uint8_t startPin, uint8_t count, bool value) {
    for (uint8_t i = 0; i < count && (startPin + i) < numPins; i++) {
      registers[startPin + i] = value;
    }
  }

  /**
   * @brief Set a bar level on the shift register (for LED bar graphs)
   * @param level Level to display (0 to numPins)
   */
  void setLevel(uint8_t level) {
    clearAll();
    for (uint8_t i = 0; i < level && i < numPins; i++) {
      registers[i] = HIGH;
    }
  }

  /**
   * @brief Write values to the shift registers
   */
  void write() {
    digitalWrite(latchPin, LOW);
    
    for (int i = numPins - 1; i >= 0; i--) {
      digitalWrite(clockPin, LOW);
      digitalWrite(dataPin, registers[i]);
      digitalWrite(clockPin, HIGH);
    }
    
    digitalWrite(latchPin, HIGH);
  }
};

/**
 * @brief Buzzer class for playing tones and melodies
 */
class Buzzer {
private:
  uint8_t pin;
  bool playing;

public:
  /**
   * @brief Constructor for Buzzer class
   * @param buzzerPin GPIO pin for the buzzer
   */
  Buzzer(uint8_t buzzerPin) : pin(buzzerPin), playing(false) {
  }

  /**
   * @brief Initialize the buzzer
   */
  void begin() {
    pinMode(pin, OUTPUT);
    noTone(pin);
    playing = false;
  }

  /**
   * @brief Play a single tone
   * @param frequency Frequency in Hz
   * @param duration Duration in milliseconds (0 for continuous)
   */
  void playTone(uint16_t frequency, uint32_t duration = 0) {
    if (duration > 0) {
      tone(pin, frequency, duration);
      delay(duration);
      noTone(pin);
    } else {
      tone(pin, frequency);
    }
    playing = true;
  }

  /**
   * @brief Stop playing any tones
   */
  void stop() {
    noTone(pin);
    playing = false;
  }

  /**
   * @brief Check if buzzer is currently playing
   * @return True if buzzer is playing
   */
  bool isPlaying() {
    return playing;
  }

  /**
   * @brief Play the Star Wars Imperial March
   */
  void playStarWars() {
    // Notes and durations for Star Wars Imperial March
    int tempo = 100;
    int melody[] = {    
      // Darth Vader theme (Imperial March)
      NOTE_AS4,8, NOTE_AS4,8, NOTE_AS4,8,
      NOTE_F5,2, NOTE_C6,2,
      NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F6,2, NOTE_C6,4,  
      NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F6,2, NOTE_C6,4,  
      NOTE_AS5,8, NOTE_A5,8, NOTE_AS5,8, NOTE_G5,2, NOTE_C5,8, NOTE_C5,8, NOTE_C5,8,
      NOTE_F5,2, NOTE_C6,2,
      NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F6,2, NOTE_C6,4,  
      
      NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F6,2, NOTE_C6,4, 
      NOTE_AS5,8, NOTE_A5,8, NOTE_AS5,8, NOTE_G5,2, NOTE_C5,-8, NOTE_C5,16, 
      NOTE_D5,-4, NOTE_D5,8, NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F5,8,
      NOTE_F5,8, NOTE_G5,8, NOTE_A5,8, NOTE_G5,4, NOTE_D5,8, NOTE_E5,4,NOTE_C5,-8, NOTE_C5,16,
      NOTE_D5,-4, NOTE_D5,8, NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F5,8,
      
      NOTE_C6,-8, NOTE_G5,16, NOTE_G5,2, REST,8, NOTE_C5,8,
      NOTE_D5,-4, NOTE_D5,8, NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F5,8,
      NOTE_F5,8, NOTE_G5,8, NOTE_A5,8, NOTE_G5,4, NOTE_D5,8, NOTE_E5,4,NOTE_C6,-8, NOTE_C6,16,
      NOTE_F6,4, NOTE_DS6,8, NOTE_CS6,4, NOTE_C6,8, NOTE_AS5,4, NOTE_GS5,8, NOTE_G5,4, NOTE_F5,8,
      NOTE_C6,1
    };
    
    // Calculate number of notes
    int notes = sizeof(melody) / sizeof(melody[0]) / 2;
    
    // Calculate whole note duration
    int wholenote = (60000 * 4) / tempo;
    
    int divider = 0, noteDuration = 0;
    
    playing = true;
    
    // Play the melody
    for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {
      // Calculate note duration
      divider = melody[thisNote + 1];
      if (divider > 0) {
        // Regular note
        noteDuration = wholenote / divider;
      } else if (divider < 0) {
        // Dotted note
        noteDuration = wholenote / abs(divider);
        noteDuration *= 1.5;
      }
      
      // Play note for 90% of the duration
      tone(pin, melody[thisNote], noteDuration * 0.9);
      
      // Wait for the specified duration
      delay(noteDuration);
      
      // Stop the tone
      noTone(pin);
    }
    
    playing = false;
  }
};

/**
 * @brief VolumeControl class for handling potentiometer input and visualization
 */
class VolumeControl {
private:
  uint8_t pin;
  ShiftRegister* ledDisplay;
  uint8_t maxLevel;
  uint8_t currentLevel;

public:
  /**
   * @brief Constructor for VolumeControl class
   * @param potPin GPIO pin for the potentiometer
   * @param display Pointer to ShiftRegister for LED display
   * @param levels Number of volume levels (max LED count)
   */
  VolumeControl(uint8_t potPin, ShiftRegister* display, uint8_t levels) 
    : pin(potPin), 
      ledDisplay(display), 
      maxLevel(levels),
      currentLevel(0) {
  }

  /**
   * @brief Initialize the volume control
   */
  void begin() {
    // Potentiometer is an analog input, no pinMode needed on ESP32
    update();
  }

  /**
   * @brief Update volume level based on potentiometer position
   * @return true if volume level changed
   */
  bool update() {
    // Read potentiometer value (0-4095 on ESP32)
    int reading = analogRead(pin);
    
    // Map to volume level (0 to maxLevel)
    uint8_t newLevel = map(reading, 0, 4095, 0, maxLevel);
    
    // Update display if level changed
    if (newLevel != currentLevel) {
      currentLevel = newLevel;
      ledDisplay->setLevel(currentLevel);
      ledDisplay->write();
      return true;
    }
    
    return false;
  }

  /**
   * @brief Get current volume level
   * @return Current volume level (0 to maxLevel)
   */
  uint8_t getLevel() {
    return currentLevel;
  }
};

/**
 * @brief Jukebox class to coordinate all components
 */
class Jukebox {
private:
  // Components
  Button ledButton;
  Button buzzerButton;
  Led statusLed;
  Display display;
  ShiftRegister ledBar;
  Buzzer buzzer;
  VolumeControl volume;
  
  // State
  bool active;
  unsigned long lastUpdateTime;

public:
  /**
   * @brief Constructor for Jukebox
   */
  Jukebox() 
    : ledButton(LED_BTN_PIN),
      buzzerButton(BUZZER_BTN_PIN),
      statusLed(LED_PIN),
      display(I2C_ADDR, LCD_COLUMNS, LCD_LINES),
      ledBar(DATA_PIN, CLOCK_PIN, LATCH_PIN, NUM_SHIFT_REGS),
      buzzer(BUZZER_PIN),
      volume(POT_PIN, &ledBar, LED_COUNT),
      active(false),
      lastUpdateTime(0) {
  }

  /**
   * @brief Initialize the jukebox
   */
  void begin() {
    // Initialize serial for debugging
    Serial.begin(115200);
    Serial.println(F("ESP32 Jukebox starting..."));
    
    // Initialize components
    ledButton.begin();
    buzzerButton.begin();
    statusLed.begin();
    display.begin();
    ledBar.begin();
    buzzer.begin();
    volume.begin();
    
    // Display welcome message
    showWelcomeMessage();
    
    active = true;
    Serial.println(F("Jukebox initialized successfully"));
  }

  /**
   * @brief Display welcome message
   */
  void showWelcomeMessage() {
    static const String welcome = "ESP32 Jukebox";
    static const String author = "By PfJn";
    static const String ready = "Ready to play!";
    
    // Display welcome message with scrolling
    display.clear();
    display.scrollText(welcome, CHAR_DELAY, 0);
    display.scrollText(author, CHAR_DELAY, 1);
    
    // Flash LED
    for (int i = 0; i < 3; i++) {
      statusLed.on();
      delay(200);
      statusLed.off();
      delay(200);
    }
    
    // Display ready message
    display.clear();
    display.print(ready, (LCD_COLUMNS - ready.length()) / 2, 0);
  }

  /**
   * @brief Update jukebox state
   */
  void update() {
    unsigned long currentTime = millis();
    
    // Update volume control and LED bar
    volume.update();
    
    // Check for button presses
    if (ledButton.update()) {
      if (ledButton.isPressed()) {
        statusLed.toggle();
        display.clear();
        String message = "LED: " + String(statusLed.getState() ? "ON" : "OFF");
        display.print(message, 0, 0);
        Serial.println(message);
      }
    }
    
    if (buzzerButton.update()) {
      if (buzzerButton.isPressed()) {
        if (buzzer.isPlaying()) {
          buzzer.stop();
          display.clear();
          display.print("Music: OFF", 0, 1);
          Serial.println("Music stopped");
        } else {
          display.clear();
          display.print("Star Wars Theme", 0, 0);
          display.print("Playing...", 0, 1);
          Serial.println("Playing Star Wars theme");
          buzzer.playStarWars();
        }
      }
    }
    
    // Periodic display update
    if (currentTime - lastUpdateTime >= DISPLAY_REFRESH) {
      // Update volume display
      display.print("Volume: " + String(volume.getLevel()), 0, 1);
      lastUpdateTime = currentTime;
    }
  }
};

/* ############################################# GLOBAL INSTANCES ##################################### */
Jukebox jukebox;

/* #############################################   SETUP   ############################################# */
void setup() {
  // Initialize jukebox
  jukebox.begin();
}

/* ############################################# MAIN LOOP ############################################# */
void loop() {
  // Update jukebox state
  jukebox.update();
}