//Definisanje
#define HIGH 0x1
#define LOW  0x0
#define STANDARDNI_MOD 0
#define SMANJENI_KAPACITET 1
#define LIMITUP  250
#define LIMITDOWN 1500



//Definisanje karaktera
  const unsigned char SEG_MAP[] = {
         0x01,
         0x4F,
         0x12,
         0x06,
         0x4c,
         0x24,
         0x20,
         0x0F,
         0x00,
         0x04

};
//Ubacivanje koda za 7-segmentni displej
  unsigned char display[4] ={SEG_MAP[0],SEG_MAP[0],SEG_MAP[0],SEG_MAP[0]};
  unsigned char indexDisplay = 0;
  unsigned short position;
  unsigned displejVrednost = 0;
  int digit;
  //broj - Broj prikazan na displeju (u milisekundama)
  unsigned int broj = 0;
  //treperenje - Prikazuje brzinu tj. frekvencu treperenja dioda RC4-RC7
  unsigned int treperenje = 200;
  //stanje koji se koristi za promenu stanja, moze biti -  STANDARDNI_MOD i  SMANJENI_KAPACITET
  bit stanje;
  int inf = 0;
  void InitTimer0(){
          T0CON         = 0x88;
          TMR0H         = 0xB1;
          TMR0L         = 0xE0;
          GIE_bit         = 1;
          TMR0IE_bit         = 1;
}
//Kod za gasenje programa
     void Stop(){
        GIE_bit = 0;
        TRISA = 0;
        TRISD = 0;
        PORTA = 0;
        PORTB = 0;
        PORTC = 0;
        PORTD = 0;
 }
void Interrupt(){
//Timer calculator opcije
  if(TMR0IF_bit){
          TMR0IF_bit = 0;
            TMR0H = 0xB1;
            TMR0L = 0xE0;
            LATA= 0x00;
            LATD = display[indexDisplay];
            LATA = position;
             position <<= 1;
             if(position > 8){
                 position = 0x01;
             }
             indexDisplay++;
             if(indexDisplay > 3){
                     indexDisplay = 0;
             }
  }

  //Uslovi modova za rad, ako je neki od 4 buttona na IOC na niskom naponu krece odbrojavanje od 10 nakon koga se program gasi, ako nisu - standard mod
  //broj (broj na displeju) je namerno stavljen na 11000 ms kako bi na displeju prva cifra prikazivanja bila 10
  if(RBIF_bit){
     RBIF_bit = 0;
     if(PORTB.F4 == 0 || PORTB.F5 == 0 || PORTB.F6  == 0|| PORTB.F7 ==0 ){
              stanje = SMANJENI_KAPACITET;
              broj = 11000;
      }
  //Kod koji nakon sto su RB4, RB5, RB6 i RB7 stavljeni na visok napon stavlja sistem u standardni mod rada.
  if(PORTB.F4 == 1 && PORTB.F5 == 1 && PORTB.F6  == 1&& PORTB.F7 ==1 ){
              stanje = STANDARDNI_MOD;
              LATC = 0xF0;
              broj = 0;

      }
  }
}

//Kod pri pokretanju programa
  void Start(){
              //Ukljucivanje i iskljucivanje portova.
              TRISB = 0xF0;
              ANSELB = 0x00;
              ANSELD = 0;
              TRISA = 0;
              TRISD = 0;
              TRISC = 0x0F;
              LATC = 0xF0;
              position = 0x01;
              ADC_Init();
              InitTimer0();
              RBIE_bit = HIGH;
              RBIF_bit = LOW;
              GIE_bit = HIGH;
              //Stavljanje Interrupt on Change portova na High tj. 1
              IOCB4_bit = HIGH;
              IOCB5_bit = HIGH;
              IOCB6_bit = HIGH;
              IOCB7_bit = HIGH;
              //Pocetni mod je standardni
              stanje = STANDARDNI_MOD;
}


void main() {
   Start(); //Pokretanje programa
     //Kod ispod se koristi za 2 dugmeta koji ubrzavaju i smanjuju frekvenciju treperenja
     while(inf != 1){
     //Port RB0 koji sluzi za promenu frekvencije treperenja
        if( Button(&PORTB, 0, 1, 1)) {
                if(treperenje > LIMITDOWN){
                        treperenje =LIMITDOWN;
                }else{
                        treperenje +=100;
                }
        }
         //Port RB1 koji sluzi za promenu frekvencije treperenja
        if( Button(&PORTB, 1, 1, 1)) {
                if(treperenje > LIMITUP){
                        treperenje =LIMITUP;
                }else{
                        treperenje -=100;
                }
        }
        //Prikazivanje digita na displeju
        if(broj % 1000 == 0){
                displejVrednost = broj/1000;
                //Prvi digit
                digit = (displejVrednost/1000)% 10;
                display[0] = SEG_MAP[digit];
                //Drugi digit
                digit = (displejVrednost/100) % 10;
                display[1] = SEG_MAP[digit];
                //Treci digit
                digit = (displejVrednost/10) % 10;
                display[2] = SEG_MAP[digit];
                //Cetvrti digit
                digit = displejVrednost  % 10;
                display[3] = SEG_MAP[digit];

         }
         Delay_us(1000);

         //Uslovi za Smanjeni mod gde se program gasi pod zadatim uslovima
         if(stanje == SMANJENI_KAPACITET){
                 broj--;
                if(PORTB.F4 == 0 && PORTB.F5 == 0 && PORTB.F6  == 0&& PORTB.F7 ==0 ){
                        GIE_bit = 0;
                        Stop();
                }
                //Nakon vracanja napona na IOC portove sistem krece da radi ponovo
                if(PORTB.F4 == 1 && PORTB.F5 == 1 && PORTB.F6  == 1&& PORTB.F7 ==1 ){

                         TRISB = 0xF0;
                         ANSELB = 0x00;
                         ANSELD = 0;
                         TRISA = 0;
                         TRISD = 0;
                         TRISC = 0x0F;
                         LATC = 0xF0;
                         RBIE_bit = HIGH;
                         RBIF_bit = LOW;
                         GIE_bit = HIGH;
                        IOCB4_bit = HIGH;
                        IOCB5_bit = HIGH;
                        IOCB6_bit = HIGH;
                        IOCB7_bit = HIGH;
                        stanje = STANDARDNI_MOD;
                         broj = 0;
                        }
                //Treperenje dioda i kod koji nakon sto broj (broj na displeju) stigne sa 10 na 0 gasi ceo program
                if(PORTB.F4 == 0){
                        if(broj % treperenje == 0){
                                LATC4_bit = ~LATC4_bit ;
                                if(broj ==0){
                                Stop();
                                return;
                        }
                }
          }
                if(PORTB.F5 == 0){
                        if(broj % treperenje == 0){
                                LATC5_bit = ~LATC5_bit;
                                if(broj ==0){
                                Stop();
                                return;
                        }
                }
          }
                if(PORTB.F6 == 0){
                        if(broj % treperenje == 0){
                                LATC6_bit = ~LATC6_bit;
                                if(broj ==0){
                                Stop();
                                return;
                        }
                }
          }
                if(PORTB.F7 == 0){
                         if(broj % treperenje == 0){
                                 LATC7_bit = ~LATC7_bit;
                                      if(broj ==0){
                                      Stop();
                                return;
                         }
                }
          }     //Else koji se koristi da kada se ponovo dugme vrati na visok napon nastavlja da broji unapred.
                } else{

                         broj++;
                      }

          }

}