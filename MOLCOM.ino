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


//initializing the RTC 
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Setting the parameter of the geolocation sensor
// Rx and Tx of GPS Sensor to the 18(Tx) and 19(Rx) of the Arduino
float lat, lon;
TinyGPS gps;
// setting the parameters for the keypad
const int Size = 6;
char special_keys[Size] = {'A','B','C','D','*','#'};
String year_h, month_h, day_h, holder_h;
String year_i, month_i, day_i, holder_i;
char meter_no[11];
byte valu;
char holder[11];
int count = 0; // Holds the count time for power duration.
int count_holder = 0; // This variable holds the value of the count at any time there is loss of power, so that a memory continual recounting can be done.
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
int buzzer_pin = 5;// The buzzer pin is the output pin

char dateday_holder; // Variable for holding the day and the date for a particular day

void setup() {
  Serial1.begin(9600);
  pinMode(Yphase_pin, INPUT);
  pinMode(Rphase_pin, INPUT);
  pinMode(Bphase_pin, INPUT);
  pinMode(buzzer_pin, OUTPUT);
  
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
  check_current_date_per_day();
  check_meter_no_eeprom();
  // Immediately the system starts get the time and store in a variable
  delay(1000);
  loop();
}

void loop() {
  // Code block for the Geolocation sensor
  
   while(Serial1.available()){ // check for gps data
    if(gps.encode(Serial1.read()))// encode gps data
    { 
    gps.f_get_position(&lat,&lon); // get latitude and longitude
    
    //Latitude
    lcd.setCursor(0,2);
    lcd.print("Latitude:");
    lcd.setCursor(10,2);
    lcd.print(lat,6);
    
    //Longitude
    lcd.setCursor(0,3);
    lcd.print("Longitude:");
    lcd.setCursor(11,3);
    lcd.print(lon,6);
   }
  }
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
void check_current_date_per_day(){
  DateTime now = rtc.now();
  year_i= now.year();
  month_i = now.month();
  day_i = now.day();
  holder_i = year_i.concat(month_i.concat(day_i));
  
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
    
    if ((digitalRead(Yphase_pin) == HIGH) || (digitalRead(Bphase_pin) == HIGH) || (digitalRead(Rphase_pin) == HIGH)){
      count_holder = count++;
      activate_rtc(); // i have to store this value once.
      if(holder_h != holder_i){
        /* Once there is power and the date stored is not equal, that means we have entered a new day
         * make the count = 0, to restart for the next day,
         * push the date and count_holder value into the EEPROM, Mscard, and send using the GSM module
         * we have the get the new date to use as a checking parameter, against the looping date
         * reinitialize count_holder = 0, for a new day.
        */
       count = 0;
       
        }
      } 

     else if ((digitalRead(Yphase_pin) == LOW) || (digitalRead(Bphase_pin) == LOW) || (digitalRead(Rphase_pin) == LOW)){
      /* Check if the we are still in the same day
       *  if we are still on the same day, take the value of the count_holder store and maintain it.
       *  if we have entered another day reintialize the count_holder to be equal to 0, likewise the count
       *  store the count_holder and the day to the EEPROM and MScard and likewise send the information
      */
      }
    
    }

 void activate_rtc(){
  // Get the day when the 3phases pin were high
  // create a variable to hold the string or character for the day whatever be the content of this variable will be checked against the value of the RTC each time.
  DateTime now = rtc.now();
  // We only need the day, month and year from the RTC for our problem. Hence i will leave behind the time section
  year_h = now.year();
  month_h = now.month();
  day_h = now.day();
  holder_h = year_h.concat(month_h.concat(day_h));
  
  // The variable holder will then be stored in the External EEPROM.
  // I will always compare the content of holder_i  to what the RTC gives at a particular instance that is holder_h.
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
   
   */
   
