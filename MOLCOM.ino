/*
 * PROGRAM SEQUENCE
 * 1. When turned on display the name of the company and the project
 * 2. Check if there is an information in the EEPROM memory (i.e the meter_no). If there is retrieve it and display on the screen
 * 3. If no content is in the EEPROM. Prompt the user to enter the meter_no
 * 4. Validate that meter_no entered is 11
 * 5. Configure the 'C' key for clearing the input one step back, in a case of mistake.
 * 6. Configure the 'A' key for entering the meter_no into the EEPROM. And clear the screen.
 * 7. Configure the geolocation based on the latitiude and longitude. Display the meter_no and geolocation on the screen.
 * 8. Configure key 'B' when pressed will display a screen where the information as regards hour of power can be retrived, 
 *    It should be in such a way that when you fill the day, it will pop up the next day you want and perform the calculation of hours.
*/
#include <RTClib.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <TinyGPS.h>
#include <SPI.h>
#include <SD.h>

//initializing the external EEPROM

#define EEPROM_address 0x50
unsigned int eepromAdd = 0;
char newStr[18];
byte i;
int lastIndex = 0;
//initializing the RTC 
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// setting the pin for the ChipSelect
const int chipSelect = 53;

//Setting the parameter of the geolocation sensor
// Rx and Tx of GPS Sensor to the 18(Tx) and 19(Rx) of the Arduino
float lat, lon;
TinyGPS gps;
// setting the parameters for the keypad
const int Size = 6;
char special_keys[Size] = {'A','B','C','D','*','#'};
String year_final, month_final, day_final, hour_final, minute_final, final_duration;
String year_initial, month_initial, day_initial, hour_initial, minute_initial, initial_duration;
char meter_no[11];
int count = 0;
String time_power_keeper;
int count_holder; // This variable holds the value of the count at any time there is loss of power, so that a memory continual recounting can be done.
byte valu;
char holder[11]; 
char meter_no_confirm[11];
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = 
{
 {'1','2','3','A'},
 {'4','5','6','B'},
 {'7','8','9','C'},
 {'*','0','#','D'}
  };
byte rowPins[ROWS] = {A0, A1, A2, A3}; //connect to the row R1-R4 pinouts of the keypad
byte colPins[COLS] = {A4, A5, A6, A7}; //connect to the column C1-C4 pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);


//setting parameters for the LCD
// Since I will be writing to the LCD the RW pin need not be grounded.
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);  // RS,EN,D4,D5,D6,D7


// Setting the pinmode for the Red, Yellow and Blue phases. These three pins are Inputs

int Yphase_pin = A8;
int Rphase_pin = A9;
int Bphase_pin = A10;

File myFile; // Create a Directory in the Mscard where the data will be stored

void setup() {
  Serial.begin(9600);
  SD.begin();
  Wire.begin();
  //Serial1.begin(9600);
  pinMode(Yphase_pin, INPUT);
  pinMode(Rphase_pin, INPUT);
  pinMode(Bphase_pin, INPUT);

  
  // set up the LCD's number of columns and rows:
  //initial_eeprom_clearing();
  lcd.begin(20, 4);
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // Print a welcome message to the screen, displaying the company name and the name of the project
  lcd.print("MOLCOM MULTICONCEPTS");
  lcd.setCursor(2,1);
  lcd.print("POWER CONSUMPTION");
  lcd.setCursor(6,2);
  lcd.print("DURATION");
  delay(3000);
  lcd.clear();
  get_initial_time();
  check_meter_no_eeprom();
  // Immediately the system starts get the time and store in a variable
  delay(1000);
  loop();
}

void loop() {
  get_USB_query();
  // Code block for the Geolocation sensor
  
//   while(Serial1.available()){ // check for gps data
//    if(gps.encode(Serial1.read()))// encode gps data
//    { 
//    gps.f_get_position(&lat,&lon); // get latitude and longitude
//    
//    //Latitude
//    lcd.setCursor(0,2);
//    lcd.print("Latitude:");
//    lcd.setCursor(10,2);
//    lcd.print(lat,6);
//    
//    //Longitude
//    lcd.setCursor(0,3);
//    lcd.print("Longitude:");
//    lcd.setCursor(11,3);
//    lcd.print(lon,6);
//   }
//  }
  /* SEQUENCE OF OPERATION FROM THIS POINT
    * If a high is detected in any of the three(3) phases then using a OR logic, activate the buzzer for 1000ms and deactivate
    * create a variable that will hold the date, and day of the week
    * create a variable that will hold the counts to mark the periods there was power from any of the 3phases
    * In the loop process each time the variable holding the day confirms with the RTC reading that the day or date is not thesame, 
    * take the value of count stored in the predefined variable and with that date push it to the EEPROM, Memory_Card,to save it.
    * After that set back the counter variable to integer 0 and the date or day variable to an empty string.
    * Send the data in the EEPROM immediately to the server via the GSM/GPRS module.
   */
   RYB_Phase_checker();
}


  void get_initial_time(){
  
  DateTime now = rtc.now();
  year_initial= now.year();
  month_initial = now.month();
  day_initial = now.day();
  hour_initial = now.hour();
  minute_initial = now.minute();
  initial_duration = year_initial+month_initial+day_initial+hour_initial+minute_initial;
  lcd.setCursor(0,0);
  lcd.print(initial_duration);
  delay(2000);
 
}

void check_meter_no_eeprom(){
    // Check for meter_no in EEPROM. If meter_no exist display it to the screen, else print no meter_no, call set_meter_no()
    for(int address = 0; address < 11; address++){
     valu = EEPROM.read(address);
     holder[address] = valu;
    }
    if(holder[0]!=0){
      lcd.setCursor(0,0);
      lcd.print("Meter_no:");
      lcd.setCursor(9,0);
      lcd.print(holder);
      lcd.setCursor(0,1);
      lcd.print("Bearing Location");
       
      }
     else{
      lcd.setCursor(1,0);
      lcd.print("NO METER NUMBER");
      warning_signal();
      delay(3000);
      lcd.clear();
      set_meter_no();
      }
    }

 void warning_signal(){ 
      // Initial warning that the meter no has to be a valid number and not a character.
      lcd.setCursor(4, 1);
      lcd.print("WARNING !!!");
      lcd.setCursor(2,2);
      lcd.print("METER NUMBERS ARE");
      lcd.setCursor(0,3);  
      lcd.print("NUMERAL NOT ALPHABET");
      }

  void set_meter_no() {
  //Enter the meter_no here
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("METER CONFIGURATION");
  lcd.setCursor(0,1);
  lcd.print("ENTER METER_NO:");
  enter_num();
  }

void enter_num(){
    // Call entry method that takes in the numbers
    
    entry();
    
   // while entry is satisfied if A is pressed display successfully
    while(entry){
       char key = keypad.getKey();
       if(key=='A'){
          second_meter_no_auth();
          while(second_meter_no_auth){
            char key = keypad.getKey();
            if(key=='A'){
              //compare the both inputs for the meter_no
              //By calling a function authenticate meter_no
              
              authenticate_meter_no();
              }
      
              }
           }
       }
    }
 
void entry(){
    // Within the context of inputting the meter_no if A,B,C,D,*,# is pressed, display invalid character. 
    // But if after the entering the value if A is pressed it means enter
    int limit_indicator = 11;
    int initial_indicator = 0;
    byte col = 2;
    byte row = 2;
    lcd.setCursor(col,row);
      while(initial_indicator<limit_indicator){
      char key = keypad.getKey();
         if(key){
          meter_no[initial_indicator++] = key;
          lcd.print(key);
          }
       // Here i am flagging against entering Alphabets as a valid meter number. Hence, you can only enter Numerals not alphabets
       for(int i = 0; i<Size; i++){
        if(key==special_keys[i]){
          if(key!='C'){
          lcd.setCursor(0,3);
          lcd.print("Invalid Character");
          lcd.clear();
          warning_signal();
          // After showing the warning it is supposed to direct me to where to set_meter_no()
          delay(3000);
          set_meter_no();
          
          }
          else if(key=='C'){
            lcd.setCursor(col,row);
            col =col-1;
            
            }
          }
       }
      }
      }
      
 void second_meter_no_auth(){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("VERIFYING METER_NO");
      lcd.setCursor(0,1);
      lcd.print("Re-enter meter_no");
      lcd.setCursor(2,2);
      int limit_indicator_2 = 11;
      int initial_indicator_2 = 0;
      while(initial_indicator_2<limit_indicator_2){
         char key2 = keypad.getKey();
            if(key2){
            meter_no_confirm[initial_indicator_2++] = key2;
            lcd.print(key2);
      }
      for(int i = 0; i<Size; i++){
        if(key2==special_keys[i]){
          lcd.setCursor(0,3);
          lcd.print("Invalid Character");
          lcd.clear();
          warning_signal();
          // After showing the warning it is supposed to direct me to where to set_meter_no()
          delay(3000);
          set_meter_no();
          
          }
       }
      }
    }

  void authenticate_meter_no(){
    // Here we check if the number enter previously and that enter latter was thesame, after that we authenticate.
    if(!(strncmp(meter_no, meter_no_confirm,11))){
      lcd.clear();
      lcd.setCursor(2,1);
      lcd.print("SAVING METER_NO");
      delay(1000);
      push_to_EEPROM();
      }
     else{
      lcd.clear();
      lcd.setCursor(2,2);
      lcd.print("INVALID METER NO");
      }
    }

  void push_to_EEPROM(){
    //The length of the EEPROM is 4096 = 4kb
    int address = 0;
     for(int i=0; i<11; i++){
      EEPROM.write(address, meter_no[i]);
      address = address+1;
      }
      lcd.setCursor(2,2);
      lcd.print("SUCCESSFULLY SAVED");
      lcd.setCursor(4,3);
      lcd.print("METER_NO");
    
    }

  void RYB_Phase_checker(){
    // This method checks the 3 phases for power 
    if (digitalRead(Yphase_pin) == HIGH || digitalRead(Rphase_pin) == HIGH || digitalRead(Bphase_pin) == HIGH){
      activate_rtc();
      if(final_duration == initial_duration){
      count_holder = count++;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(count_holder);
      delay(900);
      } 
      else{
        // the else statement means that they are not equal, that means we have entered a new day, and count and count_holder has to be reinitialized back to 0
        count_holder;
        // Concatenate the DATE and the power duration recorded as comma separated string
        time_power_keeper = (initial_duration + "," + count_holder);
        lcd.setCursor(0,1);
        lcd.print(time_power_keeper);  // I did this just for visual presentation on the LCD.

                          // SAVING DATA TO THE MICR0-SDCARD
                             
        save_MScard();
        delay(500);
      
                          // WRITING DATA TO THE EEPROM
                          
        int data_length = lastIndex + time_power_keeper.length();
        for(int i = lastIndex, j = 0 ; i < data_length; i++, j++){
          int retAddr = writeEEPROM(EEPROM_address, eepromAdd+i, time_power_keeper[i]);
          lastIndex = retAddr+1;
        } 
        Serial.print(lastIndex);

                          // READING DATA FROM THE EEPROM
        
        for(int j = 0 ; j < ; j++)
        newStr[j] = readEEPROM(EEPROM_address, j);
        Serial.print("Duration of power");
        Serial.print(newStr);
        delay(1000);
                         // SENDING DATA TO THE SERVER OR MOBILE NUMBER
                         
        send_data_toServer();
        
                        // REINITIALIZING VARIABLES AND LCD PRINTING
        initial_duration = final_duration;
        count = 0;
        count_holder = 0;
        lcd.setCursor(0,3);
        lcd.print(initial_duration);
        delay(2000);
        }

    }

    }

 void activate_rtc(){
  // Get the day when the 3phases pin were high
  // create a variable to hold the string or character for the day whatever be the content of this variable will be checked against the value of the RTC each time.
  DateTime now = rtc.now();
  // We only need the day, month and year from the RTC for our problem. Hence i will leave behind the time section
  year_final = now.year();
  month_final = now.month();
  day_final = now.day();
  hour_final = now.hour();
  minute_final = now.minute();
  final_duration = year_final+month_final+day_final+hour_final+minute_final;
  
  // The variable holder will then be stored in the External EEPROM.
  // I will always compare the content of holder_i  to what the RTC gives at a particular instance that is holder
  }

  void save_MScard(){
    lcd.setCursor(0,2);
    lcd.print("I AM ON THE SD-CARD");
    // Save the count and the time to the MScard.
    myFile = SD.open("test.txt", FILE_WRITE);
    
    // If file is open write to it
    if(myFile){
      Serial.println("Writing to test.txt...");
      time_power_keeper = (initial_duration + "," + count_holder);
      myFile.println(time_power_keeper); // time_power_keeper is a string of data that needs to be written to the sd card.
      // close the file after writing to it
      myFile.close();
      Serial.print("done");
    }
    else{
      Serial.print("File not open");
      }

    myFile = SD.open("test.txt");
    if(myFile){
      Serial.print("test.txt");

      while(myFile.available()){
        Serial.write(myFile.read());
      }
      myFile.close();
    }
    
  }


  int writeEEPROM(int deviceaddress,unsigned int eeaddress , byte duration_data ){
    // Here we save the concatenated data to an External EEPROM
    //convert_string_to_char(datan);
    Wire.beginTransmission(deviceaddress);
    Wire.write((byte)(eeaddress >> 8)); // MSB
    Wire.write((byte)(eeaddress & 0xFF)); // LSB
    Wire.write(duration_data);
    Wire.endTransmission();
    return eeaddress;
    delay(5);
  }

  byte readEEPROM(int deviceaddress, unsigned int eeaddress){
    byte readByte = 0;
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress, 1); // The 1 here means read a byte
    if(Wire.available())
      readByte = Wire.read();
    return readByte;
  }

  void send_data_toServer(){
    // This function is used in sending the data to a central server using the GSM/GPRS module

  }

 void initial_eeprom_clearing(){
  // Function for initially clearing the EEPROM memory
  for(int i = 0 ; i < EEPROM.length() ; i++){
    EEPROM.write(i,0);
    }
  }

/* MODIFICATIONS
*  Put a code that will check if the EEPROM is initially filled with any value that is not the meter no, if so clear the EEPROM memory
*  Configure a key that will clear the input by a step backward.
*  A function that takes the second recorded and convert into an hour, minute depending.

*/
