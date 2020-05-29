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

// ���荞��

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

// �h���C�o

// AD�l���ړ����ςŎ�荞��(CH=AN0�Œ�)
static uint16_t adTmp[16];
static uint8_t rng=0;
void TaskAdc(void){
	if( !ADCON0bits.GO ){
		adTmp[rng] = ADRES;
		rng = (rng+1) &0x0F;
		ADCON0 = 0x03;
	}
}

// �A�v���P�[�V����

// AD�l��ǂݏo��(CH=AN0�Œ�)
uint16_t ReadAdc( void ){
	uint16_t sum=0,i=16;
	while(i--) sum += adTmp[i];
	return sum>>4;
}

// AD�l���Ď����X���b�V�����h��A��������
#define BASE_AD			163			// 0.8V	�����l
#define UPPER_AD		120			// 0.6V	���ݒl�{���̒l�ȏ�����o�����ꍇ�A����(�ʐMHigh�o�͂Ɣ��f)
#define THRESHOLD_AD	200			// 1.0V	���ݒl�{���̒l���R���p���[�^��臒l�ɂ���B
void SetDacValue( uint16_t ad ){
	static uint16_t baseAd = BASE_AD;
	uint16_t tmp;
	
	if( ad < (baseAd+UPPER_AD) ) {
		if( ad >baseAd ) baseAd ++;
		else if( ad < baseAd ) baseAd --;

		tmp = (baseAd +THRESHOLD_AD) >>5 ;	// ��l�Z�o��10->5bit�ϊ�
		DAC1CON1 = tmp;
	}
}


// �d�͐��ʐM�̎�M�������s��
// ��M���튮���ɂ�*dat�Ƀf�[�^�𗬂�����
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

