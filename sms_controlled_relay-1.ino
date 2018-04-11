#include <gprs.h>
#include <softwareserial.h>
 
#define TIMEOUT    5000
int DEVICE1_PIN=5; // To Trigger Relay 1
int DEVICE2_PIN=6; // To Trigger Relay 2
char phone[]="XXXXXXXXXX"; // Sim number to get status via SMS
char boot[]="MOTOR CONTROLLER SYSTEM @ ROOM1, POWER ON SUCCESS";
char on1[]="MOTOR 1 POWER ON SUCCESS";
char on2[]="MOTOR 2 POWER ON SUCCESS";
char off1[]="MOTOR 1 POWER OFF SUCCESS"; 
char off2[]="MOTOR 2 POWER OFF SUCCESS"; 

GPRS gprs;
 
void setup() {
  powerUp(); // GSM module power up
  pinMode(DEVICE1_PIN, OUTPUT);
  pinMode(DEVICE2_PIN, OUTPUT);
  digitalWrite(DEVICE1_PIN, LOW);
  digitalWrite(DEVICE2_PIN, LOW);
 
  Serial.begin(9600);
  while(!Serial);
 
  //Serial.println("Starting SIM800 SMS Command Processor");
  gprs.preInit();
  delay(1000);
 
  while(0 != gprs.init()) {
      delay(1000);
      //Serial.print("init error\r\n");
  } 
 
  //Set SMS mode to ASCII
  if(0 != gprs.sendCmdAndWaitForResp("AT+CMGF=1\r\n", "OK", TIMEOUT)) {
    ERROR("ERROR:CNMI");
    return;
  }
   
  //Start listening to New SMS Message Indications
  if(0 != gprs.sendCmdAndWaitForResp("AT+CNMI=1,2,0,0,0\r\n", "OK", TIMEOUT)) {
    ERROR("ERROR:CNMI");
    return;
  }
 
  gprs.sendSMS(phone,boot);
  delay(500); 
  //Serial.println("Init success");
}
 
//Variable to hold last line of serial output from SIM800
char currentLine[500] = "";
int currentLineIndex = 0;
 
//Boolean to be set to true if message notificaion was found and next
//line of serial output is the actual SMS message content
bool nextLineIsMessage = false;
 
void loop() {
  //If there is serial output from SIM800
  if(gprs.serialSIM800.available()){
    char lastCharRead = gprs.serialSIM800.read();
    //Read each character from serial output until \r or \n is reached (which denotes end of line)
    if(lastCharRead == '\r' || lastCharRead == '\n'){
        String lastLine = String(currentLine);
         
        //If last line read +CMT, New SMS Message Indications was received.
        //Hence, next line is the message content.
        if(lastLine.startsWith("+CMT:")){
           
          //Serial.println(lastLine);
          nextLineIsMessage = true;
           
        } else if (lastLine.length() > 0) {
           
          if(nextLineIsMessage) {
            //Serial.println(lastLine);
             
            //Read message content and set status according to SMS content
            if(lastLine.indexOf("POWERON1") >= 0){
              digitalWrite(DEVICE1_PIN, HIGH);
              
              // Send reply on success
              gprs.sendSMS(phone,on1);
              delay(500); 
              //Serial.print(on1); 
              //delay(500);
                    
            }
            else if(lastLine.indexOf("POWERON2") >= 0){
              digitalWrite(DEVICE2_PIN, HIGH);
              
              // Send reply on success
              gprs.sendSMS(phone,on2);
              delay(500); 
              //Serial.print(on2); 
              //delay(500);
                    
            } 
            else if(lastLine.indexOf("POWEROFF1") >= 0) {
              digitalWrite(DEVICE1_PIN, LOW);              
              delay(500);
              
              // Send reply on success
              gprs.sendSMS(phone,off1);
              delay(500);  
              //Serial.print(off1); 
              //delay(500);
            }
            else if(lastLine.indexOf("POWEROFF2") >= 0) {
              digitalWrite(DEVICE2_PIN, LOW);              
              delay(500);
              
              // Send reply on success
              gprs.sendSMS(phone,off2);
              delay(500);  
              //Serial.print(off2); 
              //delay(500);
            }
             
            nextLineIsMessage = false;
            //Delete all messages to free up sim memory
	    gprs.serialSIM800.println("AT+CMGD=1,4");
	    delay(1000);
            gprs.serialSIM800.print("AT+CMGDA=\"");
            delay(500);
            gprs.serialSIM800.println("DEL ALL\"");
            delay(500);
            gprs.serialSIM800.write((char)26);
          }
           
        }
         
        //Clear char array for next line of read
        for( int i = 0; i < sizeof(currentLine);  ++i ) {
         currentLine[i] = (char)0;
        }
        currentLineIndex = 0;
    } else {
      currentLine[currentLineIndex++] = lastCharRead;
    }
  }
}

  // To Power on the GSM module (This is no need for some types of GSM modules)
  void powerUp()
  {
    pinMode(9, OUTPUT);
    digitalWrite(9,LOW);
    delay(1000);
    digitalWrite(9,HIGH);
    delay(2000);
    digitalWrite(9,LOW);
    delay(3000);
  }
