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

#include <LiquidCrystal.h>
#include <Keypad.h>
#include <EEPROM.h>


// setting the parameters for the keypad
const int Size = 6;
char special_keys[Size] = {'A','B','C','D','*','#'};
char meter_no[11];
byte value;
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

void setup() {
  // set up the LCD's number of columns and rows:
  //initial_eeprom_clearing();
  lcd.begin(20, 4);
  // Print a welcome message to the screen, displaying the company name and the name of the project
  lcd.print("MOLCOM MULTICONCEPTS");
  lcd.setCursor(2,1);
  lcd.print("POWER CONSUMPTION");
  lcd.setCursor(6,2);
  lcd.print("DURATION");
  delay(3000);
  lcd.clear();
  check_meter_no_eeprom();
  
  
}

void loop() {
 
 
}

void check_meter_no_eeprom(){
    // Check for meter_no in EEPROM. If meter_no exist display it to the screen, else print no meter_no, call set_meter_no()
    for(int address = 0; address < 11; address++){
     value = EEPROM.read(address);
     holder[address] = value;
    }
    if(holder[0]!=0){
      lcd.setCursor(0,0);
      lcd.print("Meter_no:");
      lcd.setCursor(9,0);
      lcd.print(holder);
      lcd.setCursor(0,1);
      lcd.print("Bearing Location");
      lcd.setCursor(0,2);
      lcd.print("Longitude:");
      lcd.setCursor(0,3);
      lcd.print("Latitude:");
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
   
