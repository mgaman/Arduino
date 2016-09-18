#ifndef _CALL_H_
#define _CALL_H_

#include "GenericGSM.h"

class callGSM
{
	public:
		    // finds out the status of call
    call_ret_val_enum CallStatus(void);
    byte CallStatusWithAuth(char *phone_number,
                            byte first_authorized_pos, byte last_authorized_pos);
    // picks up an incoming call
    void PickUp(void);
    // hangs up an incomming call
    void HangUp(void);
    // calls the specific number
    void Call(char *number_string);
    // makes a call to the number stored at the specified SIM position
    void Call(int sim_position);
	void SendDTMF(char *number_string, int time);
	
    void SetDTMF(int DTMF_status);
	char DetDTMF();

};
#endif

