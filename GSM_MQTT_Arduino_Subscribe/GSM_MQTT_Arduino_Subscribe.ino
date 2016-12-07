#include <string.h>
#include "GSM_MQTT.h"
#include <SoftwareSerial.h>
#define LOCAL
#ifdef LOCAL
String MQTT_HOST = "david-henry.dyndns.tv";
#else
String MQTT_HOST = "test.mosquitto.org";
#endif
/*
   MQTT host address
*/
int MQTT_PORT = 1883;
String APN = "uinternet";
String USERID = "";
String PASSWORD = "";
/*
   MQTT port
*/
SoftwareSerial DB_SERIAL(2,3); // RX, TX
char *UserID = "abcdef";
/*
   Software Serial through which mqtt events log is printed at 9600 baud rate
*/
#define RESET_PIN 8
void GSM_MQTT::Reset()
{
  DB_SERIAL.println("RS");
  pinMode(RESET_PIN,OUTPUT);
  digitalWrite(RESET_PIN,HIGH);
  delay(1200);
  digitalWrite(RESET_PIN,LOW);
  delay(10000);
}
void GSM_MQTT::AutoConnect(void)
{
  /*
     Use this function if you want to use autoconnect(and auto reconnect) facility
     This function is called whenever TCP connection is established (or re-established).
     put your connect codes here.
  */
#ifdef LOCAL
  connect(UserID, 1, 1, "henryd", "mga1958", 1, 1, 0, 0, "Will", "Testament");
#else
  connect("a", 0, 0, "", "", 1, 0, 0, 0, "", "");
#endif
/*    void connect(char *ClientIdentifier, char UserNameFlag, char PasswordFlag, char *UserName, char *Password, char CleanSession, char WillFlag, char WillQoS, char WillRetain, char *WillTopic, char *WillMessage);
          ClientIdentifier  :Is a string that uniquely identifies the client to the server.
                            :It must be unique across all clients connecting to a single server.(So it will be better for you to change that).
                            :It's length must be greater than 0 and less than 24
                            :Example "qwerty"
          UserNameFlag      :Indicates whether UserName is present
                            :Possible values (0,1)
                            :Default value 0 (Disabled)
          PasswordFlag      :Valid only when  UserNameFlag is 1, otherwise its value is disregarded.
                            :Indicates whether Password is present
                            :Possible values (0,1)
                            :Default value 0 (Disabled)
          UserName          :Mandatory when UserNameFlag is 1, otherwise its value is disregarded.
                            :The UserName corresponding to the user who is connecting, which can be used for authentication.
          Password          :alid only when  UserNameFlag and PasswordFlag are 1 , otherwise its value is disregarded.
                            :The password corresponding to the user who is connecting, which can be used for authentication.
          CleanSession      :If not set (0), then the server must store the subscriptions of the client after it disconnects.
                            :If set (1), then the server must discard any previously maintained information about the client and treat the connection as "clean".
                            :Possible values (0,1)
                            :Default value 1
          WillFlag          :This flag determines whether a WillMessage published on behalf of the client when client is disconnected involuntarily.
                            :If the WillFlag is set, the WillQoS, WillRetain, WillTopic, WilMessage fields are valid.
                            :Possible values (0,1)
                            :Default value 0 (Disables will Message)
          WillQoS           :Valid only when  WillFlag is 1, otherwise its value is disregarded.
                            :Determines the QoS level of WillMessage
                            :Possible values (0,1,2)
                            :Default value 0 (QoS 0)
          WillRetain        :Valid only when  WillFlag is 1, otherwise its value is disregarded.
                            :Determines whether the server should retain the Will message.
                            :Possible values (0,1)
                            :Default value 0
          WillTopic         :Mandatory when  WillFlag is 1, otherwise its value is disregarded.
                            :The Will Message will published to this topic (WillTopic) in case of involuntary client disconnection.
          WillMessage       :Mandatory when  WillFlag is 1, otherwise its value is disregarded.
                            :This message (WillMessage) will published to WillTopic in case of involuntary client disconnection.
  */
  
}
void GSM_MQTT::OnConnect(void)
{
  /*
     This function is called when mqqt connection is established.
     put your subscription publish codes here.
  */
//  DB_SERIAL.println("OCN");
  subscribe(0,_generateMessageID(), "ST", 1);
  /*    void subscribe(char DUP, unsigned int MessageID, char *SubTopic, char SubQoS);
          DUP       :This flag is set when the client or server attempts to re-deliver a SUBSCRIBE message
                    :This applies to messages where the value of QoS is greater than zero (0)
                    :Possible values (0,1)
                    :Default value 0
          Message ID:The Message Identifier (Message ID) field
                    :Used only in messages where the QoS levels greater than 0 (SUBSCRIBE message is at QoS =1)
          SubTopic  :Topic names to which  subscription is needed
          SubQoS    :QoS level at which the client wants to receive messages
                    :Possible values (0,1,2)
                    :Default value 0
  */

  publish(0, 1, 0, _generateMessageID(), "ST", "HI");
  /*  void publish(char DUP, char Qos, char RETAIN, unsigned int MessageID, char *Topic, char *Message);
      DUP       :This flag is set when the client or server attempts to re-deliver a PUBLISH message
                :This applies to messages where the value of QoS is greater than zero (0)
                :Possible values (0,1)
                :Default value 0
      QoS       :Quality of Service
                :This flag indicates the level of assurance for delivery of a PUBLISH message
                :Possible values (0,1,2)
                :Default value 0
      RETAIN    :if the Retain flag is set (1), the server should hold on to the message after it has been delivered to the current subscribers.
                :When a new subscription is established on a topic, the last retained message on that topic is sent to the subscriber
                :Possible values (0,1)
                :Default value 0
      Message ID:The Message Identifier (Message ID) field
                :Used only in messages where the QoS levels greater than 0
      Topic     :Publishing topic
      Message   :Publishing Message
  */
}
void GSM_MQTT::OnMessage(char *Topic, int TopicLength, char *Message, int MessageLength)
{
  /*
    This function is called whenever a message received from subscribed topics
    put your subscription publish codes here.
  */

  /*
     Topic        :Name of the topic from which message is coming
     TopicLength  :Number of characters in topic name
     Message      :The containing array
     MessageLength:Number of characters in message
  */
  DB_SERIAL.print("<< ");
  DB_SERIAL.print(TopicLength);
  DB_SERIAL.print(Topic);
  DB_SERIAL.print(MessageLength);
  DB_SERIAL.println(Message);

}
GSM_MQTT MQTT(30);
/*
   20 is the keepalive duration in seconds
*/

void setup()
{
  // initialize mqtt:
  // GSM modem should be connected to Hardware Serial
  //  index =0;
  MQTT.begin();
  /*
     You can write your code here
  */

}
void loop()
{

  /*
     You can write your code here
  */
  if (MQTT.available())
  {
    /*
      if you want to do something when mqtt connection is live.
      You can write your code here
    */
  }
  MQTT.processing();
}

