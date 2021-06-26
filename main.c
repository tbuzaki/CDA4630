//Thomas Buzaki
//Midterm Code
//6/26/21

//NOTE TO GRADER: I have made some changes to the code.
//The lines for the edited code are: 29, 97-100, and 111-116.
//The changes occuring on line 29 are: a new variable
//created to track the current state of the motor, forward or backward.
//A simple int variable is used.
//The changes occuring lines 97-100 are: the temperature range was
//changed to allow for the light to turn on once it reaches 1.04
//and turn back off at 1.01.
//The changes occuring lines 111-116 are: the values used to determine
//touch are modified to increase sensitivity. An if/else structure
//is also used in conjunction with the variable created on line 29
//to alternate motor rotation.



#include <msp430.h>

int i=0 ;
int light = 0, lightroom = 0, dimled=50;
int temp = 0, temproom = 0;
int touch =0, touchroom =0;
int flag =0;
int ADCReading [3];
int tempthld=0; // threshold value for the temp sensor
int direction =0; // Determines the current direction of motor. 0 for Rev, 1 for Fwd.

// Function Prototypes
void fadeLED(int valuePWM);
void ConfigureAdc(void);
void getanalogvalues();


int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;               // Stop WDT
    P1OUT = 0;              // set default values to all zeros for port 1
    P1DIR = 0;              // set port direction to all inputs for port 1
    P2OUT = 0;              // set default values to all zeros for port 2
    P2DIR = 0;              // set port direction to all inputs for port 2

    P1REN = 0;              // disable pull up/down resistors for port 1
    P2REN = 0;              // disable pull up/down resistors for port 2

    P1DIR |= ( BIT4 | BIT5 | BIT6);     // set port 1 bits 4, 5, 6 as outputs
    P2DIR |= ( BIT0 | BIT1       );     // set port 2 bits 0, 1    as outputs

    // start the motor in forward direction and wait 2 seconds so the voltages on the breadboard settle before you start reading the analog values
    // also turn the temp LED off before our start the loop
    P1OUT &=~BIT4; P1OUT |= BIT5; P1OUT |= BIT6;__delay_cycles(2000000);
    P2OUT &=~BIT0;                              __delay_cycles(2000000);

    ConfigureAdc();

    // reading the initial room values, lightroom, touchroom, temproom
    __delay_cycles(250);
    getanalogvalues();
    lightroom = light; touchroom = touch; temproom = temp;
    __delay_cycles(250);

for (;;)
{
    //reading light, touch, and temp repeatedly at the beginning of the main loop
    getanalogvalues();

    //light controlling LED2 on launch pad (P1.6) via variable dimled
    dimled = light;

    //use the light reading range min/max limits 50-1000, and convert them to 0-100%
    dimled = ((dimled- 700)*100)/(1000- 700);
    if(dimled <= 20)dimled = 0; else if (dimled >=80)dimled = 100;
    fadeLED(dimled);

    //------------- Light Controlling a LED on port 2.1, which is on the demo board
    //These are extra lines for testing the light sensor and the concept of "dead zone"
    //Observe the dead zone for no action to avoid LED flickering
    //I chose the range 1.1 to 1.5 of the value;
    //that is no action if  (1.1* lightroom < light < 1.5 * lightroom)
    if(light < lightroom * 1.50 && light > lightroom * 1.10) {}
    else
    { if(light >= lightroom * 1.50) {P2OUT |=  BIT1; __delay_cycles(200);}    // on if dark
      if(light <= lightroom * 1.10) {P2OUT &= ~BIT1; __delay_cycles(200);}    // off if light
    }

    //------------- Temp Controlling a LED on port 2.0
    //The code below uses a simple comparison decision, which creates flickering
    //if temp is greater than 90 F, turn led on. 95 F degrees = .95V * 1024/3.3 V
    //tempthld = (.95 * 1024) / 2.9; // set the temp threshold as 90 F, and assuming VCC is 2.9 when motor is active
    //if(temp > tempthld) {P2OUT |=  BIT0; __delay_cycles(200);}    // LED on
    //else                {P2OUT &= ~BIT0; __delay_cycles(200);}    // LED off

    //You can also use the following code to turn tmep LED on if the temp is 3% higher than the baseline temproom
    //You may have to change the 1.03 value to 1.02 or 1.04 as temp sensors are all different and not all people have same body temp
    if(temp > temproom *1.04 ) {P2OUT |=  BIT0; __delay_cycles(200);}    // LED on
    else
    { if(temp < temproom *1.01 ) {P2OUT &= ~BIT0; __delay_cycles(200);}
    }// LED off

    //Required changes:
    //Modify the code above to create a dead zone to prevent flickering
    //if temp is higher than 1.04 * temproom, turn LED on
    //if temp is lower  than 1.02 * temproom, turn LED off
    //also you may have to tweak the boundary values of the dead zone to suit your particular surrounding.


    //------------- Touch Controlling Motor Direction
    //observe the dead zone for no action between 0.07-0.09 of the value touch
    if(touch < touchroom * 0.9 && touch > touchroom * 0.5)
    {   if(direction == 0){P1OUT &=~BIT4; P1OUT |= BIT5; __delay_cycles(200);
        direction = 1;}
        else{P1OUT |= BIT4; P1OUT &=~BIT5; __delay_cycles(200);
        direction = 0;}
    }
    //Required changes:
    //The 2 lines above make a simple turn-on while still touching, off when not touching
    //Modify the code so that with each touch, the direction toggles and stays

} //end for
} //end main

void ConfigureAdc(void)
{
   ADC10CTL1 = INCH_2 | CONSEQ_1;                // A2 + A1 + A0, single sequence
   ADC10CTL0 = ADC10SHT_2 | MSC | ADC10ON;
   while (ADC10CTL1 & BUSY);
   ADC10DTC1 = 0x03;                             // 3 conversions
   ADC10AE0 |= (BIT0 | BIT1 | BIT2);             // ADC10 option select
}

void fadeLED(int valuePWM)
{
    P1SEL |= (BIT6);                              // P1.0 and P1.6 TA1/2 options
    CCR0 = 100 - 0;                               // PWM Period
    CCTL1 = OUTMOD_3;                             // CCR1 reset/set
    CCR1 = valuePWM;                              // CCR1 PWM duty cycle
    TACTL = TASSEL_2 + MC_1; // SMCLK, up mode
}

void getanalogvalues()
{
 i = 0; temp = 0; light = 0; touch =0;            // set all analog values to zero
    for(i=1; i<=5 ; i++)                          // read all three analog values 5 times each and average
  {
    ADC10CTL0 &= ~ENC;
    while (ADC10CTL1 & BUSY);                     //Wait while ADC is busy
    ADC10SA = (unsigned)&ADCReading[0];           //RAM Address of ADC Data, must be reset every conversion
    ADC10CTL0 |= (ENC | ADC10SC);                 //Start ADC Conversion
    while (ADC10CTL1 & BUSY);                     //Wait while ADC is busy
    light += ADCReading[0];                       // sum  all 5 reading for the three variables
    touch += ADCReading[1];
    temp += ADCReading[2];
  }
 light = light/5; touch = touch/5; temp = temp/5; // Average the 5 reading for the three variables
}


#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
    __bic_SR_register_on_exit(CPUOFF);
}
