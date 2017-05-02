#include <Adafruit_RGBLCDShield.h>

#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

enum Display {
  CLOCK,
  CLOCKSET,
  ALARMSET,
  ALARM,
};

Display display = CLOCK;

void SetDisplayType(Display type){
  display = type;
}

class Time {
  private:
    long second;
    long minute;
    long hour;
    long starttime;
    long startmillis;

    bool aalive = false;
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

    void SetTime(int h, int m, int s){
      SetHour(h);
      SetMinute(m);
      SetSecond(s);
      SetOffset();
    }

    void SetOffset(){
      starttime = TimeToSeconds();
      SetMillisOffset();
    }

    void SetMillisOffset(){
      startmillis = millis() / 1000;
    }

    long GetHour(){
      return hour;
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
      }
      SetOffset();
    }

    long GetMinute(){
      return minute;
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
      }
      SetOffset();
    }

    long GetSecond(){
      return second;
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
      }
      SetOffset();
    }

    String GetTime(){
      return DigitDisplay(hour) + ":" + DigitDisplay(minute) + ":" + DigitDisplay(second);
    }

    void SetAlarm(long h, long m, long s){
      ahour = h;
      aminute = m;
      asecond = s;
      aalive = true;
    }

    void SetAlarm(bool active){
      aalive = active;
    }

    bool IsAlarm(){
      return aalive;
    }

    void CheckAlarm(){
      if (!aalive){ return; }
      if (second == asecond && minute == aminute && hour == ahour){
        Alarm();
      }
    }

    void Alarm(){
      SetDisplayType(ALARM);
      lcd.clear();
      aalive = false;
    }

    void AddTime(long s){
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
      }
    }
    
    void Update(){
      long t = ((millis() / 1000) - startmillis) + starttime; // Applies the offset of when we set the time
      second = (t % 60);
      minute = (t / 60) % 60;
      hour = (t / 60 / 60) % 24;

      CheckAlarm();
    }
};

Time time;
long timeOffset;

char buffer[24];
const int CLICK_DELAY = 350;
int LastClick = 0;

enum Character{
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
uint8_t AClock[8] = {31,17,10,4,10,17,31};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.begin(16,2);
  //lcd.print("Testing 123 again");

  lcd.createChar(LeftArrow, LArrow);
  lcd.createChar(RightArrow, RArrow);
  lcd.createChar(DownArrow, DArrow);
  lcd.createChar(UpArrow, UArrow);
  lcd.createChar(Alarm, AAlarm);
  lcd.createChar(Clock, AClock);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (display == CLOCK){
     time.Update(); // Update the Time based on system clock / offset
  }
  
  switch(display){
    case CLOCK: HandleClock(); break;
    case CLOCKSET: HandleClockSet(); break;
    case ALARMSET: HandleAlarmSet(); break;
    case ALARM: HandleAlarm(); break;
    default: HandleClock();
  }
  
  ButtonHandler();
}

void HandleClock(){
  lcd.setCursor(0,0);
  lcd.print("    " + time.GetTime() + " "); // string is used for padding to centre time

  if(time.IsAlarm()){
    lcd.setCursor(15,0);
    sprintf(buffer, "%c", Alarm);
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

SetType caretPos = Hour;

void HandleClockSet(){
  uint8_t buttons = lcd.readButtons();
  
  if(buttons){
    if(buttons & BUTTON_UP){
      switch(caretPos){
        case Hour: time.SetTime(time.GetHour() + 1, time.GetMinute(), time.GetSecond()); break;
        case Minute: time.SetTime(time.GetHour(), time.GetMinute() + 1, time.GetSecond());  break;
        case Second: time.SetTime(time.GetHour(), time.GetMinute(), time.GetSecond() + 1);  break;
      }
      time.SetMillisOffset();
    }
    if(buttons & BUTTON_DOWN){
      switch(caretPos){
        case Hour: time.SetTime(time.GetHour() - 1, time.GetMinute(), time.GetSecond()); break;
        case Minute: time.SetTime(time.GetHour(), time.GetMinute() - 1, time.GetSecond()); break;
        case Second: time.SetTime(time.GetHour(), time.GetMinute(), time.GetSecond() -1); break;
      }
      time.SetMillisOffset();
    }
    if(buttons & BUTTON_RIGHT){
      int tmp = (int)caretPos + 1; // todo: shitty workaround
      if(tmp > (int)Second){
        caretPos = Hour;
      }
      else{
        caretPos = (SetType)tmp;
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
  lcd.print(time.GetTime());
  
  lcd.setCursor(0,1);
  sprintf(buffer, "%cBCK %c+ %c- CHNG%c", LeftArrow, UpArrow, DownArrow, RightArrow);
  lcd.print(buffer); // Help Menu for Setting Clock
}


Time alarm(23,59,55);
void HandleAlarmSet(){
  uint8_t buttons = lcd.readButtons();
  
  if(buttons){
    if(buttons & BUTTON_UP){
      switch(caretPos){
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
      int tmp = (int)caretPos + 1; // todo: shitty workaround
      if(tmp > (int)Second){
        caretPos = Hour;
      }
      else{
        caretPos = (SetType)tmp;
      } 
    }
    if(buttons & BUTTON_SELECT){
      if(time.IsAlarm()){       
        time.SetAlarm(false);
        lcd.setBacklight(RED);
        delay(500);
      }
      else{
        time.SetAlarm(alarm.GetHour(), alarm.GetMinute(), alarm.GetSecond());
        lcd.setBacklight(GREEN);
        delay(500);        
      }
      lcd.setBacklight(WHITE);
    }
    if(buttons & BUTTON_LEFT){
      SetDisplayType(CLOCK);
    }
  }
    
  lcd.setCursor(9, 0);
  switch(caretPos){
    case Hour: lcd.print("Hour"); break; // Prevents us from having to clear the screen if we make them all the same amount of characters
    case Minute: lcd.print("Mins"); break;
    case Second: lcd.print("Secs"); break;
  }
  sprintf(buffer, "%c", Alarm);
  lcd.setCursor(15,0);
  lcd.print(buffer);
  
  lcd.setCursor(0,0);
  lcd.print(alarm.GetTime());
  
  lcd.setCursor(0,1);
  sprintf(buffer, "%cBCK %c+ %c- CHNG%c", LeftArrow, UpArrow, DownArrow, RightArrow);
  lcd.print(buffer); // Help Menu for Setting Clock
}

void HandleAlarm(){
  lcd.setCursor(3,0);
  sprintf(buffer, "%c ALARM! %c", Alarm, Alarm);
  lcd.print(buffer);

  lcd.setBacklight(YELLOW);

  lcd.setCursor(0,1);
  sprintf(buffer, "ANY      %cSnooze", DownArrow);
  lcd.print(buffer);
  
  uint8_t buttons = lcd.readButtons();

  if(buttons){
    if(buttons & BUTTON_DOWN){
      alarm.AddTime(30);
      time.SetAlarm(alarm.GetHour(), alarm.GetMinute(), alarm.GetSecond());
    }
    SetDisplayType(CLOCK);
    lcd.setBacklight(WHITE);
  }
}

void ButtonHandler(){
  uint8_t buttons = lcd.readButtons();
  
  if(buttons && millis() > LastClick + CLICK_DELAY){
    if(buttons & BUTTON_LEFT){
      if (display == CLOCK){
        SetDisplayType(CLOCKSET);
      }
      else{
        SetDisplayType(CLOCK);
      }
      lcd.clear(); // Clear the LCD for the change in draw
    }
    if(buttons & BUTTON_RIGHT){
      if (display == CLOCK){ // Only trigger this when we're at the "main menu"
        SetDisplayType(ALARMSET);
      }
      lcd.clear(); // Clear the LCD for the change in draw
    }
    LastClick = millis();
  }
}

