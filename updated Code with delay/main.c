/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 * 
 *                       MSP430 CODE EXAMPLE DISCLAIMER
 *
 * MSP430 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
 * for an API functional library-approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/
//*******************************************************************************
//	FZI - Code for internal Tempereature sensor of MSP430x22x4 (MSP430F2274)
//******************************************************************************
//  MSP430F22x4 Demo - ADC10, Sample A10 Temp and Convert to oC and oF
//
//  Description: A single sample is made on A10 with reference to internal
//  1.5V Vref. Software sets ADC10SC to start sample and conversion - ADC10SC
//  automatically cleared at EOC. ADC10 internal oscillator/4 times sample
//  (64x) and conversion. In Mainloop MSP430 waits in LPM0 to save power until
//  ADC10 conversion complete, ADC10_ISR will force exit from LPM0 in
//  Mainloop on reti. Temperature in oC stored in IntDegC, oF in IntDegF.
//  Uncalibrated temperature measured from device to device will vary with
//  slope and offset - please see datasheet.
//  ACLK = n/a, MCLK = SMCLK = default DCO ~1.2MHz, ADC10CLK = ADC10OSC/4
//
//                MSP430F22x4
//             -----------------
//            |              XIN|-
//            |                 |
//            |RST          XOUT|-
//            |                 |
//            |A10              |
//
//  
//			IAR Embedded Workbench
//******************************************************************************

#include <msp430.h>
// #include <stdio.h>

long Temp;
int i;
volatile long IntDegree_F;
volatile long IntDegree_C;

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT (watch dog timer)
  ADC10CTL1 = INCH_10 + ADC10DIV_3;         // Temp Sensor ADC10CLK/4 , ADC control register 1, ADDRESS OFFSET -1B2h
					    // ADC10DIVx - ADC10 clock divider (Here clock divided by 4)
					    // INCH_10 - Input channel select (1010) for Temperature sensor

  ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE; // ADC control register 0, ADDRESS OFFSET - 1B0h   // SREFx bits define the voltage reference
  TACCR0 = 30;                              // Delay to allow Ref to settle
  TACCTL0 |= CCIE;                          // Compare-mode interrupt
  TACTL = TASSEL_2 + MC_1;                  // TACLK = SMCLK, Up mode
  __bis_SR_register(CPUOFF + GIE);          // LPM0, TA0_ISR will force exit
  TACCTL0 &= ~CCIE;                         // Disable timer Interrupt

  
  while(1)
  {
    
     __delay_cycles(10000000);               // 1Mhz is a system clock, so for 10 sec. of delay -> 10 * (1/1Mhz)
      ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
    __bis_SR_register(CPUOFF + GIE);        // LPM0 with interrupts enabled

    // Temperature in F = ((A10/1024)*1500mV)-923mV)*1/1.97mV = A10*761/1024 - 468
    Temp = ADC10MEM;
    IntDegree_F = ((Temp - 630) * 761) / 1024;

    // Temperature in C = ((A10/1024)*1500mV)-986mV)*1/3.55mV = A10*423/1024 - 278
    Temp = ADC10MEM;
    IntDegree_C = ((Temp - 673) * 423) / 1024;
    
                        // Not working with Terminal I/O, printf(�Integer value is %d\n� , IntDegree_C); scanf(�%d�, &IntDegree_C);
     
    __no_operation();                       // SET BREAKPOINT HERE
  }
}

// ADC10 interrupt service routine

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC10_VECTOR))) ADC10_ISR (void)
#else
#error Compiler not supported!
#endif
{
  __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMERA0_VECTOR
__interrupt void TA0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMERA0_VECTOR))) TA0_ISR (void)
#else
#error Compiler not supported!
#endif
{
    TACTL = 0;                                // Clear Timer_A control registers
  __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}

