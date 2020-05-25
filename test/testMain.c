#include "dmy.h"

typedef struct{
	uint8_t FanLevel	:2 ;
	uint8_t LRFanOn		:1 ;
	uint8_t UDFanOn		:1 ;
	uint8_t Check1		:4 ;
}stCommData;

void ReceiveData( stCommData *dat );

void UartDataInput( uint8_t d, stCommData *in ){
	PIR3bits.RCIF = true;
	RC1REG = d;
	ReceiveData( in );
}

void test_ReceiveData(void){
	stCommData d;
	uint8_t *pd = (uint8_t *)&d;
	uint8_t testData = 0x93;

	// 正常成功
	UartDataInput( 'S', &d );
	UartDataInput( ~'S', &d );
	UartDataInput( testData, &d );
	cut_assert_equal_int( 0, *pd );
	UartDataInput( ~testData, &d );
	cut_assert_equal_int( testData, *pd );

	// 正常失敗
	testData = 0x87;
	UartDataInput( 'S', &d );
	UartDataInput( ~'S', &d );
	UartDataInput( testData, &d );
	UartDataInput( testData, &d );
	cut_assert_not_equal_int( testData, *pd );

	// 正常：タイムアウト
	testData = 0x45;
	UartDataInput( 'S', &d );
	UartDataInput( ~'S', &d );
	UartDataInput( testData, &d );
	uint16_t i;
	for( i=0; i<60; i++ ) ReceiveData( 0 );
	UartDataInput( ~testData, &d );
	cut_assert_not_equal_int( testData, *pd );

	// 正常：タイムアウト以前の正常受信
	UartDataInput( 'S', &d );
	UartDataInput( ~'S', &d );
	UartDataInput( testData, &d );
	for( i=0; i<40; i++ ) ReceiveData( 0 );
	UartDataInput( ~testData, &d );
	cut_assert_equal_int( testData, *pd );
}


