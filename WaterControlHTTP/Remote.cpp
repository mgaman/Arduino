#include <avr/pgmspace.h>
#include "Arduino.h"
#include "Configuration.h"
#include "Remote.h"
// GSM shield, use SMS for control
// NOTE the following includes MUST be in the main INO file else the include path not setup correctly
// added messaging via email as that is cheaper than SMS. 
#include "SIM900.h"
#include "sms.h"
//#include "smtp.h"
#include "inetGSM.h"
#include "wcEEPROM.h"
//#include "DS3234.h"

char * EEPROMGetIndex(enum eEEPROMIndex);

SMSGSM sms;
//SMTPGSM smtp;
InetGSM inet;

char LastIncomingPhone[20];
boolean started=false;
char smsbuffer[SMS_LENGTH];
char IncomingPhone[20];  // phone number
char SMStime[20];  // timestamp
extern bool SMSout;  // if true send SMS else print to Serial
#ifdef ANTENNA_BUG
extern bool ignoreMeter;     // set true while sending emails
#endif
char cooked[300];

void Connect(int cid)
{
  bool rc = false;
   //GPRS attach, put in order APN, username and password.
  if (inet.CloseGprs(HTTPCID)) // ignore if error
    Serial.println("status=DETACHED");
  else
    Serial.println("status=NOT DETACHED");
//  Serial.println(EEPROMGetIndex(APN));
  while (!inet.OpenGprs(cid,EEPROMGetIndex(APN),20))
//  while (!inet.OpenGprs(HTTPCID,"uinternet",20))
    Serial.println("status=NOT ATTACHED");
  Serial.println("status=ATTACHED");
}

bool RemoteInit(enum eRemoteType type)
{
  bool success = false;
  strcpy(LastIncomingPhone,DefaultHouse);
  // set up serial line and gsm if relevant
  REMOTE_SERIAL.begin(REMOTE_BAUD);
  switch (type)
  {
    case SERIAL_REMOTE:
      success = true;
      break;
    case GSM_REMOTE:
      success = gsm.begin(REMOTE_BAUD);
      if (success)
      {
        // open up socket & keep it forever
        Connect(HTTPCID); // loops until done
      }
      break;
  }
  return success;
}

// if a message as available, pass it to caller
// delete from SIM when read
bool RemoteMessageAvailable(char *msg,char *timestamp)
{
    int i = sms.IsSMSPresent(SMS_UNREAD);
    if (i > 0)
    {
      // modified GetSMS to extract timestamp
      sms.GetSMS(i,IncomingPhone,SMStime,smsbuffer,SMS_LENGTH);
#if 1
      Serial.print("Incoming from: ");
      Serial.println("--");
      Serial.println(IncomingPhone);
      Serial.println("--");
      Serial.println(SMStime);
      Serial.println("--");
      Serial.println(smsbuffer);
#endif
      strcpy(LastIncomingPhone,"0545919886"/*IncomingPhone*/);
      strcpy(timestamp,SMStime);
      strcpy(msg,smsbuffer);
      sms.DeleteSMS(i);
      return true;
    } 
    else
      return false; 
}

bool RemoteSetClock(char *s)
{
  return gsm.SetClock(s);
}

char *RemoteGetClock()
{
  char *cs,*ce;
  //return smtp.SmtpGetClock();
  //return "16/12/1 2:3:4+12";
  // string includes quotes and spaces so trim those
  cs = strchr(gsm.GetClock(),'\"');
  if (cs)
  {
    cs++;
    ce = strchr(cs,'+');
    *ce = 0;
  //  Serial.println(cs);
  //  Serial.println(urlencode(cs,cooked));
  //  Serial.println(cooked);
    urlencode(cs,cooked);
    return cooked;    
  }
  else
    return "0";
}

int urlencode(char *src, char *tgt)
{
  int offset = 0;
//  const char rfc3986[] = " !*'();:@&=+$,/?#[]";
  const char rfc3986[] = " !*'();:@+$,/?#[]";  // without & = 
  while (*src)
  {
    if (strchr(rfc3986,*src))
    {
      sprintf(&tgt[offset],"%%%02X",*src);
      offset += 3;
    }
    else
      tgt[offset++] = *src;
    src++;
  }
  tgt[offset] = 0;
  return offset;
}

int HTTPGet(char *server,char *url,char *buf, int buflen)
{
  char *c;
  int rc,CR;
#ifdef ANTENNA_BUG
//  ignoreMeter = true;     // set true while transmitting
#endif
  CR = inet.httpGET(server, 80, url, buf, buflen);
#ifdef ANTENNA_BUG
 // ignoreMeter = false; 
#endif
  Serial.print("Length of data received: ");
  Serial.println(CR);  
    // extract return code from Nth line HTTP/1.1 200 OK
  Serial.println("--------------------------------");
  if (CR > 0)
  {
    c = strchr(buf,'H');
    Serial.println(c);
    c = strchr(c,' ');
    sscanf(c,"%d",&rc);
    return rc;    
  }
  else
    return 0;
}

bool RemoteSendMessage(char * srv, char *url, int port) // to http server
{
  int retries = 3;
  char SRV[30];
  bool success = false;
  int rc;
  memcpy(SRV,srv,30);
  Serial.println(SRV);
  Serial.println(url);
  // check if connection still open, if not re-open
 // check IP address
  char *c = inet.GetGprsIP(HTTPCID);
  Serial.println(c);
  if (strcmp(c,"0.0.0.0") == 0)
  {
    // restart GPRS
    Serial.println("Restart GPRS");
    gsm.Echo(0);
    gsm.RxInit(15000, 2000); 
    Connect(HTTPCID);
    c = inet.GetGprsIP(HTTPCID);
    Serial.println(c);
  }
  retries = 3;
  success = false;
  while (retries-- > 0 && !success)
  {
    rc = HTTPGet(SRV,url,smsbuffer,sizeof(smsbuffer));
    Serial.println(rc);
    success = (rc == 200);
  }
  return success;
}

bool RemoteSendSMS(char *msg, char *pn)
{
  return sms.SendSMS(pn, msg);
}

bool RemoteSendSMS(char *msg)
{
  return sms.SendSMS(EEPROMGetIndex(DP),msg);
}

