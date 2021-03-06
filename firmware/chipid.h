
#ifndef CHIP_ID_H
#define CHIP_ID_H 1

#if defined(__AVR_ATtiny40__)
#  define MICRONUCLEUS_CHIP_ID	1
#elif defined(__AVR_ATtiny44__)
#  define MICRONUCLEUS_CHIP_ID	2
#elif defined(__AVR_ATtiny84__)
#  define MICRONUCLEUS_CHIP_ID	3
#elif defined(__AVR_ATtiny45__)
#  define MICRONUCLEUS_CHIP_ID	4
#elif defined(__AVR_ATtiny85__)
#  define MICRONUCLEUS_CHIP_ID	5
#elif defined(__AVR_ATtiny461__)
#  define MICRONUCLEUS_CHIP_ID	6
#elif defined(__AVR_ATtiny861__)
#  define MICRONUCLEUS_CHIP_ID	7
#elif defined(__AVR_ATtiny4313__)
#  define MICRONUCLEUS_CHIP_ID	8
#elif defined(__AVR_ATtiny43U__)
#  define MICRONUCLEUS_CHIP_ID	9
#elif defined(__AVR_ATtiny48__)
#  define MICRONUCLEUS_CHIP_ID	10
#elif defined(__AVR_ATtiny88__)
#  define MICRONUCLEUS_CHIP_ID	11
#elif defined(__AVR_ATtiny828__)
#  define MICRONUCLEUS_CHIP_ID	12
#elif defined(__AVR_ATtiny87__)
#  define MICRONUCLEUS_CHIP_ID	13
#elif defined(__AVR_ATtiny167__)
#  define MICRONUCLEUS_CHIP_ID	14
#  ifndef VECTOR_SIZE
#    define VECTOR_SIZE 4
#  endif
#elif defined(__AVR_ATtiny1634__)
#  define MICRONUCLEUS_CHIP_ID	15
#  ifndef VECTOR_SIZE
#    define VECTOR_SIZE 4
#  endif
#else
#  warning "Your chips family is not recognized."
#  define MICRONUCLEUS_CHIP_ID	255
#endif

#ifndef VECTOR_SIZE
#  define VECTOR_SIZE 2
#endif

#endif
