#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

char IBuf[256], OBuf[256];
uint8_t IBufIt=0, IBufSz=0, OBufIt=0, OBufSz=0;

// inicjalizacja UART
void uart_init(){
  UBRR0 = UBRR_VALUE;
  UCSR0A = 0;
  UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0) | _BV(TXCIE0) | _BV(UDRIE0);
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01); // 8n
}

// transmisja jednego znaku AKA dopisz do bufora
int uart_transmit(char data, FILE *stream){
  // czekaj aż transmiter gotowy
  IBuf[IBufIt = IBufIt == sizeof(IBuf) ? 0 : IBufIt+1] = data;
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream){
  // czekaj aż znak dostępny
  while (!(UCSR0A & _BV(RXC0)));
  return UDR0;
}

FILE uart_file;


ISR(USART_RX_vect){
  IBuf[IBufIt = IBufIt == sizeof(IBuf) ? 0 : IBufIt+1] = UDR0;
}

ISR(USART_UDRE_vect){ // ready to receive
  UDR0 = OBuf[OBufIt = OBufIt == sizeof(OBuf) ? 0 : OBufIt+1];

}

ISR(USART_TXC_vect){

}

int main()
{
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;


  // program testowy
  printf("Hello world!\r\n");
  while(1) {
    int16_t a = 1;
    scanf("%"SCNd16, &a);
    printf("Odczytano: %"PRId16"\r\n", a);
  }
}

