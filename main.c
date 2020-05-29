#ifndef TDD
#include "mcc_generated_files/mcc.h"
#endif

#define oMAINFAN_HIGH	LATBbits.LATB5
#define oMAINFAN_MID	LATBbits.LATB4
#define oMAINFAN_LOW	LATBbits.LATB3
#define oLR_FAN			LATBbits.LATB2
#define oUD_FAN			LATBbits.LATB1

typedef struct{
	uint8_t FanLevel	:2 ;
	uint8_t LRFanOn		:1 ;
	uint8_t UDFanOn		:1 ;
	uint8_t Check1		:4 ;
}stCommData;

stCommData gComm;
uint8_t gInterval;

// 割り込み

void isrTimer0(void){
	gInterval ++;
}

#define MAIN_DEADTIME	5
static uint8_t sMainFanChangeDeadtime;
static uint8_t exMainFanLevel;
void isrZeroCrossCapture(void){
	if( gComm.FanLevel == exMainFanLevel ){
		sMainFanChangeDeadtime = MAIN_DEADTIME;
		oMAINFAN_HIGH = ( exMainFanLevel == 1 ) ?1 :0 ;
		oMAINFAN_MID = ( exMainFanLevel == 2 ) ?1 :0 ;
		oMAINFAN_LOW = ( exMainFanLevel == 3 ) ?1 :0 ;
	}else if( sMainFanChangeDeadtime ){
		sMainFanChangeDeadtime --;
		oMAINFAN_HIGH = 0;
		oMAINFAN_MID = 0;
		oMAINFAN_LOW = 0;
	}else{
		exMainFanLevel = gComm.FanLevel;
	}
	
	oUD_FAN = gComm.UDFanOn;
	oLR_FAN = gComm.LRFanOn;
}

// ドライバ

// AD値を移動平均で取り込む(CH=AN0固定)
static uint16_t adTmp[16];
static uint8_t rng=0;
void TaskAdc(void){
	if( !ADCON0bits.GO ){
		adTmp[rng] = ADRES;
		rng = (rng+1) &0x0F;
		ADCON0 = 0x03;
	}
}

// アプリケーション

// AD値を読み出す(CH=AN0固定)
uint16_t ReadAdc( void ){
	uint16_t sum=0,i=16;
	while(i--) sum += adTmp[i];
	return sum>>4;
}

// AD値を監視しスレッショルドを連動させる
#define BASE_AD			163			// 0.8V	初期値
#define UPPER_AD		120			// 0.6V	現在値＋この値以上を検出した場合、無視(通信High出力と判断)
#define THRESHOLD_AD	200			// 1.0V	現在値＋この値をコンパレータの閾値にする。
void SetDacValue( uint16_t ad ){
	static uint16_t baseAd = BASE_AD;
	uint16_t tmp;
	
	if( ad < (baseAd+UPPER_AD) ) {
		if( ad >baseAd ) baseAd ++;
		else if( ad < baseAd ) baseAd --;

		tmp = (baseAd +THRESHOLD_AD) >>5 ;	// 基準値算出と10->5bit変換
		DAC1CON1 = tmp;
	}
}


// 電力線通信の受信処理を行う
// 受信正常完了にて*datにデータを流し込む
void ReceiveData( stCommData *dat ){
	static uint8_t sTimeout = 50;
	static uint8_t pos,buf[3],tmp,exTmp;
	uint8_t *pDat = (uint8_t *)dat;
	
	if( PIR3bits.RCIF ){
		PIR3bits.RCIF = false;
		sTimeout = 50;
		tmp = RC1REG;
		if( (tmp+exTmp) == 0xff ){
			buf[pos++] = exTmp;
			if( pos >= 2 ){
				if( buf[0] == 'S' ){
					pDat[0] = buf[1];
					sTimeout = 0;
				}
			}
		}
		exTmp = tmp;
	}else if( sTimeout ) sTimeout --;
	
	if( !sTimeout ){
		pos = 0;
		exTmp = 0xAA;
	}
}

#ifndef TDD
void main(void)
{
    // initialize the device
    SYSTEM_Initialize();
	
	TMR0_SetInterruptHandler(isrTimer0);
	INT_SetInterruptHandler(isrZeroCrossCapture);
			
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();

    while (1){
		if( gInterval ){
			gInterval --;
			
			TaskAdc();
			SetDacValue( ReadAdc() );
			
			ReceiveData( &gComm );

		}
    }
}
#endif

