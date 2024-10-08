# OpenButton
+ **OpenButton** is a tiny but useful button manager for Arduino.  
+ **Multiple button** can be managed.  
+ **Double-click** and **long-press** can be detected.  
+ **Chattering** will be reduced.  

**Sample:**  
```
#include <Arduino.h>
#include <OpenButton.hpp>

// set button GPIOs
#define BTN_A_PIN (39)
#define BTN_B_PIN (25)
#define LED_PIN (21)

OpenButton btn; // create OpenButton object
int8_t btnAId; // button A ID
int8_t btnBId; // button B ID

void showButtonInfo(uint32_t _res){
  if(btn.onPress(btnAId)) Serial.print("+"); // print "+" when BTN_A is pressed
  if(btn.onHold(btnAId, 1000)) Serial.print("H"); // print "H" when BTN_A was hold over 1000ms before release
  if(btn.onDblClk(btnAId)) Serial.print("D"); // print "D" when BTN_A is double clicked
  if(btn.onRelease(btnAId)) Serial.print("-"); // print "-" when BTN_A is released
  Serial.print(_res); // press status: 0=none, 1=BTN_A, 2=BTN_B, 3=both
  digitalWrite(LED_PIN, btn.isOn(btnBId)); // turn LED on while BTN_A is on
}

void setup() {
  Serial.begin(115200);  //while (!Serial);
  pinMode(LED_PIN, OUTPUT); // set LED_PIN as output
  btn.Setup(2); // set up to handle 2 buttons
  btnAId = btn.AddButton(BTN_A_PIN); // add button A
  btnBId = btn.AddButton(BTN_B_PIN); // add button B
  Serial.println(btnAId); // print button A ID(=0)c  
}

void loop() {
  uint32_t _res = btn.Update(); // update button status
  showButtonInfo(_res); // show button status
  delay(20);
}
```

**Result:**  
```
000000000000000000000000000000000000000000000+111111-000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000+111111111-000000000000+111111111-000000000000000000222222222000000+111111333-222222000000222222222000000000000000000000000000000000000000+111111111-000000222222222000+111111333-222222000+111333333D-222000000000+111111111111-222222222+333111111111-222222222000+111111111-000000000000222222222222000+111111111-222222222222000000000000000000000+111111111-000+111111D-000000+111111111-000000000222222222222000000000000+11111111
```

![image](img/ss00.jpg)


