#include "hw.h"
#include "stat.h"
uint8_t g_nState = PLAY_STATE;

int main(void)
{
	HW_Init();
	
	while(1)
	{	
		if(g_nState == PLAY_STATE)
			PlayService();
		else if(g_nState == ESR_STATE)
			EsrBegin();
		else if(g_nState == PWRDOWN_STATE || g_nState == SLEEP_STATE){
			PlayPwrDownAduio((PEN_STATE)g_nState);
			HW_EnterPowerDown((PEN_STATE)g_nState);
		}else if(g_nState == RECORD_STATE)
			RecordBegin(RECUSEDCODE,REC_PURE);
		else if(g_nState == MVREC_STATE){
			RecordBegin(MVUSEDCODE,REC_MAVO);
			PlayMagicVoice();
		}
	}	
}
