; GPS data to aprs 1200 baud afsk mic-e by OE5DXL
; <oe5dxl@oe5xbl.#oe5.aut.eu, oe5dxl@oevsv.at>
;     This program is free software: you can redistribute it and/or modify
;     it under the terms of the GNU General Public License as published by
;     the Free Software Foundation, either version 3 of the License, or
;     (at your option) any later version.
;
;     This program is distributed in the hope that it will be useful,
;     but WITHOUT ANY WARRANTY; without even the implied warranty of
;     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;     GNU General Public License for more details.
;
;     You should have received a copy of the GNU General Public License
;     along with this program.  If not, see <http://www.gnu.org/licenses/>.
;
; assembler:  gavrasm or avrstudio   (gavrasm -S aprs.asm)
; attiny13 attiny2313 atmega88  
; 07.04.2013 oe5hpm added support for burning 2nd config over serial interface
;                   if first character after #Pe is '1' 0xff in eeprom is searched and following config is burnt there
;                   otherwise following config is burnt in eeprom beginning at location 0

;.EQU avrstudioATMEGA88=1         ;fuses(low): ef  fuses(high): df
;.EQU avrstudioATtiny2313=1      ;fuses(low): ef  fuses(high): df
.EQU avrstudioATtiny13=1         ;fuses(low): 38  fuses(high): fb



.IFDEF avrstudioATMEGA88
.include "m88def.inc"      
.ENDIF
.IFDEF avrstudioATtiny2313
.include "tn2313def.inc" 
.ENDIF
.IFDEF avrstudioATtiny13
.include "tn13def.inc"
.ENDIF

.EQU ramstart=sram_start    

;------------------------------------- configuration
.IFDEF avrstudioATMEGA88
.EQU CPUCLOCK=20000000                        ; /245/34
.EQU switchdmask=0b00001100                   ; config switches input bits
.EQU countmask=  0b00000100                   ; decrement switchmask by this to count through
;.EQU switchbbit=4
.EQU indbit=4                                 ; uart input bit
;.EQU inbbit=2                                 ; uart input bit
.EQU pttdbit=5                                ; ptt pin
.EQU portbout=0b00000000
.EQU portdout=0b01000000                      ; pd6 oc0a = pwm out
.ENDIF

.IFDEF avrstudioATtiny2313
.EQU CPUCLOCK=10000000                        ; /245/34
.EQU inbbit=0                                 ; uart input bit
.EQU pttbbit=1                                ; ptt pin
.EQU portbout=0b00000100                      ; pwm out
.ENDIF

.IFDEF avrstudioATtiny13
.EQU CPUCLOCK=10000000                        ; /245/34
.EQU switchbbit=4                             ; config switch input bit
.EQU inbbit=2                                 ; uart input bit
.EQU pttbbit=1                                ; ptt pin
.EQU portbout=0b00000001                      ; pwm out
.ENDIF
;------------------------------------- configuration


;---uart
.EQU BAUD=9600
.EQU bauddiv=(CPUCLOCK+BAUD/2)/BAUD-1
.EQU PRESCALE=high(bauddiv)+1
.EQU baudclk=bauddiv/PRESCALE
.EQU linefeed=10

;---AX.25
.EQU POLINOM=0b1000010000001000
.EQU FLAG=126
.EQU TXTAIL=2
.EQU STUFFLEN=5
.EQU AXBAUD=1200
.EQU AXSAMP=CPUCLOCK/256
.EQU BAUDINC=AXBAUD*65536/AXSAMP
.EQU LOFREQ=1200*65536/AXSAMP
.EQU HIFREQ=2200*65536/AXSAMP



.IFDEF pttbbit
 .EQU portboutmask=portbout | (1<<pttbbit)
.ENDIF
.IFDEF pttdbit
 .EQU portdoutmask=portdout | (1<<pttdbit)
.ENDIF


.IFDEF pcmsk
  .EQU pcmskx=pcmsk
.ENDIF

.IFDEF inbbit
  .IFDEF pcmsk0
    .EQU pcmskx=pcmsk0
    .EQU pcicrmask=0b00000001
  .ENDIF
.ENDIF

.IFDEF incbit
  .IFDEF pcmsk1
    .EQU pcmskx=pcmsk1
    .EQU pcicrmask=0b00000010
  .ELSE
    .ERROR cannot enable pcint
  .ENDIF
.ENDIF

.IFDEF indbit
  .IFDEF pcmsk2
    .EQU pcmskx=pcmsk2
    .EQU pcicrmask=0b00000100
  .ELSE 
    .ERROR cannot enable pcint
  .ENDIF
.ENDIF


.DEF sregsave=r0
.DEF rshigh=r1
.DEF prescaler=r2
.DEF temp5=r3
.DEF crclo=r4
.DEF crchi=r5
.DEF ddsl=r6
;.DEF higthhh=r6
.DEF ddsh=r7
.DEF freq1=r8
.DEF freq1h=r9
.DEF freq2=r10
.DEF seconds=r10
.DEF freq2h=r11
.DEF freq=r12
.DEF freqh=r13
.DEF higth=r14
.DEF higthh=r15
.DEF temp=r16
.DEF temp1=r17
.DEF bitcnt=r17
.DEF temp2=r18
.DEF stuffcnt=r18
.DEF temp3=r19
.DEF shift=r19
.DEF temp4=r20
.DEF state=r20
.DEF isrwork=r21
.DEF rslow=r22
.DEF textend=r23
.DEF baudcl=r24
.DEF baudch=r25



.IFDEF timsk0
  .EQU TIMSKX=timsk0
.ENDIF
.IFDEF timsk
  .EQU TIMSKX=timsk
.ENDIF

.IFDEF TIFR0
  .EQU TIFRX=TIFR0
.ENDIF
.IFDEF TIFR
  .EQU TIFRX=TIFR
.ENDIF


;--- uart ringbuffer
.EQU RINGBUFFERLEN=4                         ; size of uart fifo (max 255)                         
.DEF ixl=r26
.DEF ixh=r27
.EQU ringbuf=ramstart                        ; start of ringbuffer
.DEF iyl=r28
.DEF iyh=r29
.DEF izl=r30
.DEF izh=r31
.EQU ringend=ringbuf+RINGBUFFERLEN
.IF high(ringbuf)!=high(ringend)
.ERROR ringbuffer must be inside a 256 byte page
.ENDIF
;--- uart ringbuffer

.EQU notextcnt=ringend
.EQU framebuf=notextcnt+1

       rjmp reset

.IFDEF avrstudioATTINY13
       reti
       rjmp pcisr                   ; pin change interrupt vector
       reti
       reti
       reti
       rjmp timer0a                 ; timer0 compare A interrupt vector 
       reti
       reti
       reti
.ENDIF

.IFDEF avrstudioATTINY2313
       reti
       reti
       reti
       reti
       reti
       reti
       reti
       reti
       reti
       reti
       rjmp pcisr                   ; pin change interrupt vector
       reti
       rjmp timer0a                 ; timer0 compare A interrupt vector
       reti
       reti
       reti
       reti
       reti
.ENDIF

.IFDEF avrstudioATMEGA88
       reti
       reti

.IFDEF inbbit
       rjmp pcisr                   ; pin change interrupt vector
.ELSE
       reti
.ENDIF

.IFDEF incbit
       rjmp pcisr                   ; pin change interrupt vector
.ELSE
       reti
.ENDIF

.IFDEF indbit
       rjmp pcisr                   ; pin change interrupt vector
.ELSE
       reti
.ENDIF

       reti
       reti
       reti
       reti
       reti
       reti
       reti
       reti
       rjmp timer0a                 ; timer0 compare A interrupt vector
       reti
       reti
       reti
       reti
       reti
.ENDIF

sintab:
.DB  128,134,141,147,153,159,165,171,177,183,188,194,199,204,209,214,218,223,227,231,234,237,241,243,246,248,250,252,253,254,255,255

timer0a:
         in   sregsave, SREG

.IF PRESCALE > 1
         dec  prescaler             ; software timer prescaler
         brne tireti

         ldi  isrwork, PRESCALE
         mov  prescaler, isrwork    ; software timer prescaler
.ENDIF
 
         ldi  isrwork, baudclk      ; set full step
         out  ocr0a, isrwork

         sbrc rshigh,6              ; byte shiftet in?
         rjmp stopstep              ; middle of stopstep
         lsr  rslow
         ror  rshigh

;   sbi PINB, 6

.IFDEF inbbit
         sbic pinb, inbbit          ; read data bit
.ENDIF
.IFDEF incbit
         sbic pinc, incbit          ; read data bit
.ENDIF
.IFDEF indbit
         sbic pind, indbit          ; read data bit
.ENDIF
         sbr  rslow, 0b10000000     ; copy data bit to shift reg

         clr  isrwork               ; stop pin change int
.IF pcmskx>=64
         sts  pcmskx, isrwork
.ELSE
         out  pcmskx, isrwork
.ENDIF
         ldi  isrwork, 1
         out  tccr0b, isrwork       ; start timer

tireti:
         out  SREG, sregsave
         reti

stopstep:
         clr  isrwork
         out  tccr0b, isrwork       ; stop timer
.IFDEF inbbit
         sbic pinb, inbbit          ; read data bit (stop step)
.ENDIF
.IFDEF incbit
         sbic pinc, incbit          ; read data bit (stop step)
.ENDIF
.IFDEF indbit
         sbic pind, indbit          ; read data bit (stop step)
.ENDIF
         rjmp invrs
         com  rshigh                ; exor with startstep polarity
         com  rslow
invrs:
         bst  rshigh, 7             ; start xor stopbit 
         brts frameerr              ; wrong stopstep

         st   x+, rslow             ; store byte to fifo
         cpi  ixl, low(ringend)
         brne noring

         ldi  ixl, low(ringbuf)
noring:
frameerr:
.IFDEF inbbit
         ldi  isrwork, (1<<inbbit)   ; start pin change int for next startstep
.ENDIF
.IFDEF incbit
         ldi  isrwork, (1<<incbit)   ; start pin change int for next startstep
.ENDIF
.IFDEF indbit
         ldi  isrwork, (1<<indbit)   ; start pin change int for next startstep
.ENDIF
.IF pcmskx>=64
         sts  pcmskx, isrwork
.ELSE
         out  pcmskx, isrwork
.ENDIF
;         sbi  pcmskx, inbit          ; start pin change int for next startstep
         rjmp tireti 

pcisr:
         in   sregsave, SREG

         clr  rshigh 
         out  tcnt0, rshigh         ; start from 0 time
         ldi  isrwork,baudclk/2-34/PRESCALE    ; correct isr to port read delay
         out  ocr0a, isrwork        ; set half step, next timer int middle of startstep  
         ldi  isrwork, 1
         out  tccr0b, isrwork       ; start timer
         ldi  rslow,0b10000000      ; shift register
.IF PRESCALE > 1
         ldi  isrwork, PRESCALE
         mov  prescaler, isrwork    ; software timer prescaler
.ENDIF

         out  SREG, sregsave
         reti

reset:

         ldi  r16, low(ramend)
         out  spl, r16
.IF ramend>=256
         ldi  r16, high(ramend)
         out  sph, r16
.ENDIF

;---init parallel port, uart in
.IFDEF portboutmask
         ldi r16, portboutmask
         out DDRB, r16
.ENDIF
.IFDEF portdoutmask
         ldi r16, portdoutmask
         out DDRD, r16
.ENDIF
.IFDEF avrstudioATMEGA88
         ldi r16, 0b11001111
         sts PRR, r16
.ENDIF

.IFDEF switchbbit
         sbi  PORTB, switchbbit        ; pullup text switch input
.ENDIF
.IFDEF switchdbit
         sbi  PORTD, switchdbit        ; pullup text switch input
.ENDIF
.IFDEF switchdmask
         ldi  r16, switchdmask
         out  PORTD, r16               ; pullup text switch input
.ENDIF
;---rs232


.IFDEF gimsk
         ldi  r16, (1<<PCIE)
         out  GIMSK, r16
.ENDIF
.IFDEF pcicr
         ldi  r16, pcicrmask
         sts  PCICR, r16
.ENDIF
         ldi  r16, 0b10000000
         out  ACSR, r16               ; disable analog comparator to save power

.IFDEF inbbit
         ldi  r16, (1<<inbbit)         ; start pin change int for next startstep
.ENDIF
.IFDEF incbit
         ldi  r16, (1<<incbit)         ; start pin change int for next startstep
.ENDIF
.IFDEF indbit
         ldi  r16, (1<<indbit)         ; start pin change int for next startstep
.ENDIF
.IF PCMSKX>=64
         sts  PCMSKX, r16
.ELSE
         out  PCMSKX, r16
.ENDIF


frameloop:

;---ringbuf
         ldi  ixl, low(ringbuf)
         ldi  ixh, high(ringbuf)

         movw iyl, ixl
;---ringbuf

;---init timer
         clr  higthh                  ; set invalid higth
         out  OCR0A, r16
         ldi  r16, 2
         out  TCCR0A, r16 
         ldi  r16, 1
         out  TCCR0B, r16
         ldi  r16, (1<<OCIE0A)
.IF TIMSKX>=64 
         sts  TIMSKX, r16
.ELSE
         out  TIMSKX, r16
.ENDIF

         sei
         clr   crclo
         clr   crchi

waitline:
.IFDEF pttbbit
         cbi   PORTB, pttbbit
.ENDIF
.IFDEF pttdbit
         cbi   PORTD, pttdbit
.ENDIF
         ldi  izl, low(framebuf)
.IF ramend>=256
         ldi  izh, high(framebuf)
.ENDIF
;         rcall waitch
;         cpi   temp1, linefeed
;         brne  waitline

         rcall waitch
         cpi   temp1, '$'
         brne  waitline

         rcall waitch
         cpi   temp1, '#'
         brne  gps

; --- parameters to eeprom
         rcall waitch
         cpi   temp1, 'P'
         brne  waitline

         rcall waitch
         cpi   temp1, 'e'
         brne  waitline

         clr  temp                            ; eeprom address
.IFDEF avrstudioATMEGA88
         out  EEARH, temp                     ; eeprom address high
.ENDIF
         out  EEARL, temp                     ; eeprom address low

         rcall waitch
         cpi   temp1, '1'                     ; wenn '1' dann naechste cfg im eeprom suchen
         brne  burnget
findCfg:
         rcall geteeprom                      ; 255 for end of first text
         inc   temp
         brne  findCfg

burnget:
         rcall getsigned
         st    z+, freq 
         cpi   temp1, ','-'0'
         breq  burnget

         mov  textend, izl
         ldi  izl, low(framebuf)
.IF ramend>=256
         ldi  izh, high(framebuf)
.ENDIF
         cli                                  ; no interrupts
.IFDEF pttbbit  
         sbi  PORTB, pttbbit                  ; show burn on ptt
.ENDIF
.IFDEF pttdbit
         sbi  PORTD, pttdbit                  ; show burn on ptt
.ENDIF
         in temp1, EEARL                      ; restore address from previous eeprom transaction
.IFDEF avrstudioATMEGA88
         in temp2, EEARH
burnloop:
         sbic eecr, eepe                      ; eeprom ready
         rjmp burnloop

         clr  temp
         out  EECR, temp                      ; erase + write
         out  EEARH, temp2
         out  EEARL, temp1
         ld   temp, z+
         out  EEDR, temp                      ; data byte
         sbi  EECR, EEMPE                     ; master write
         sbi  EECR, EEPE                      ; write
         inc  temp1                           ; eeprom address
         brne noeeh

         inc  temp2
noeeh:
         cp   izl, textend
         brne burnloop
.ELSE
burnloop:
         sbic eecr, eepe                      ; eeprom ready
         rjmp burnloop

         clr  temp
         out  EECR, temp                      ; erase + write
         out  EEARL, temp1
         ld   temp, z+
         out  EEDR, temp                      ; data byte
         sbi  EECR, EEMWE                     ; master write
         sbi  EECR, EEWE                      ; write
         inc  temp1                           ; eeprom address
         cp   izl, textend
         brne burnloop
.ENDIF
         rjmp frameloop


gps:
         cpi   temp1, 'G'
         brne  waitline
         rcall waitch
;         cpi   temp1, 'P'
;         brne  waitline
         rcall waitch
         cpi   temp1, 'R'
         breq  rmc
;- gga
         cpi   temp1, 'G'
         brne  waitline
         rcall waitch
         cpi   temp1, 'G'
         brne  waitline
         rcall waitch
         cpi   temp1, 'A'
         brne  waitline

         ldi   temp2, 9
skipgga:
         rcall skiprest
         dec   temp2
         brne  skipgga
;---heigth

         rcall getsigned
         ldi   temp, low(10000)
         add   freq, temp
         ldi   temp, high(10000)
         adc   freqh, temp
         movw  higth, freq                    ; higth + 10000 
jwaitline:
         rjmp  waitline 

rmc:
         rcall waitch
         cpi   temp1, 'M'
         brne  jwaitline
         rcall waitch
         cpi   temp1, 'C'
         brne  jwaitline

         rcall skiprest
         rcall skiprest                       ; time
         rcall waitch
         cpi   temp1, 'A'                     ; gps data valid  
         brne  jwaitline

;.IFDEF pttbbit
;         sbi  PORTB, pttbbit                  ; show valid gps data
;.ENDIF
;.IFDEF pttdbit
;         sbi  PORTD, pttdbit                  ; show valid gps data
;.ENDIF

; --- set config start in eeprom
         clr   temp
         out   EEARL, temp                    ; eeprom read pointer 
.IFDEF EEARH
         out   EEARH, temp
.ENDIF

.IFDEF switchbbit
         sbic  pinb, switchbbit
  .EQU doubletext=1
.ENDIF
.IFDEF switchdbit
         sbic  pind, switchdbit
  .EQU doubletext=1
.ENDIF

.IFDEF doubletext
         rjmp  firsttext                      ; do not search next text start
find255:
         rcall geteeprom                      ; 255 for end of first text
         inc   temp
         brne  find255
firsttext:
.ENDIF

.IFDEF switchdmask
         in    temp1, pind
         com   temp1
         andi  temp1, switchdmask
nexttext:
         breq  textfound
find255:
         rcall geteeprom                      ; 255 for end of first text
         inc   temp
         brne  find255
         
         subi  temp1, countmask
         rjmp  nexttext  
textfound:
.ENDIF

         inc   crclo                          ; count valid rmc lines as timer
         rcall geteeprom                      ; send interval
         cp    crclo, temp
         brlo  jwaitline

         clr   crclo

         rcall skiprest
         ldi   temp2, 32
         rcall storech                        ; alt10 + 32
         rcall storech                        ; alt1  + 32
         clr   temp2
         rcall storech                        ; altmin10
         rcall storech                        ; altmin1
         rcall waitch                         ; "."
         rcall storech                        ; altmin0.1
         rcall storech                        ; altmin0.01
         rcall skiprest

         rjmp  cpcall
callloop:
         st    z+, temp
cpcall:
         rcall geteeprom                      ; ssid of destination, source call+ssid, cmd, pid, mic-e 
         brne  callloop

         rcall waitch                         ; "N" "S"
         cpi   temp1, 'N'   
         brne  nonorth

         lds   temp1, framebuf+3
         subi  temp1, -64
         sts   framebuf+3, temp1
nonorth:
         rcall skiprest
         clr   temp1
         rcall waitdec                        ; long100
         rcall waitdec                        ; long10
         rcall waitdec                        ; long1
         ldi   temp2, 28
         cpi   temp1, 10
         brge  long10

         ldi   temp2, 118 
         rjmp  stolong32
long10:
         cpi   temp1, 100
         brlo  stolong

         ldi   temp2, 8
         cpi   temp1, 110
         brlt  stolong32

         ldi   temp2, -72
stolong32:
         lds   temp, framebuf+4
         subi  temp, -64
         sts   framebuf+4, temp
stolong:
         add   temp1, temp2
         st    z+, temp1

         clr   temp1
         rcall waitdec                        ; longmin10
         rcall waitdec                        ; longmin1
         cpi   temp1, 10
         brge  noad60

         subi  temp1, -60
noad60:
         subi  temp1, -28
         st    z+, temp1

         rcall waitch                         ; "."
         clr   temp1
         rcall waitdec                        ; longmin0.1
         rcall waitdec                        ; longmin0.01
         subi  temp1, -28
         st    z+, temp1

         rcall skiprest
         rcall waitch                         ; "W" "E"
         cpi   temp1, 'W'
         brne  nowest

         lds   temp1, framebuf+5
         subi  temp1, -64
         sts   framebuf+5, temp1
nowest:
         rcall skiprest
         rcall getbcd                         ; speed

; ---test speed
         rcall geteeprom                      ; speed limit
         tst   temp4                       
         brne  fast                           ; >=100mph

         mov   freq, temp3
         rcall mul10
         add   freq, temp2
         cp    freq, temp
         brcs  slow
fast:
         ldi   temp, 255
         mov   crchi, temp   
slow:
         rcall geteeprom                      ; send interval high speed
         cp    crchi, temp
         brcc  sendnow

         inc   crchi
         rjmp  waitline
; ---test speed

sendnow:
         mov   freq, temp4
         rcall mul10
         add   temp3, freq
         subi  temp3, -28
         st    z+, temp3                      ; speed10

         mov   freq, temp2                    ; speed1
         rcall getbcd                         ; course
         rcall mul10                          ; speed1*10
         add   temp4, freq                    ; course100
         subi  temp4, -32
         st    z+, temp4

         mov   freq, temp3                    
         rcall mul10                          ; course10*10
         add   temp2, freq                    ; course1
         subi  temp2, -28
         mov   temp, temp2
sybolloop:
         st    z+, temp
         rcall geteeprom                      ; append symbol bytes
         brne  sybolloop

         tst   higthh
         breq  nohigth                        ; higth not valid

         ldi   temp,  low (91*91)
         ldi   temp1, high(91*91)
         rcall mod91                          ; h MOD (91*91)
         ldi   temp, 91
         clr   temp1
         rcall mod91                          ; h MOD 91
         ldi   temp, 1
         rcall mod91                          ; rest
         ldi   temp, '}'                      ; higth symbol
         st    z+, temp
nohigth:

         mov  textend, izl
textloop:
         rcall geteeprom                      ; optional text field
         st    z+, temp
         brne  textloop

         rcall geteeprom                      ; no text frame number
         lds   temp1, notextcnt
         inc   temp1
         cp    temp, temp1
         brcc  notext 

         mov  textend, izl
         dec  textend
         clr  temp1
notext:
         sts  notextcnt, temp1


         ldi  izl, low(framebuf)
.IF ramend>=256
         ldi  izh, high(framebuf)
.ENDIF


         cli 
;--- crc
         clr  crclo
         clr  crchi
         ldi  ixl, low(framebuf)
.IF ramend>=256
         ldi  ixh, high(framebuf)
.ENDIF
         ldi  temp3, low(POLINOM)
         ldi  temp4, high(POLINOM)
crcbyte:
         ld   temp1, x+
         ldi  temp, 8
crcbit:
         mov  temp2, crclo
         eor  temp2, temp1
         sec
         ror  crchi
         ror  crclo
         ror  temp1
         ror  temp2
         brcs nopoli                          ; bit0 crc <> bit0 data

         eor  crclo, temp3
         eor  crchi, temp4        
nopoli: 
         dec  temp
         brne crcbit
         cp   ixl, textend
         brne crcbyte
         
         st   x+, crclo  
         st   x+, crchi
         subi textend, -2
;--- crc

         rcall geteeprom                      ; txdel
         mov  state, temp
.IFDEF pttbbit
         sbi  PORTB, pttbbit
.ENDIF
.IFDEF pttdbit
         sbi  PORTD, pttdbit
.ENDIF

         ldi  r16,  low(LOFREQ)
         ldi  r17, high(LOFREQ)
         movw freq1, r16
         ldi  r16,  low(HIFREQ)
         ldi  r17, high(HIFREQ)
         movw freq2, r16

         ldi  ixl, low(framebuf)
.IF ramend>=256
         ldi  ixh, high(framebuf)
.ENDIF

         clr  bitcnt

         ldi  temp,  0b00000000            ; no timer interrupts

.IF TIMSKX>=64
         sts  TIMSKX, temp
.ELSE
         out  TIMSKX, temp
.ENDIF
         ldi  temp,  0b11000011            ; compare a
         out  TCCR0A, temp
         ldi  temp,  0b00000001            ; fast pwm, system clock 
         out  TCCR0B, temp

sendloop:
waittimer:
         in   temp, TIFRX
         sbrs temp, TOV0
         rjmp waittimer                    ; wait till timer overflow

         ldi  temp, (1<<TOV0)
         out  TIFRX, temp                  ; clear timer overflow flag
; ---sin
         ldi  izl, low (sintab*2)
         ldi  izh, high(sintab*2)
         mov  temp, ddsh 
         lsr  temp
         sbrc ddsh, 6
         com  temp
         andi temp, 31
         add  izl, temp 
         lpm  temp, z
         sbrc ddsh, 7
         com  temp
         out  OCR0A, temp
; ---dds
         add  ddsl, freq1
         adc  ddsh, freq1h
; ---baudgen
         subi  baudcl, low (BAUDINC)
         sbci  baudch, high(BAUDINC)
         brcc  sendloop                    ; no new bit
; ---bit
         tst  bitcnt
         brne nextbit                      ; next bit of byte

; ---newbyte
         tst  state
         breq testend
         brpl decstate 

         cpi  state, -TXTAIL
         breq sendend
testend:
         cp   ixl, textend
         brne godat
         rjmp endflag
decstate:
         clr  stuffcnt
endflag:
         dec  state
godat:
         tst  state
         breq sendat

         ldi  shift, FLAG  
;         clr  stuffcnt
         rjmp sendbit
sendat:
         ld   shift, x+ 
sendbit:
         ldi  bitcnt, 8 
; ---newbyte
nextbit:
         cpi  stuffcnt, STUFFLEN  
         breq send0

         dec  bitcnt
         tst  state
         brne nostuff

         inc  stuffcnt
nostuff:
         lsr  shift
         brcs send1                       ; send nrzi 1 (do nothing)
send0:                                    ; send nrzi 0 (change freq)
         clr  stuffcnt
         movw freq, freq1
         movw freq1, freq2
         movw freq2, freq
send1:
         rjmp sendloop 
sendend:

         rjmp frameloop








;copyloop:
;         st    z+, temp2
;memcopy:
;         movw  crclo, izl
;         movw  izl, temp
;         lpm   temp2, z+
;         movw  temp, izl
;         movw  izl, crclo
;         tst   temp2
;         brne  copyloop
;         ret

;copyloop:
;         st    z+, temp
;memcopy:
;         sbi   EECR, EERE
;         in    temp, EEARL
;         inc   temp
;         out   EEARL, temp
;.IFDEF EEARH
;         brne  noeov
;         in    temp, EEARH
;         inc   temp
;         out   EEARH, temp
;noeov:
;.ENDIF
;         in    temp, EEDR
;         tst   temp
;         brne  copyloop                     ; copy till 00h
;         ret

geteeprom:
         sbi   EECR, EERE
         in    temp, EEARL
         inc   temp
         out   EEARL, temp
.IFDEF EEARH
         brne  noeov
         in    temp, EEARH
         inc   temp
         out   EEARH, temp
noeov:
.ENDIF
         in    temp, EEDR
         tst   temp
         ret






mod91:
         ldi   temp2, 32
modrep:
         inc   temp2
         sub   higth, temp
         sbc   higthh, temp1
         brcc  modrep

         add   higth, temp
         adc   higthh, temp1
         st    z+, temp2
         ret


getbcd:
         clr   temp2
         clr   temp3
bcdloop:
         rcall waitch
         subi  temp1, '0'
         brlt  skiprest

         mov   temp4, temp3
         mov   temp3, temp2
         mov   temp2, temp1
         rjmp  bcdloop


getsigned:
         clr   freq
         clr   freqh
         rcall waitch
         cpi   temp1, 0x2d                    ; "-"
         brne  nextdez

         rcall getdez
         com   freq
         com   freqh
         ret
dezloop:
         rcall mul10 
         add   freq, temp1
         brcc  getdez
         inc   freqh 
getdez:
         rcall waitch
nextdez:
         subi  temp1, '0'
         brcc  dezloop
         ret


waitdec:
         mov   freq, temp1
         rcall mul10
         rcall waitch
         subi  temp1, '0'
         add   temp1, freq                   ; x:=x*10 + chr-48
         ret

;mul10:
;         lsl   temp2                         ; *2
;         mov   temp1, temp2
;         lsl   temp1
;         lsl   temp1                         ; *4
;         add   temp2, temp1                  ; *10
;         ret

;mul10:                                       ; freq <- freq + freq1 * 10 
;         ldi   temp, 10
;loop10:
;         add   freq, freq1
;         adc   freqh, freq1h
;         dec   temp
;         brne  loop10
;         ret


mul10:                                       ; freq <- freq * 10  
;         lsl   freq                          ; *2
;         rol   freqh
;         movw  freq1, freq
;         lsl   freq                          ; *2
;         rol   freqh
;         lsl   freq                          ; *2
;         rol   freqh
;         add   freq, freq1                   ; *10
;         adc   freqh, freq1h 

         movw  freq1, freq
         lsl   freq                          ; *2
         rol   freqh
         lsl   freq                          ; *4
         rol   freqh
         add   freq, freq1                   ; *5
         adc   freqh, freq1h 
         lsl   freq                          ; *10
         rol   freqh
         ret



storech:
         rcall waitch
         add   temp1, temp2
         lsl   temp1                         ; shift address up
         st    z+, temp1   
         ret

skiprest:                                    ; skip string to next ',' or ctrl
skiploop:
         rcall waitch
         cpi   temp1, ','
         breq  skipend

         cpi   temp1, 32
         brcc  skiploop
skipend:
         ret




; ---get char from fifo
noch:
.IFDEF SMCR
         ldi  r16, 0b00000001         ; sleep enable, idle mode 
         out  SMCR, r16
.ELSE
         ldi temp, 0b00100000         ; sleep enable, idle mode
         out MCUCR, temp
.ENDIF

         sleep
.IFDEF SMCR
         ldi  r16, 0b00000000         ; sleep disable, idle mode
         out  SMCR, r16
.ELSE
         ldi temp, 0b00000000         ; sleep disable, idle mode
         out MCUCR, temp
.ENDIF
waitch:
         cp   ixl, iyl
         breq noch                            ; fifo empty go to sleep, wake on uart pinchange

         ld   temp1, y+                       ; read byte from fifo
         cpi  iyl, low(ringend)
         brne norring

         ldi  iyl, low(ringbuf)
norring:
         ret
; ---get char from fifo
;
; PB0 als Eingang GPS, dann PB2 fuer NF out und PB1 fuern PTT
; emulatorkommando
; ./tinyemu -C 13 -q 10000000 -B 57600 -s /tmp/rohr -d 2 asm/aprs.bin -b 1 -c 0 -w /tmp/pwm -e /tmp/eeprom
; (old) sox kommando
; sox -t raw -b -u -r 39062 /tmp/pwm -w -s /tmp/p.wav
;
; $GPRMC,141948.000,A,3858.4863,N,00430.6450,W,505.18,44.69,140410,0,,A*74
; $GPGGA,141949.000,3858.5860,N,00430.5187,W,1,08,1.1,11581.1,M,51.2,M,,0000*4C
; $#Pe04,100,156,158,134,130,152,152,105,3,240,96,0,10,2,98,47,0,84,101,115,116,0,3,22,255
;     s  -2   N   O   C   A   L   L  -4 UI PID `   km *  b  /    T   e   s   t   * txd END
;     ^ tx periode drive
;         ^ wide*2
;             ^ call and optional via ascii*2
;                                     ^ symbol*2+1 end of address field is odd number
;                                       ^ UI frame
;                                         ^ PID
;                                              ^ mic-e
;                                                ^ end of string
;                                                  ^ miles per h is driving 
;                                                     ^ multiplier for not drive tx periode+1
;                                                        ^  ^ symbol
;                                                             ^ end of string
;                                                                ^ optional text field
;                                                                              ^ end of string
;                                                                                ^ x times no text
;                                                                                  ^ txdel (bytes)
; 
;
.ESEG
 .DB 10,0x32*2,0x4e*2,0x4f*2,0x43*2,0x41*2,0x4c*2,0x4c*2,0x39*2+1,3,0xf0,0x60,0,6,6,">/",0,"Auto",0,4,22,255
 .DB 10,0x32*2,0x4e*2,0x4f*2,0x43*2,0x41*2,0x4c*2,0x4c*2,0x34*2+1,3,0xf0,0x60,0,5,6,"b/",0,"Rad",0,4,22,255
;     s   -2      N      O      C      A      L      L     -4     03  F0 mice  km * symb    text   n tx
; s  = seconds to send when driving
; -2 = ssid of destination = WIDEn-n
; 03 = UI Frame
; F0 = PID
; mic-e frame type 0-terminated
; km = above is driving
; *  = multiplier for seconds if below driving speed
; text field 0-terminated
; n  = times do not send text field
; tx = txdelay in byte (1 = 6.66ms 1200 baud) 
; 255 config delimiter (not allowed inside configs)                               
; $#Pe04,100,156,158,134,130,152,152,105,3,240,96,0,10,2,98,47,0,84,101,115,116,0,3,22,255
; $#Pe14,100,156,158,134,130,152,152,105,3,240,96,0,10,2,98,47,0,84,101,115,116,0,3,22,255
