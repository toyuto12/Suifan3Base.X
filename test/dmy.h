#ifndef DMY_H
#define DMY_H

#include <stdint.h>
#include <stdbool.h>

typedef union {
    struct {
        unsigned LATB0                  :1;
        unsigned LATB1                  :1;
        unsigned LATB2                  :1;
        unsigned LATB3                  :1;
        unsigned LATB4                  :1;
        unsigned LATB5                  :1;
        unsigned LATB6                  :1;
        unsigned LATB7                  :1;
    };
} LATBbits_t;
LATBbits_t LATBbits;

typedef union {
    struct {
        unsigned SSP1IF                 :1;
        unsigned BCL1IF                 :1;
        unsigned SSP2IF                 :1;
        unsigned BCL2IF                 :1;
        unsigned TXIF                   :1;
        unsigned RCIF                   :1;
    };
} PIR3bits_t;
PIR3bits_t PIR3bits;

typedef union {
    struct {
        unsigned ADGO                   :1;
        unsigned                        :1;
        unsigned ADFM                   :2;
        unsigned ADCS                   :1;
        unsigned                        :1;
        unsigned ADCONT                 :1;
        unsigned ADON                   :1;
    };
    struct {
        unsigned DONE                   :1;
        unsigned                        :1;
        unsigned ADFRM                  :2;
    };
    struct {
        unsigned nDONE                  :1;
    };
    struct {
        unsigned GO                     :1;
        unsigned                        :1;
        unsigned ADFM0                  :1;
        unsigned ADFM1                  :1;
    };
    struct {
        unsigned GO_nDONE               :1;
        unsigned                        :1;
        unsigned ADFRM0                 :1;
        unsigned ADFRM1                 :1;
    };
} ADCON0bits_t;
ADCON0bits_t ADCON0bits;

uint8_t DAC1CON1;
uint8_t RC1REG;
uint8_t ADCON0;
uint16_t ADRES;

#endif
