#include "inetGSM.h"
#define _GSM_CONNECTION_TOUT_ 5
#define _TCP_CONNECTION_TOUT_ 20
#define _GSM_DATA_TOUT_ 10
bool writewithtimeout(char *buf,const char *response,unsigned long start,unsigned long interchar);
static const char *OK = "OK";

int InetGSM::httpGET(const char* server, int port, const char* path, char* result, int resultlength)
{
  boolean connected=false;
  int n_of_at=0;
  int length_write;
  char end_c[2];
  end_c[0]=0x1a;
  end_c[1]='\0';
	int res;
  /*
  Status = ATTACHED.
  if(gsm.getStatus()!=GSM::ATTACHED)
    return 0;
  */
  while(n_of_at<3){
	  if(!connectTCP(server, port)){
	  	#ifdef DEBUG_ON
			DEBUG_SERIAL.println("DB:NOT CONN");
		#endif	
	    	n_of_at++;
	  }
	  else{
		connected=true;
		n_of_at=3;
	}
  }

  if(!connected) return 0;
	
  gsm.SimpleWrite("GET ");
  gsm.SimpleWrite(path);
  gsm.SimpleWrite(" HTTP/1.0\nHost: ");
  gsm.SimpleWrite(server);
  gsm.SimpleWrite("\n");
  gsm.SimpleWrite("User-Agent: Arduino");
  gsm.SimpleWrite("\n\n");
  gsm.SimpleWrite(end_c);

  switch(gsm.WaitResp(10000, 10, "SEND OK")){
	case RX_TMOUT_ERR: 
		return 0;
	break;
	case RX_FINISHED_STR_NOT_RECV: 
		return 0; 
	break;
  }
  
  
 //delay(2000);
  	#ifdef DEBUG_ON
		DEBUG_SERIAL.println("DB:SENT");
	#endif	
#if 0
  int res= gsm.read(result, resultlength);
#else
	byte status;
	bool success = false;
	// 1 sec. for initial comm tmout (some PHP mailer takes 5 secs)
    // and max. 150 msec. for inter character timeout
	gsm.RxInit(15000, 2000); 
	// wait response is finished
	do
	{
		if (gsm.IsStringReceived("OK"))
		{ 
			// perfect - we have some response, but what:
			status = RX_FINISHED;
			success = true;
			break; // so finish receiving immediately and let's go to 
             // to check response 
		}
		status = gsm.IsRxFinished();
	} while (status == RX_NOT_FINISHED);
	if (success)
	{
		memcpy(result,gsm.comm_buf,resultlength);
		res = 30; // covers 1st line
	}
	else
		res = 0;
#endif
  //gsm.disconnectTCP();
  
  //int res=1;
  return res;
}

int InetGSM::httpPOST(const char* server, int port, const char* path, const char* parameters, char* result, int resultlength)
{
  boolean connected=false;
  int n_of_at=0;
  char itoaBuffer[8];
  int num_char;
  char end_c[2];
  end_c[0]=0x1a;
  end_c[1]='\0';

  while(n_of_at<3){
	  if(!connectTCP(server, port)){
	  	#ifdef DEBUG_ON
			DEBUG_SERIAL.println("DB:NOT CONN");
		#endif	
	    	n_of_at++;
	  }
	  else{
		connected=true;
		n_of_at=3;
	}
  }

  if(!connected) return 0;
	
  gsm.SimpleWrite("POST ");
  gsm.SimpleWrite(path);
  gsm.SimpleWrite(" HTTP/1.1\nHost: ");
  gsm.SimpleWrite(server);
  gsm.SimpleWrite("\n");
  gsm.SimpleWrite("User-Agent: Arduino\n");
  gsm.SimpleWrite("Content-Type: application/x-www-form-urlencoded\n");
  gsm.SimpleWrite("Content-Length: ");
  itoa(strlen(parameters),itoaBuffer,10);
  gsm.SimpleWrite(itoaBuffer);
  gsm.SimpleWrite("\n\n");
  gsm.SimpleWrite(parameters);
  gsm.SimpleWrite("\n\n");
  gsm.SimpleWrite(end_c);
 
  switch(gsm.WaitResp(10000, 10, "SEND OK")){
	case RX_TMOUT_ERR: 
		return 0;
	break;
	case RX_FINISHED_STR_NOT_RECV: 
		return 0; 
	break;
  }

 delay(50);
	#ifdef DEBUG_ON
		DEBUG_SERIAL.println("DB:SENT");
	#endif	

  int res= gsm.read(result, resultlength);
  //gsm.disconnectTCP();
  return res;
}

int InetGSM::openmail(char* server, char* loginbase64, char* passbase64, char* from, char* to, char* subj)
{
  boolean connected=false;
  int n_of_at=0;
  char end_c[2];
  end_c[0]=0x1a;
  end_c[1]='\0';
	
  while(n_of_at<3){
	  if(!connectTCP(server, 25)){
	  	#ifdef DEBUG_ON
			DEBUG_SERIAL.println("DB:NOT CONN");
		#endif	
	    	n_of_at++;
	  }
	  else{
		connected=true;
		n_of_at=3;
	}
  }

  if(!connected) return 0;  
  
	delay(100);   
    gsm.SimpleWrite("HELO ");  gsm.SimpleWrite(server);  gsm.SimpleWrite("\n");
	gsm.SimpleWrite(end_c);
	gsm.WaitResp(5000, 100, "OK");
	if(!gsm.IsStringReceived("SEND OK"))
		return 0;
    delay(500);
	gsm.WaitResp(5000, 100);
	
	delay(100);   
	gsm.SimpleWriteln("AT+CIPSEND");
	switch(gsm.WaitResp(5000, 200, ">")){
		case RX_TMOUT_ERR: 
			return 0;
			break;
		case RX_FINISHED_STR_NOT_RECV: 
			return 0; 
			break;
	}
    gsm.SimpleWrite("AUTH LOGIN\n");
	gsm.SimpleWrite(end_c);
	gsm.WaitResp(5000, 100, "OK");
	if(!gsm.IsStringReceived("OK"))
		return 0;
    delay(500);
	gsm.WaitResp(5000, 100);
		
	delay(100);   
	gsm.SimpleWriteln("AT+CIPSEND");
	switch(gsm.WaitResp(5000, 200, ">")){
		case RX_TMOUT_ERR: 
			return 0;
			break;
		case RX_FINISHED_STR_NOT_RECV: 
			return 0; 
			break;
	}
    gsm.SimpleWrite(loginbase64);gsm.SimpleWrite("\n");
	gsm.SimpleWrite(end_c);
	gsm.WaitResp(5000, 100, "OK");
	if(!gsm.IsStringReceived("OK"))
		return 0;
    delay(500);
	gsm.WaitResp(5000, 100);

	delay(100);   
	gsm.SimpleWriteln("AT+CIPSEND");
	switch(gsm.WaitResp(5000, 200, ">")){
		case RX_TMOUT_ERR: 
			return 0;
			break;
		case RX_FINISHED_STR_NOT_RECV: 
			return 0; 
			break;
	}	
    gsm.SimpleWrite(passbase64);gsm.SimpleWrite("\n");
	gsm.SimpleWrite(end_c);
	gsm.WaitResp(5000, 100, "OK");
	if(!gsm.IsStringReceived("OK"))
		return 0;
    delay(500);
	gsm.WaitResp(5000, 100);
	

	delay(100);   
	gsm.SimpleWriteln("AT+CIPSEND");
	switch(gsm.WaitResp(5000, 200, ">")){
		case RX_TMOUT_ERR: 
			return 0;
			break;
		case RX_FINISHED_STR_NOT_RECV: 
			return 0; 
			break;
	}
    gsm.SimpleWrite("MAIL From: <");gsm.SimpleWrite(from);gsm.SimpleWrite(">\n");
	gsm.SimpleWrite(end_c);
	gsm.WaitResp(5000, 100, "OK");
	if(!gsm.IsStringReceived("OK"))
		return 0;
    delay(500);
	gsm.WaitResp(5000, 100, "");
	
	delay(100);   
	gsm.SimpleWriteln("AT+CIPSEND");
	switch(gsm.WaitResp(5000, 200, ">")){
		case RX_TMOUT_ERR: 
			return 0;
			break;
		case RX_FINISHED_STR_NOT_RECV: 
			return 0; 
			break;
	}
    gsm.SimpleWrite("RCPT TO: <");gsm.SimpleWrite(to);gsm.SimpleWrite(">\n");
	gsm.SimpleWrite(end_c);
	gsm.WaitResp(5000, 100, "OK");
	if(!gsm.IsStringReceived("OK"))
		return 0;
    delay(500);
	gsm.WaitResp(5000, 100, "");

		delay(100);   
	gsm.SimpleWriteln("AT+CIPSEND");
	switch(gsm.WaitResp(5000, 200, ">")){
		case RX_TMOUT_ERR: 
			return 0;
			break;
		case RX_FINISHED_STR_NOT_RECV: 
			return 0; 
			break;
	}
    gsm.SimpleWrite("Data\n");
	gsm.SimpleWrite(end_c);
	gsm.WaitResp(5000, 100, "OK");
	if(!gsm.IsStringReceived("OK"))
		return 0;
    delay(500);
	gsm.WaitResp(5000, 100, "");

	delay(100);   
	gsm.SimpleWriteln("AT+CIPSEND");
	switch(gsm.WaitResp(5000, 200, ">")){
		case RX_TMOUT_ERR: 
			return 0;
			break;
		case RX_FINISHED_STR_NOT_RECV: 
			return 0; 
			break;
	}
    gsm.SimpleWrite("Subject: ");gsm.SimpleWrite(subj);gsm.SimpleWrite("\n\n");
	
    return 1;
}
int InetGSM::closemail()
{
	char end_c[2];
	end_c[0]=0x1a;
	end_c[1]='\0';
  
	gsm.SimpleWrite("\n.\n");
	gsm.SimpleWrite(end_c);
	disconnectTCP();
	return 1;
}
 

int InetGSM::attachGPRS(char* domain, char* dom1, char* dom2)
{
   int i=0;
   delay(5000);
   
  //gsm._tf.setTimeout(_GSM_DATA_TOUT_);	//Timeout for expecting modem responses.
  gsm.WaitResp(50, 50);
  gsm.SimpleWriteln("AT+CIFSR");
  if(gsm.WaitResp(5000, 50, "ERROR")!=RX_FINISHED_STR_RECV){
  	#ifdef DEBUG_ON
		DEBUG_SERIAL.println("DB:ALREADY HAVE AN IP");
	#endif
	  gsm.SimpleWriteln("AT+CIPCLOSE");
	gsm.WaitResp(5000, 50, "ERROR");
	delay(2000);
	gsm.SimpleWriteln("AT+CIPSERVER=0");
	gsm.WaitResp(5000, 50, "ERROR");
	return 1;
  }
  else{

	#ifdef DEBUG_ON
		DEBUG_SERIAL.println("DB:STARTING NEW CONNECTION");
	#endif
  
  gsm.SimpleWriteln("AT+CIPSHUT");
  
  switch(gsm.WaitResp(500, 50, "SHUT OK")){

	case RX_TMOUT_ERR: 
		return 0;
	break;
	case RX_FINISHED_STR_NOT_RECV: 
		return 0; 
	break;
  }
	#ifdef DEBUG_ON
		DEBUG_SERIAL.println("DB:SHUTTED OK");
	#endif
	 delay(1000);
	 
  gsm.SimpleWrite("AT+CSTT=\"");
  gsm.SimpleWrite(domain);
  gsm.SimpleWrite("\",\"");
  gsm.SimpleWrite(dom1);
  gsm.SimpleWrite("\",\"");
  gsm.SimpleWrite(dom2);
  gsm.SimpleWrite("\"\r");  

  
  switch(gsm.WaitResp(500, 50, "OK")){

	case RX_TMOUT_ERR: 
		return 0;
	break;
	case RX_FINISHED_STR_NOT_RECV: 
		return 0; 
	break;
  }
	#ifdef DEBUG_ON
		DEBUG_SERIAL.println("DB:APN OK");
	#endif
	 delay(5000);
	  
	gsm.SimpleWriteln("AT+CIICR");  

  switch(gsm.WaitResp(10000, 50, "OK")){
	case RX_TMOUT_ERR: 
		return 0; 
	break;
	case RX_FINISHED_STR_NOT_RECV: 
		return 0; 
	break;
  }
  	#ifdef DEBUG_ON
		DEBUG_SERIAL.println("DB:CONNECTION OK");
	#endif

  delay(1000);


 gsm.SimpleWriteln("AT+CIFSR");
 if(gsm.WaitResp(5000, 50, "ERROR")!=RX_FINISHED_STR_RECV){
	#ifdef DEBUG_ON
		DEBUG_SERIAL.println("DB:ASSIGNED AN IP");
	#endif
	gsm.setStatus(gsm.ATTACHED);
	return 1;
}
	#ifdef DEBUG_ON
		DEBUG_SERIAL.println("DB:NO IP AFTER CONNECTION");
	#endif
 return 0;
 }
}

int InetGSM::detachGPRS()
{
  if (gsm.getStatus()==gsm.IDLE) return 0;
   
  //gsm._tf.setTimeout(_GSM_CONNECTION_TOUT_);

  //_cell.flush();

  //GPRS dettachment.
  gsm.SimpleWriteln("AT+CGATT=0");
  if(gsm.WaitResp(5000, 50, "OK")!=RX_FINISHED_STR_NOT_RECV)
  {
    gsm.setStatus(gsm.ERROR);
    return 0;
  }
  delay(500);
  
  // Commented in initial trial code!!
  //Stop IP stack.
  //_cell << "AT+WIPCFG=0" <<  _DEC(cr) << endl;
  //	if(!gsm._tf.find("OK")) return 0;
  //Close GPRS bearer.
  //_cell << "AT+WIPBR=0,6" <<  _DEC(cr) << endl;

  gsm.setStatus(gsm.READY);
  return 1;
}

int InetGSM::connectTCP(const char* server, int port)
{
  //gsm._tf.setTimeout(_TCP_CONNECTION_TOUT_);

  //Status = ATTACHED.
  //if (getStatus()!=ATTACHED)
    //return 0;

  //_cell.flush();
  
  //Visit the remote TCP server.
   gsm.SimpleWrite("AT+CIPSTART=\"TCP\",\"");
   gsm.SimpleWrite(server);
   gsm.SimpleWrite("\",");
   gsm.SimpleWriteln(port);
  
  switch(gsm.WaitResp(1000, 200, "OK")){
	case RX_TMOUT_ERR: 
		return 0;
	break;
	case RX_FINISHED_STR_NOT_RECV: 
		return 0; 
	break;
  }
  #ifdef DEBUG_ON
	DEBUG_SERIAL.println("DB:RECVD CMD");
  #endif	
  if (!gsm.IsStringReceived("CONNECT OK")) {
    switch(gsm.WaitResp(15000, 200, "OK")) {
	case RX_TMOUT_ERR: 
		return 0;
	break;
	case RX_FINISHED_STR_NOT_RECV: 
		return 0; 
	break;
    }
  }

  #ifdef DEBUG_ON
	DEBUG_SERIAL.println("DB:OK TCP");
  #endif

  delay(3000);
  gsm.SimpleWriteln("AT+CIPSEND");
  switch(gsm.WaitResp(5000, 200, ">")){
	case RX_TMOUT_ERR: 
		return 0;
	break;
	case RX_FINISHED_STR_NOT_RECV: 
		return 0; 
	break;
  }

  #ifdef DEBUG_ON
	DEBUG_SERIAL.println("DB:>");
  #endif
  delay(4000);
  return 1;
}

int InetGSM::connectUDP(const char* server, int port)
{
  
  //Visit the remote TCP server.
   gsm.SimpleWrite("AT+CIPSTART=\"UDP\",\"");
   gsm.SimpleWrite(server);
   gsm.SimpleWrite("\",");
   gsm.SimpleWriteln(port);
  
  switch(gsm.WaitResp(1000, 200, "OK")){
	case RX_TMOUT_ERR: 
		return 0;
	break;
	case RX_FINISHED_STR_NOT_RECV: 
		return 0; 
	break;
  }
  #ifdef DEBUG_ON
	DEBUG_SERIAL.println("DB:RECVD CMD");
  #endif	
  if (!gsm.IsStringReceived("CONNECT OK")) {
    switch(gsm.WaitResp(15000, 200, "OK")) {
	case RX_TMOUT_ERR: 
		return 0;
	break;
	case RX_FINISHED_STR_NOT_RECV: 
		return 0; 
	break;
    }
  }

  #ifdef DEBUG_ON
	DEBUG_SERIAL.println("DB:OK UDP");
  #endif

#if 0   // let app do encapsulating
  // this is for sending ASCII with ^Z as terminating character
  delay(3000);
  gsm.SimpleWriteln("AT+CIPSEND");
  switch(gsm.WaitResp(5000, 200, ">")){
	case RX_TMOUT_ERR: 
		return 0;
	break;
	case RX_FINISHED_STR_NOT_RECV: 
		return 0; 
	break;
  }

  #ifdef DEBUG_ON
	DEBUG_SERIAL.println("DB:>");
  #endif
  delay(4000);
#else
	// select normal data send mode
  gsm.SimpleWriteln("AT+CIPQSEND=0");
  switch(gsm.WaitResp(5000, 200, "OK")){
	case RX_TMOUT_ERR: 
		return 0;
	break;
	case RX_FINISHED_STR_NOT_RECV: 
		return 0; 
	break;
  }
#endif
  return 1;
}


int InetGSM::disconnectTCP()
{
  //Close TCP/UDP client and deact.
  gsm.SimpleWriteln("AT+CIPCLOSE");
  if(gsm.getStatus()==gsm.TCPCONNECTEDCLIENT)
      	gsm.setStatus(gsm.ATTACHED);
   else
        gsm.setStatus(gsm.TCPSERVERWAIT);   
    return 1;
}

int InetGSM::connectTCPServer(int port)
{
/*
  if (getStatus()!=ATTACHED)
     return 0;
*/
  //gsm._tf.setTimeout(_GSM_CONNECTION_TOUT_);

  //_cell.flush();

  // Set port
  
  gsm.SimpleWrite("AT+CIPSERVER=1,");
  gsm.SimpleWriteln(port);
/*
  switch(gsm.WaitResp(5000, 50, "OK")){
	case RX_TMOUT_ERR: 
		return 0;
	break;
	case RX_FINISHED_STR_NOT_RECV: 
		return 0; 
	break;
  }

  switch(gsm.WaitResp(5000, 50, "SERVER")){ //Try SERVER OK
	case RX_TMOUT_ERR: 
		return 0;
	break;
	case RX_FINISHED_STR_NOT_RECV: 
		return 0; 
	break;
  }
*/
  //delay(200);  

  return 1;

}

boolean InetGSM::connectedClient()
{
  /*
  if (getStatus()!=TCPSERVERWAIT)
     return 0;
  */
  
   gsm.SimpleWriteln("AT+CIPSTATUS");
  // Alternative: AT+QISTAT, although it may be necessary to call an AT 
  // command every second,which is not wise
  /*
  switch(gsm.WaitResp(1000, 200, "OK")){
	case RX_TMOUT_ERR: 
		return 0;
	break;
	case RX_FINISHED_STR_NOT_RECV: 
		return 0; 
	break;
  }*/
  //gsm._tf.setTimeout(1);
  if(gsm.WaitResp(5000, 50, "CONNECT OK")!=RX_FINISHED_STR_RECV)
  {
    gsm.setStatus(gsm.TCPCONNECTEDSERVER);
    return true;
  }
  else
    return false;
 }
bool InetGSM::OpenGprs(int CID,char *apn,int responsetime)
{
	bool success = false;
	cid = CID;
	sprintf(printbuf,"AT+SAPBR=3,%d,\"CONTYPE\",\"GPRS\"\r",cid);
	if (writewithtimeout(printbuf,OK,1000,1000))
	{
		delay(2000);
		sprintf(printbuf,"AT+SAPBR=3,%d,\"APN\",\"%s\"\r",cid,apn);
#ifdef DEBUG_ON
		DEBUG_SERIAL.println("DEBUG:Inet OPENGPRS");
		DEBUG_SERIAL.println(printbuf);
#endif
		if (writewithtimeout(printbuf,OK,10000,150))
		{
			delay(2000);
			sprintf(printbuf,"AT+SAPBR=1,%d\r",cid);
#ifdef DEBUG_ON
			DEBUG_SERIAL.println(printbuf);
			DEBUG_SERIAL.println(responsetime);
#endif
			success = (writewithtimeout(printbuf,OK,responsetime*1000,responsetime*1000));
		}
	}
	return success;
}

bool InetGSM::CloseGprs(int CID)
{
	sprintf(printbuf,"AT+SAPBR=0,%d\r",CID);
#ifdef DEBUG_ON
	DEBUG_SERIAL.println("DEBUG:Inet CLOSEGPRS");
	DEBUG_SERIAL.println(printbuf);
#endif
	return writewithtimeout(printbuf,OK,1000,150);
}

char *InetGSM::GetGprsIP(int CID)
{
	char *cs,*ce;
	sprintf(printbuf,"AT+SAPBR=2,%d\r",CID);
#ifdef DEBUG_ON
	DEBUG_SERIAL.println("DEBUG:Inet GetGPRSIP");
	DEBUG_SERIAL.println(printbuf);
#endif
	if  (writewithtimeout(printbuf,OK,1000,150))
	{
		// reply like this +SAPBR: 1,1,"10.173.225.36"
#ifdef DEBUG_ON
		DEBUG_SERIAL.println("DEBUG:Inet reply");
		DEBUG_SERIAL.println((char *)gsm.comm_buf);
#endif
		cs = strchr((char *)gsm.comm_buf,'\"');
		ce = strchr(++cs,'\"');
		*ce = 0;
		return cs;
	}
	else
		return 0;
}
