/*
  MQTT.h - Library for GSM MQTT Client.
  Created by Nithin K. Kurian, Dhanish Vijayan, Elementz Engineers Guild Pvt. Ltd, July 2, 2016.
  Released into the public domain.
*/
/*
 *   add Reset function
 *   print tcpstatus correctly
 *   Get APN from nain program
 *   Replace all Serial by GSM_SERIAL
 *   Set HW serial via macro GSM_SERIAL
 *   Replace all serialEvent by SERIALEVENT
 *   Set hw serial event via macro SERIALEVENT
 *   
 *   change MQTT_PORT to int
 *   Add USERID, PASSWORD
 *   Replace printMessageType and printConnectAck by printConstString
 *   Bugfix replace 200 by UART_BUFFER_LENGTH in SERIALEVENT
 */
#include "GSM_MQTT.h"
#include "Arduino.h"
#include <SoftwareSerial.h>
#include <avr/pgmspace.h>
//extern uint8_t GSM_Response;

extern SoftwareSerial DB_SERIAL;
extern String MQTT_HOST;
extern int MQTT_PORT;
extern String APN;
extern String USERID;
extern String PASSWORD;

extern GSM_MQTT MQTT;
int GSM_Response = 0;
unsigned long previousMillis = 0;
//char inputString[UART_BUFFER_LENGTH];         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
void SERIALEVENT();
GSM_MQTT::GSM_MQTT(unsigned long KeepAlive)
{
  _KeepAliveTimeOut = KeepAlive;
}

void GSM_MQTT::begin(void)
{
  bool init = false;
  DB_SERIAL.begin(9600);
  GSM_SERIAL.begin(9600);
//  GSM_SERIAL.write("AT\r\n");
  while (!init)
  {
    if (sendATreply("AT\r","OK",1000) == 1)
      init = true;
    else
      GSM_MQTT::Reset();
  }
  delay(1000);
  _tcpInit();
}
char GSM_MQTT::_sendAT(char *command, unsigned long waitms)
{
  unsigned long PrevMillis = millis();
  strcpy(reply, "none");
  GSM_Response = 0;
  GSM_SERIAL.write(command);
  unsigned long currentMillis = millis();
  //  DB_SERIAL.println(PrevMillis);
  //  DB_SERIAL.println(currentMillis);
  while ( (GSM_Response == 0) && ((currentMillis - PrevMillis) < waitms) )
  {
    //    delay(1);
    SERIALEVENT();
    currentMillis = millis();
  }
  return GSM_Response;
}
char GSM_MQTT::sendATreply(char *command, char *replystr, unsigned long waitms)
{
  strcpy(reply, replystr);
  unsigned long PrevMillis = millis();
  GSM_ReplyFlag = 0;
  GSM_SERIAL.write(command);
  unsigned long currentMillis = millis();

  //  DB_SERIAL.println(PrevMillis);
  //  DB_SERIAL.println(currentMillis);
  while ( (GSM_ReplyFlag == 0) && ((currentMillis - PrevMillis) < waitms) )
  {
    //    delay(1);
    SERIALEVENT();
    currentMillis = millis();
  }
  return GSM_ReplyFlag;
}
void GSM_MQTT::_tcpInit(void)
{
  DB_SERIAL.print("MS ");
  DB_SERIAL.println(modemStatus);
  switch (modemStatus)
  {
    case 0:
      {
        delay(1000);
        GSM_SERIAL.print("+++");
        delay(500);
        if (_sendAT("AT\r\n", 5000) == 1)
        {
          modemStatus = 1;
        }
        else
        {
          modemStatus = 0;
          break;
        }
      }
    case 1:
      {
        if (_sendAT("ATE1\r\n", 2000) == 1)
        {
          modemStatus = 2;
        }
        else
        {
          modemStatus = 1;
          break;
        }
      }
    case 2:
      {
        if (sendATreply("AT+CREG?\r\n", "0,1", 5000) == 1)
        {
          _sendAT("AT+CIPMUX=0\r\n", 2000);
          _sendAT("AT+CIPMODE=1\r\n", 2000);
          if (sendATreply("AT+CGATT?\r\n", ": 1", 4000) != 1)
          {
            _sendAT("AT+CGATT=1\r\n", 2000);
          }
          modemStatus = 3;
          _tcpStatus = 2;
        }
        else
        {
          modemStatus = 2;
          break;
        }
      }
    case 3:
      {
        if (GSM_ReplyFlag != 7)
        {
          _tcpStatus = sendATreply("AT+CIPSTATUS\r\n", "STATE", 4000);
          if (_tcpStatusPrev == _tcpStatus)
          {
            tcpATerrorcount++;
            if (tcpATerrorcount >= 10)
            {
              tcpATerrorcount = 0;
              _tcpStatus = 7;
            }

          }
          else
          {
            _tcpStatusPrev = _tcpStatus;
            tcpATerrorcount = 0;
          }
        }
        _tcpStatusPrev = _tcpStatus;
        DB_SERIAL.print("TSC_S ");
        DB_SERIAL.println(_tcpStatus);
        switch (_tcpStatus)
        {
          case 2:
            {
//              _sendAT("AT+CSTT=\"AIRTELGPRS.COM\"\r\n", 5000);
                GSM_SERIAL.print("AT+CSTT=\"");
                GSM_SERIAL.print(APN);
                GSM_SERIAL.print("\",\"");
                GSM_SERIAL.print(USERID);
                GSM_SERIAL.print("\",\"");
                GSM_SERIAL.print(PASSWORD);
                _sendAT("\"\r\n",5000);
              break;
            }
          case 3:
            {
              _sendAT("AT+CIICR\r\n", 5000)  ;
              break;
            }
          case 4:
            {
              sendATreply("AT+CIFSR\r\n", ".", 4000) ;
              break;
            }
          case 5:
            {
              GSM_SERIAL.print("AT+CIPSTART=\"TCP\",\"");
              GSM_SERIAL.print(MQTT_HOST);
              GSM_SERIAL.print("\",");
              GSM_SERIAL.print(MQTT_PORT);
              if (_sendAT("\r\n", 5000) == 1)
              {
                unsigned long PrevMillis = millis();
                unsigned long currentMillis = millis();
                while ( (GSM_Response != 4) && ((currentMillis - PrevMillis) < 20000) )
                {
                  //    delay(1);
                  SERIALEVENT();
                  currentMillis = millis();
                }
              }
              break;
            }
          case 6:
            {
              unsigned long PrevMillis = millis();
              unsigned long currentMillis = millis();
              while ( (GSM_Response != 4) && ((currentMillis - PrevMillis) < 20000) )
              {
                //    delay(1);
                SERIALEVENT();
                currentMillis = millis();
              }
              break;
            }
          case 7:
            {
              sendATreply("AT+CIPSHUT\r\n", "OK", 4000) ;
              modemStatus = 0;
              _tcpStatus = 2;
              break;
            }
        }
      }
  }

}

void GSM_MQTT::_ping(void)
{
  if (pingFlag == true)
  {
    unsigned long currentMillis = millis();
    if ((currentMillis - _PingPrevMillis ) >= _KeepAliveTimeOut * 1000)
    {
      // save the last time you blinked the LED
      _PingPrevMillis = currentMillis;
      GSM_SERIAL.print(char(PINGREQ * 16));
      _sendLength(0);
    }
  }
}
void GSM_MQTT::_sendUTFString(char *s)
{
  int localLength = strlen(s);
#if 0
  DB_SERIAL.print("LL ");
  DB_SERIAL.println(localLength);
  DB_SERIAL.println(s);
#endif
  GSM_SERIAL.print(char(localLength / 256));
  GSM_SERIAL.print(char(localLength % 256));
  GSM_SERIAL.print(s);
}
void GSM_MQTT::_sendLength(int len)
{
  bool  length_flag = false;
#if 0
  DB_SERIAL.print("L ");
  DB_SERIAL.println(len);
#endif
  while (length_flag == false)
  {
    if ((len / 128) > 0)
    {
      GSM_SERIAL.print(char(len % 128 + 128));
      len /= 128;
    }
    else
    {
      length_flag = true;
      GSM_SERIAL.print(char(len));
    }
  }
}
void GSM_MQTT::connect(char *ClientIdentifier, char UserNameFlag, char PasswordFlag, char *UserName, char *Password, char CleanSession, char WillFlag, char WillQoS, char WillRetain, char *WillTopic, char *WillMessage)
{
  ConnectionAcknowledgement = NO_ACKNOWLEDGEMENT ;
  GSM_SERIAL.print(char(CONNECT * 16 ));
  char ProtocolName[7] = "MQIsdp";
  int localLength = (2 + strlen(ProtocolName)) + 1 + 3 + (2 + strlen(ClientIdentifier));
  if (WillFlag != 0)
  {
    localLength = localLength + 2 + strlen(WillTopic) + 2 + strlen(WillMessage);
  }
  if (UserNameFlag != 0)
  {
    localLength = localLength + 2 + strlen(UserName);

    if (PasswordFlag != 0)
    {
      localLength = localLength + 2 + strlen(Password);
    }
  }
  _sendLength(localLength);
  _sendUTFString(ProtocolName);
  GSM_SERIAL.print(char(_ProtocolVersion));
  GSM_SERIAL.print(char(UserNameFlag * User_Name_Flag_Mask + PasswordFlag * Password_Flag_Mask + WillRetain * Will_Retain_Mask + WillQoS * Will_QoS_Scale + WillFlag * Will_Flag_Mask + CleanSession * Clean_Session_Mask));
  GSM_SERIAL.print(char(_KeepAliveTimeOut / 256));
  GSM_SERIAL.print(char(_KeepAliveTimeOut % 256));
  _sendUTFString(ClientIdentifier);
  if (WillFlag != 0)
  {
    _sendUTFString(WillTopic);
    _sendUTFString(WillMessage);
  }
  if (UserNameFlag != 0)
  {
    _sendUTFString(UserName);
    if (PasswordFlag != 0)
    {
       _sendUTFString(Password);
    }
  }
}
void GSM_MQTT::publish(char DUP, char Qos, char RETAIN, unsigned int MessageID, char *Topic, char *Message)
{
  GSM_SERIAL.print(char(PUBLISH * 16 + DUP * DUP_Mask + Qos * QoS_Scale + RETAIN));
  int localLength = (2 + strlen(Topic));
  if (Qos > 0)
  {
    localLength += 2;
  }
  localLength += strlen(Message);
  _sendLength(localLength);
  _sendUTFString(Topic);
  if (Qos > 0)
  {
    GSM_SERIAL.print(char(MessageID / 256));
    GSM_SERIAL.print(char(MessageID % 256));
  }
  GSM_SERIAL.print(Message);
}
void GSM_MQTT::publishACK(unsigned int MessageID)
{
  GSM_SERIAL.print(char(PUBACK * 16));
  _sendLength(2);
  GSM_SERIAL.print(char(MessageID / 256));
  GSM_SERIAL.print(char(MessageID % 256));
}
void GSM_MQTT::publishREC(unsigned int MessageID)
{
  GSM_SERIAL.print(char(PUBREC * 16));
  _sendLength(2);
  GSM_SERIAL.print(char(MessageID / 256));
  GSM_SERIAL.print(char(MessageID % 256));
}
void GSM_MQTT::publishREL(char DUP, unsigned int MessageID)
{
  GSM_SERIAL.print(char(PUBREL * 16 + DUP * DUP_Mask + 1 * QoS_Scale));
  _sendLength(2);
  GSM_SERIAL.print(char(MessageID / 256));
  GSM_SERIAL.print(char(MessageID % 256));
}

void GSM_MQTT::publishCOMP(unsigned int MessageID)
{
  GSM_SERIAL.print(char(PUBCOMP * 16));
  _sendLength(2);
  GSM_SERIAL.print(char(MessageID / 256));
  GSM_SERIAL.print(char(MessageID % 256));
}
void GSM_MQTT::subscribe(char DUP, unsigned int MessageID, char *SubTopic, char SubQoS)
{
  GSM_SERIAL.print(char(SUBSCRIBE * 16 + DUP * DUP_Mask + 1 * QoS_Scale));
  int localLength = 2 + (2 + strlen(SubTopic)) + 1;
  _sendLength(localLength);
  GSM_SERIAL.print(char(MessageID / 256));
  GSM_SERIAL.print(char(MessageID % 256));
  _sendUTFString(SubTopic);
  GSM_SERIAL.print(SubQoS);
}
void GSM_MQTT::unsubscribe(char DUP, unsigned int MessageID, char *SubTopic)
{
  GSM_SERIAL.print(char(UNSUBSCRIBE * 16 + DUP * DUP_Mask + 1 * QoS_Scale));
  int localLength = (2 + strlen(SubTopic)) + 2;
  _sendLength(localLength);

  GSM_SERIAL.print(char(MessageID / 256));
  GSM_SERIAL.print(char(MessageID % 256));

  _sendUTFString(SubTopic);
}
void GSM_MQTT::disconnect(void)
{
  GSM_SERIAL.print(char(DISCONNECT * 16));
  _sendLength(0);
  pingFlag = false;
}


//Connect Ack
#define NUMBER_OF_CONNECT_ELEMENTS 6
const char ConnectAck0[] PROGMEM  = {"Connection Accepted\r\n"};
const char ConnectAck1[] PROGMEM  = {"Connection Refused: unacceptable protocol version\r\n"};
const char ConnectAck2[] PROGMEM  = {"Connection Refused: identifier rejected\r\n"};
const char ConnectAck3[] PROGMEM  = {"Connection Refused: server unavailable\r\n"};
const char ConnectAck4[] PROGMEM  = {"Connection Refused: bad user name or password\r\n"};
const char ConnectAck5[] PROGMEM  = {"Connection Refused: not authorized\r\n"};
const char * const ConnectMessages[NUMBER_OF_CONNECT_ELEMENTS] PROGMEM = {
  ConnectAck0,ConnectAck1,ConnectAck2,ConnectAck3,ConnectAck4,ConnectAck5
};

// General Messages
//Messages
#define NUMBER_OF_GENERAL_MESSAGES 15  // CONNECT is 1
const char BlankMessage[] PROGMEM  = {""};
const char CONNECTMessage[] PROGMEM  = {"Client request to connect to Server\r\n"};
const char CONNACKMessage[] PROGMEM  = {"Connection Acknowledgment\r\n"};
const char PUBLISHMessage[] PROGMEM  = {"Publish message\r\n"};
const char PUBACKMessage[] PROGMEM  = {"Publish Acknowledgment\r\n"};
const char PUBRECMessage[] PROGMEM  = {"Publish Received (assured delivery part 1)\r\n"};
const char PUBRELMessage[] PROGMEM  = {"Publish Release (assured delivery part 2)\r\n"};
const char PUBCOMPMessage[] PROGMEM  = {"Publish Complete (assured delivery part 3)\r\n"};
const char SUBSCRIBEMessage[] PROGMEM  = {"Client Subscribe request\r\n"};
const char SUBACKMessage[] PROGMEM  = {"Subscribe Acknowledgment\r\n"};
const char UNSUBSCRIBEMessage[] PROGMEM  = {"Client Unsubscribe request\r\n"};
const char UNSUBACKMessage[] PROGMEM  = {"Unsubscribe Acknowledgment\r\n"};
const char PINGREQMessage[] PROGMEM  = {"PING Request\r\n"};
const char PINGRESPMessage[] PROGMEM  = {"PING Response\r\n"};
const char DISCONNECTMessage[] PROGMEM  = {"Client is Disconnecting\r\n"};
const char * const GeneralMessages[NUMBER_OF_GENERAL_MESSAGES] PROGMEM = {
  BlankMessage,
  CONNECTMessage,CONNACKMessage,PUBLISHMessage,PUBACKMessage,PUBRECMessage,PUBRELMessage,
  PUBCOMPMessage,SUBSCRIBEMessage,SUBACKMessage,UNSUBSCRIBEMessage,UNSUBACKMessage,PINGREQMessage,
  PINGRESPMessage,DISCONNECTMessage
};

void GSM_MQTT::printConstString(const char * const list[],int index)
{
    char * ptr = (char *) pgm_read_word (&list [index]);
    char buffer [80]; // must be large enough!
//    DB_SERIAL.print("INDEX ");
//    DB_SERIAL.print(index);
//    DB_SERIAL.print(" STRLEN ");
//    DB_SERIAL.println(strlen_P(ptr));
    strcpy_P (buffer, ptr);
    DB_SERIAL.print(buffer);
}
unsigned int GSM_MQTT::_generateMessageID(void)
{
  if (_LastMessaseID < 65535)
  {
    return ++_LastMessaseID;
  }
  else
  {
    _LastMessaseID = 0;
    return _LastMessaseID;
  }
}
void GSM_MQTT::processing(void)
{
  if (TCP_Flag == false)
  {
    MQTT_Flag = false;
    _tcpInit();
  }
    _ping();
}
bool GSM_MQTT::available(void)
{
  return MQTT_Flag;
}
void SERIALEVENT()
{

  while (GSM_SERIAL.available())
  {
    char inChar = (char)GSM_SERIAL.read();
    DB_SERIAL.print(inChar);
    if (MQTT.TCP_Flag == false)
    {
      if (MQTT.index < UART_BUFFER_LENGTH)
      {
        MQTT.inputString[MQTT.index++] = inChar;
      }
      if (inChar == '\n')
      {
        MQTT.inputString[MQTT.index] = 0;
        stringComplete = true;
        if (strstr(MQTT.inputString, MQTT.reply) != NULL)
        {
          MQTT.GSM_ReplyFlag = 1;
          if (strstr(MQTT.inputString, " INITIAL") != 0)
          {
            MQTT.GSM_ReplyFlag = 2; //
          }
          else if (strstr(MQTT.inputString, " START") != 0)
          {
            MQTT.GSM_ReplyFlag = 3; //
          }
          else if (strstr(MQTT.inputString, "IP CONFIG") != 0)
          {
            _delay_us(10);
            MQTT.GSM_ReplyFlag = 4;
          }
          else if (strstr(MQTT.inputString, " GPRSACT") != 0)
          {
            MQTT.GSM_ReplyFlag = 4; //
          }
          else if ((strstr(MQTT.inputString, " STATUS") != 0) || (strstr(MQTT.inputString, "TCP CLOSED") != 0))
          {
            MQTT.GSM_ReplyFlag = 5; //
          }
          else if (strstr(MQTT.inputString, " TCP CONNECTING") != 0)
          {
            MQTT.GSM_ReplyFlag = 6; //
          }
          else if ((strstr(MQTT.inputString, " CONNECT OK") != 0) || (strstr(MQTT.inputString, "CONNECT FAIL") != NULL) || (strstr(MQTT.inputString, "PDP DEACT") != 0))
          {
            MQTT.GSM_ReplyFlag = 7;
          }
        }
        else if (strstr(MQTT.inputString, "OK") != NULL)
        {
          GSM_Response = 1;
        }
        else if (strstr(MQTT.inputString, "ERROR") != NULL)
        {
          GSM_Response = 2;
        }
        else if (strstr(MQTT.inputString, ".") != NULL)
        {
          GSM_Response = 3;
        }
        else if (strstr(MQTT.inputString, "CONNECT FAIL") != NULL)
        {
          GSM_Response = 5;
        }
        else if (strstr(MQTT.inputString, "CONNECT") != NULL)
        {
          GSM_Response = 4;
          MQTT.TCP_Flag = true;
          DB_SERIAL.println("MQTT.TCP_Flag True");
          MQTT.AutoConnect();
          MQTT.pingFlag = true;
          MQTT.tcpATerrorcount = 0;
        }
        else if (strstr(MQTT.inputString, "CLOSED") != NULL)
        {
          GSM_Response = 4;
          MQTT.TCP_Flag = false;
          MQTT.MQTT_Flag = false;
        }
        MQTT.index = 0;
        MQTT.inputString[0] = 0;
      }
    }
    else
    {
      uint8_t ReceivedMessageType = (inChar / 16) & 0x0F;
      uint8_t DUP = (inChar & DUP_Mask) / DUP_Mask;
      uint8_t QoS = (inChar & QoS_Mask) / QoS_Scale;
      uint8_t RETAIN = (inChar & RETAIN_Mask);
      if ((ReceivedMessageType >= CONNECT) && (ReceivedMessageType <= DISCONNECT))
      {
        bool NextLengthByte = true;
        MQTT.length = 0;
        MQTT.lengthLocal = 0;
        uint32_t multiplier=1;
        delay(2);
        char Cchar = inChar;
        while ( (NextLengthByte == true) && (MQTT.TCP_Flag == true))
        {
          if (GSM_SERIAL.available())
          {
            inChar = (char)GSM_SERIAL.read();
//            DB_SERIAL.print((char)'.');
//            DB_SERIAL.print((unsigned char)inChar, HEX);
            if ((((Cchar & 0xFF) == 'C') && ((inChar & 0xFF) == 'L') && (MQTT.length == 0)) || (((Cchar & 0xFF) == '+') && ((inChar & 0xFF) == 'P') && (MQTT.length == 0)))
            {
              MQTT.index = 0;
              MQTT.inputString[MQTT.index++] = Cchar;
              MQTT.inputString[MQTT.index++] = inChar;
              MQTT.TCP_Flag = false;
              MQTT.MQTT_Flag = false;
              MQTT.pingFlag = false;
              DB_SERIAL.println("Disconnecting");
            }
            else
            {
              if ((inChar & 128) == 128)
              {
                MQTT.length += (inChar & 127) *  multiplier;
                multiplier *= 128;
                DB_SERIAL.println("More");
              }
              else
              {
                NextLengthByte = false;
                MQTT.length += (inChar & 127) *  multiplier;
                multiplier *= 128;
              }
            }
          }
        }
        MQTT.lengthLocal = MQTT.length;
//        DB_SERIAL.print('L');
//        DB_SERIAL.println(MQTT.length);
        if (MQTT.TCP_Flag == true)
        {
          DB_SERIAL.print("MSG type ");
          DB_SERIAL.println(ReceivedMessageType);
          MQTT.printConstString(GeneralMessages,ReceivedMessageType);
          MQTT.index = 0L;
          uint32_t a = 0;
          while ((MQTT.length-- > 0) && (GSM_SERIAL.available()))
          {
            MQTT.inputString[uint32_t(MQTT.index++)] = (char)GSM_SERIAL.read();
            delay(1);
          }
        //  DB_SERIAL.println(" ");
          if (ReceivedMessageType == CONNACK)
          {
            MQTT.ConnectionAcknowledgement = MQTT.inputString[0] * 256 + MQTT.inputString[1];
        //    MQTT.waitingForAck = false;
            if (MQTT.ConnectionAcknowledgement == 0)
            {
              MQTT.MQTT_Flag = true;
              MQTT.OnConnect();
            }
            MQTT.printConstString(ConnectMessages,MQTT.ConnectionAcknowledgement);
            // MQTT.OnConnect();
          }
          else if (ReceivedMessageType == PUBLISH)
          {
            uint32_t TopicLength = (MQTT.inputString[0]) * 256 + (MQTT.inputString[1]);
            DB_SERIAL.print("Topic : '");
            MQTT.PublishIndex = 0;
            for (uint32_t iter = 2; iter < TopicLength + 2; iter++)
            {
              DB_SERIAL.print(MQTT.inputString[iter]);
              MQTT.Topic[MQTT.PublishIndex++] = MQTT.inputString[iter];
            }
            MQTT.Topic[MQTT.PublishIndex] = 0;
            DB_SERIAL.print("' Message :'");
            MQTT.TopicLength = MQTT.PublishIndex;

            MQTT.PublishIndex = 0;
            uint32_t MessageSTART = TopicLength + 2UL;
            int MessageID = 0;
            if (QoS != 0)
            {
              MessageSTART += 2;
              MessageID = MQTT.inputString[TopicLength + 2UL] * 256 + MQTT.inputString[TopicLength + 3UL];
            }
            for (uint32_t iter = (MessageSTART); iter < (MQTT.lengthLocal); iter++)
            {
              DB_SERIAL.print(MQTT.inputString[iter]);
              MQTT.Message[MQTT.PublishIndex++] = MQTT.inputString[iter];
            }
            MQTT.Message[MQTT.PublishIndex] = 0;
            DB_SERIAL.println("'");
            MQTT.MessageLength = MQTT.PublishIndex;
            if (QoS == 1)
            {
              MQTT.publishACK(MessageID);
            }
            else if (QoS == 2)
            {
              MQTT.publishREC(MessageID);
            }
            MQTT.OnMessage(MQTT.Topic, MQTT.TopicLength, MQTT.Message, MQTT.MessageLength);
            MQTT.MessageFlag = true;
          }
          else if (ReceivedMessageType == PUBREC)
          {
            DB_SERIAL.print("Message ID :");
            MQTT.publishREL(0, MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;
            DB_SERIAL.println(MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;

          }
          else if (ReceivedMessageType == PUBREL)
          {
            DB_SERIAL.print("Message ID :");
            MQTT.publishCOMP(MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;
            DB_SERIAL.println(MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;

          }
          else if ((ReceivedMessageType == PUBACK) || (ReceivedMessageType == PUBCOMP) || (ReceivedMessageType == SUBACK) || (ReceivedMessageType == UNSUBACK))
          {
            DB_SERIAL.print("Message ID :");
            DB_SERIAL.println(MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;
          }
          else if (ReceivedMessageType == PINGREQ)
          {
            MQTT.TCP_Flag = false;
            MQTT.pingFlag = false;
            DB_SERIAL.println("Disconnecting");
            MQTT.sendATreply("AT+CIPSHUT\r\n", ".", 4000) ;
            MQTT.modemStatus = 0;
          }
        }
      }
      else if ((inChar = 13) || (inChar == 10))
      {
      }
      else
      {
        DB_SERIAL.print("Received :Unknown Message Type :");
        DB_SERIAL.println(inChar);
      }
    }
  }
}
