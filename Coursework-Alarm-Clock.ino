#include <Adafruit_RGBLCDShield.h> // Header file for LCD Shield function

#define RED 0x1 // Colour Constants for the LED Backlight
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield(); // Initialise LCD Shield library

enum Display { // Declare all the alarms possible states as an Enum
  CLOCK,
  CLOCKSET,
  ALARMSET,
  ALARM, // Each one represents a different possible state,
};       // and allows us to easily determine what state the alarm is in at any one time


Display display = CLOCK; // Start the alarm in the CLOCK state

void SetDisplayType(Display type){ // create a Helper function to change the state of the alarm
  display = type;
  lcd.clear(); // Clear the screen because when we change state we are most likely going to be drawing something new
}

class Time { // Time Class is used help us handle the time format and update it, while keeping it all organised and easy to use
  private:
    long second; // Private variables are only available to functions inside this class
    long minute;
    long hour;
    long starttime; // starttime is used to offset the time at the start, allowing us to start at times other than 00:00:00
    long startmillis; // startmillis is used as an offset to time how long people spend in the menu setting the alarm then adjust the time accordingly
                      // since my clock uses millis() the time is always being updated
                      
    bool aalive = false; // Alarm clock variables
    long ahour;
    long asecond;
    long aminute;

    String DigitDisplay(int t){ // Adds trailing zeroes to numbers below 10, keeps formatting pretty standard
      if(t < 10){
        return "0" + String(t);
      }
      return String(t);
    }

    long TimeToSeconds(){
      return (hour * 60 * 60) + (minute * 60) + (second); // Converts our whole time to seconds
    }
    
  public:

    Time(int h=15,int m=10, int s=5){
      SetTime(h,m,s);
      SetOffset(); // When the class is intialised and the default time is set store it as our start time
    }

    void SetTime(int h, int m, int s){ // used to set our time after the class has been constructed
      SetHour(h);
      SetMinute(m);
      SetSecond(s);
      SetOffset();
    }

    void SetOffset(){
      starttime = TimeToSeconds(); // Resets the offset of the time
      SetMillisOffset();
    }

    void SetMillisOffset(){
      startmillis = millis() / 1000; // Sets the clock set offset in seconds
    }

    long GetHour(){
      return hour; // Returns the hour of the time as a long
    }

    void SetHour(long h){
      if (h > 23){
        hour = 23;
      }
      else if (h <= 0){
        hour = 0;
      }
      else{
        hour = h;
      } // used to clamp the hour variable to standard hours
      SetOffset(); // update the offsets with the new time
    }

    long GetMinute(){
      return minute; // Return the minutes as a long
    }

    void SetMinute(long m){
      if (m > 59){
        minute = 59;
      }
      else if(m <= 0){
        minute = 0;
      }
      else{
        minute = m; 
      } // Clamps minutes between 0-59
      SetOffset();// update the offsets since the time has been changed
    }

    long GetSecond(){
      return second; // return second as a long
    }

    void SetSecond(long s){
      if (s > 59){
        second = 59;
      }
      else if(s <= 0){
        second = 0;
      }
      else{
        second = s;
      } // same as above
      SetOffset();
    }

    String GetTime(){
      return DigitDisplay(hour) + ":" + DigitDisplay(minute) + ":" + DigitDisplay(second);
    } // Return a formatted time with trailing zeroes and :

    void SetAlarm(long h, long m, long s){
      ahour = h;
      aminute = m;
      asecond = s;
      aalive = true; // used to start the alarm at a given time
    }

    void SetAlarm(bool active){
      aalive = active; // Sets the alarm on or off
    }

    bool IsAlarm(){
      return aalive; // returns if the alarm is on as a bool (True = on / False = off)
    }

    void CheckAlarm(){
      if (!aalive){ return; } // check if the alarm is active
      if (second == asecond && minute == aminute && hour == ahour){ // check if all the seconds, minutes and hours line up
        Alarm(); // if so activate the alarm
      }
    }

    void Alarm(){
      SetDisplayType(ALARM); // set the display type
      lcd.clear(); // clear the screen
      aalive = false; // turn the alarm off
    }

    void AddTime(long s){ // used to add seconds to the current time
      if (GetSecond() + s < 60){
        SetSecond(GetSecond() + s);
      }
      else {
        int r = (GetSecond() + s) - 60;
        if(GetMinute() == 59){
          if(GetHour() == 23){
            SetHour(0);
          }
          else{
            SetHour(GetHour() + 1);
          }
          SetMinute(0);
        }
        else{
          SetMinute(GetMinute() + 1);  
        }
        SetSecond(r);
      } // each statement checks to ensure that the updating value will be inside their range, if not we update that value and carry the remainder to update
    }   // only supports an extra minute or so, because of `int r = (GetSecond() + s) - 60;` could possible be still above 60 and then setSecond will clamp that value back to 59
    
    void Update(){
      long t = ((millis() / 1000) - startmillis) + starttime; // Applies the offset of when we set the time, returns uptime in seconds with the offsets applied
      second = (t % 60); // work out the amount of seconds passed this minute
      minute = (t / 60) % 60; // works out the amount of minutes
      hour = (t / 60 / 60) % 24; // works out the amount of hours
      
      CheckAlarm(); // check the updated values against the alarm
    }
};

Time time; // Initial Time class used for time keeping

char buffer[24]; // buffer used to print the help menus, along the bottom

enum Character{ // Custom Characters
  LeftArrow = 1,
  RightArrow = 2,
  DownArrow = 3,
  UpArrow = 4,
  Alarm = 5,
  Clock = 6,
};

uint8_t LArrow[8] = {0,2,6,14,6,2,0};
uint8_t RArrow[8] = {0,8,12,14,12,8,0};
uint8_t DArrow[8] = {0,0,31,14,4,0,0};
uint8_t UArrow[8] = {0,0,4,14,31,0,0};

uint8_t AAlarm[8] = {4,14,14,14,31,0,4};
uint8_t AClock[8] = {31,17,10,4,10,17,31}; // Custom Characters

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // start the serial monitor
  lcd.begin(16,2); // start the lcd with a dimension of 16,2

  lcd.createChar(LeftArrow, LArrow);
  lcd.createChar(RightArrow, RArrow);
  lcd.createChar(DownArrow, DArrow);
  lcd.createChar(UpArrow, UArrow);
  lcd.createChar(Alarm, AAlarm);
  lcd.createChar(Clock, AClock); // register the custom characters to their corresponding enum with the predefined arrays
}

void loop() { // main loop, everything updates in here
  // put your main code here, to run repeatedly:
  if (display == CLOCK){ // if we're on the main display update the time
     time.Update(); // Update the Time based on system clock / offset
  }
  
  switch(display){ // depending on the display depends on what function we run, and in turn what is drawn on the screen
    case CLOCK: HandleClock(); break;
    case CLOCKSET: HandleClockSet(); break;
    case ALARMSET: HandleAlarmSet(); break;
    case ALARM: HandleAlarm(); break;
    default: HandleClock();
  }
  
  ButtonHandler(); // Run the global button handler, some functions will have their own handlers as well to handle custom functions
}

void HandleClock(){
  lcd.setCursor(0,0);
  lcd.print("    " + time.GetTime() + " "); // string is used for padding to centre time

  if(time.IsAlarm()){
    lcd.setCursor(15,0);
    sprintf(buffer, "%c", Alarm); // if the alarm is on draw an indicator in the top right
    lcd.print(buffer);
  }
  
  lcd.setCursor(0,1);
  sprintf(buffer, "%cCLOCK    ALARM%c", LeftArrow, RightArrow);
  lcd.print(buffer); // Help Menu for Clock
}

enum SetType{
  Hour,
  Minute,
  Second,  
};

SetType caretPos = Hour; // stores the current position to alter

void HandleClockSet(){ // Clock Set Function
  uint8_t buttons = lcd.readButtons();
  
  if(buttons){
    if(buttons & BUTTON_UP){
      switch(caretPos){ // get the current caret position and add 1 to that value
        case Hour: time.SetTime(time.GetHour() + 1, time.GetMinute(), time.GetSecond()); break;
        case Minute: time.SetTime(time.GetHour(), time.GetMinute() + 1, time.GetSecond());  break;
        case Second: time.SetTime(time.GetHour(), time.GetMinute(), time.GetSecond() + 1);  break;
      }
      time.SetMillisOffset();
    }
    if(buttons & BUTTON_DOWN){
      switch(caretPos){ // same as above but take away 1
        case Hour: time.SetTime(time.GetHour() - 1, time.GetMinute(), time.GetSecond()); break;
        case Minute: time.SetTime(time.GetHour(), time.GetMinute() - 1, time.GetSecond()); break;
        case Second: time.SetTime(time.GetHour(), time.GetMinute(), time.GetSecond() -1); break;
      }
      time.SetMillisOffset();
    }
    if(buttons & BUTTON_RIGHT){
      int tmp = (int)caretPos + 1; // cast enum to int + 1
      if(tmp > (int)Second){ // check if we're on the last enum
        caretPos = Hour; // change enum to start
      }
      else{
        caretPos = (SetType)tmp; // if not cast the increment enum back to SetType
      } 
    }
  }
  
  lcd.setCursor(9, 0);
  switch(caretPos){
    case Hour: lcd.print("Hour"); break; // Prevents us from having to clear the screen if we make them all the same amount of characters
    case Minute: lcd.print("Mins"); break;
    case Second: lcd.print("Secs"); break;
  }
  sprintf(buffer, "%c", Clock);
  lcd.setCursor(15,0);
  lcd.print(buffer);
    
  lcd.setCursor(0,0);
  lcd.print(time.GetTime()); // general drawing, time, icon and help menu
  
  lcd.setCursor(0,1);
  sprintf(buffer, "%cBCK %c+ %c- CHNG%c", LeftArrow, UpArrow, DownArrow, RightArrow);
  lcd.print(buffer); // Help Menu for Setting Clock
}


Time alarm(23,59,55); // default alarm time
void HandleAlarmSet(){ // Alarm Set Function
  uint8_t buttons = lcd.readButtons();
  
  if(buttons){
    if(buttons & BUTTON_UP){
      switch(caretPos){ // Same code used in the SetClock is used here, only difference here is we used another class to store our values
        case Hour: alarm.SetTime(alarm.GetHour() + 1, alarm.GetMinute(), alarm.GetSecond()); break;
        case Minute: alarm.SetTime(alarm.GetHour(), alarm.GetMinute() + 1, alarm.GetSecond());  break;
        case Second: alarm.SetTime(alarm.GetHour(), alarm.GetMinute(), alarm.GetSecond() + 1);  break;
      }
    }
    if(buttons & BUTTON_DOWN){
      switch(caretPos){
        case Hour: alarm.SetTime(alarm.GetHour() - 1, alarm.GetMinute(), alarm.GetSecond()); break;
        case Minute: alarm.SetTime(alarm.GetHour(), alarm.GetMinute() - 1, alarm.GetSecond()); break;
        case Second: alarm.SetTime(alarm.GetHour(), alarm.GetMinute(), alarm.GetSecond() -1); break;
      }
    }
    if(buttons & BUTTON_RIGHT){
      int tmp = (int)caretPos + 1;
      if(tmp > (int)Second){
        caretPos = Hour;
      }
      else{
        caretPos = (SetType)tmp;
      } 
    }
    if(buttons & BUTTON_SELECT){
      if(time.IsAlarm()){ // if the alarm has been set turn it off       
        time.SetAlarm(false);
        lcd.setBacklight(RED);
        delay(500);
      }
      else{ // otherwise turn it one and set the alarm on the main time class
        time.SetAlarm(alarm.GetHour(), alarm.GetMinute(), alarm.GetSecond());
        lcd.setBacklight(GREEN);
        delay(500); // delays are used to make the backlights appear longer     
      }
      lcd.setBacklight(WHITE);
    }
  }
    
  lcd.setCursor(9, 0);
  switch(caretPos){
    case Hour: lcd.print("H"); break; // Prevents us from having to clear the screen if we make them all the same amount of characters
    case Minute: lcd.print("M"); break;
    case Second: lcd.print("S"); break;
  }
  sprintf(buffer, "SEL%c", Alarm);
  lcd.setCursor(12,0);
  lcd.print(buffer);
  
  lcd.setCursor(0,0);
  lcd.print(alarm.GetTime()); // general drawing functions
  
  lcd.setCursor(0,1);
  sprintf(buffer, "%cBCK %c+ %c- CHNG%c", LeftArrow, UpArrow, DownArrow, RightArrow);
  lcd.print(buffer); // Help Menu for Setting Clock
}

void HandleAlarm(){ // Alarm Screen
  lcd.setCursor(3,0);
  sprintf(buffer, "%c ALARM! %c", Alarm, Alarm);
  lcd.print(buffer);

  lcd.setBacklight(YELLOW); // General Drawing functions

  lcd.setCursor(0,1);
  sprintf(buffer, "ANY      %cSnooze", DownArrow);
  lcd.print(buffer);
  
  uint8_t buttons = lcd.readButtons();

  if(buttons){
    if(buttons & BUTTON_DOWN){
      alarm.SetTime(time.GetHour(), time.GetMinute(), time.GetSecond()); // Update the time to the most recent before setting snooze
      alarm.AddTime(30);
      time.SetAlarm(alarm.GetHour(), alarm.GetMinute(), alarm.GetSecond()); // If snooze was activate add 30 seconds to the alarm clock and set the alarm on the time class with updated values
    }
    SetDisplayType(CLOCK); // If a button is pressed reset back to the clock state
    lcd.setBacklight(WHITE); // reset backlight
  }
}

void ButtonHandler(){ // Main Button Handler
  uint8_t buttons = lcd.readButtons();
  
  if(buttons){
    if(buttons & BUTTON_LEFT){
      if (display == CLOCK){ 
        SetDisplayType(CLOCKSET);
      } // if the display is clock and button is left go to clockset, else go back to clock
      else{
        SetDisplayType(CLOCK);
      }
      lcd.clear(); // Clear the LCD for the change in draw
    }
    if(buttons & BUTTON_RIGHT){
      if (display == CLOCK){ // Only trigger this when we're at the "main menu" / main clock
        SetDisplayType(ALARMSET);
      }
      lcd.clear(); // Clear the LCD for the change in draw
    }
  }
}

