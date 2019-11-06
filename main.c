#include <lcd.h>
#define MCU_CLOCK           1000000
#define PWM_FREQUENCY       50
#define SERVO_STEPS         180
#define SERVO_MIN           700
#define SERVO_MAX           3000
unsigned int PWM_Period     = (MCU_CLOCK / PWM_FREQUENCY);
unsigned int PWM_Duty       = 0;
int angle=90;
unsigned int servo_stepval, servo_stepnow;
   unsigned int servo_lut[ SERVO_STEPS+1 ];
   unsigned int i;
   int a = 0;
   int miliseconds;
   int distance;
   long sensor;
   void  dis(void);
void automa(void);
void manual(void);
void main (void)
{
    BCSCTL1 = CALBC1_1MHZ;
      DCOCTL = CALDCO_1MHZ;
      WDTCTL = WDTPW + WDTHOLD;
      TA1CCTL0 = CCIE;
      TA1CCR0 = 1000;
      TA1CTL = TASSEL_2 + MC_1;
    servo_stepval   = ( (SERVO_MAX - SERVO_MIN) / SERVO_STEPS );
    servo_stepnow   = SERVO_MIN;
    for (i = 0; i < SERVO_STEPS; i++)
    {
        servo_stepnow += servo_stepval;
        servo_lut[i] = servo_stepnow;
    }
// هدول معروفين
    // Setup the PWM, etc.
    WDTCTL  = WDTPW + WDTHOLD;
    TACCTL1 = OUTMOD_7;
    TACTL   = TASSEL_2 + MC_1;
    TACCR0  = PWM_Period-1;
    TACCR1  = PWM_Duty;
    P1DIR   |= BIT6;
    P2DIR &= ~(BIT2|BIT1);
    P1SEL   |= BIT6;
    P2DIR &= ~BIT0;
    P2IE |= (BIT1|BIT2);
    P2IFG &= ~(BIT1|BIT2);
    lcd_init();
    __enable_interrupt(); //مشان يشتغلو الانتربت

while (1) //الحلقة اللانهائية
 { //بدنا من الحلقة أن تبقى عبتفحص قيمة ال و على أساسه يتحدد مود العمل ، راجعي الخوارزمية
    if (a==1) //ااذا كان قيمتو 1
    {
        manual();//استدعي التابع تبع اليدوي
    }
    if (a==2) //اذا كان قيمتو 2
    { //كود الاوتو
        for (angle = 0; angle < 180; angle+=30)
        { //دوران باتجاه عقارب الساعة
                    TACCR1 = servo_lut[angle]; //تحديد زاوية الدوران
                    dis(); //حساب المسافة
                    __delay_cycles(1000000); //تأخير حتى الشاشة تلحق تعرض
                    if (a==1) //افحص اذا انكبس مانول
                        break; //طلاع من هالحلقة

        } //هون نففس اللي قبلا بس باتجاه تاني
        for (angle = 180 ; angle > 0; angle-=30)
        {
            TACCR1 = servo_lut[angle];
                               dis();
                               __delay_cycles(1000000);
                               if (a==1)
                               break;
        }
    }
 }
}
 void manual(void)
{//اليدوي

    if ((P2IN&BIT0) == BIT0) //قيمة الدخل واحد بالبن P2.0
    {
        angle+=5; //لليمين در
        if (angle > 180)
            angle = 180;
        TACCR1 = servo_lut[angle];
        dis();
        __delay_cycles(200000);
    }
    else
    { //لليسار در
        angle-=5;
                   if (angle < 0)
                       angle = 0;
                   TACCR1 = servo_lut[angle];
                   dis();
                   __delay_cycles(200000);
    }
    P2IFG &= ~(BIT1|BIT2); //نزل الفلاج مشان ما يعلق بالانتربت
}
void dis(void) //حساب المسافة
{ // بتعرفيهم تبعات أن تغريف دخل و خرج و هيك
    P1DIR |= BIT4;
     P1OUT |= BIT4;
     __delay_cycles(10);
     P1OUT &= ~BIT4;
     P1DIR &= ~BIT5;
         P1IFG = 0x00;
     P1IE |= BIT5;
     P1IES &= ~BIT5;    //لو اجتو اشارة لهالبن رح يصير انتربت
         __delay_cycles(30000);
         distance = sensor/58;
         LCD_CLR();
        send_int(distance);
        __delay_cycles(20000);
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{ //انتربت البورت 2
    if ((P2IFG & BIT1)==BIT1) //من البت 1 ؟
     a = 1;
        //   manual();
    if ((P2IFG & BIT2)==BIT2)
         a = 2;
        //automa();
    P2IFG &= ~(BIT1|BIT2);
}


#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    if(P1IFG&BIT5) //مرفوع الفلاج ؟
        {
          if(!(P1IES&BIT5)) //جبهة صاعدة (بلش حساب الوقت اللي عبيجي من الايكو)
          {
            TA1CTL|=TACLR;   //صفر التايمر
            miliseconds = 0;
            P1IES |= BIT5;  //بس تشوف جبهة هابطة بصير اعمل انتربت
          }
          else //لما يخلص النبضة الجاي من الايكو رح يجي حبهة هابطة و يدخل لهون
          {
            sensor = (long)miliseconds*1000 + (long)TA1R; //احسب قيمة المسافة حسب القيمة اللي تسجلت من نبضة الايكو
          }
    P1IFG &= ~BIT5;
    }
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer_A (void)
{
  miliseconds++;//زيادة قيمة متحول عبعد النبضة تبع الإيكو
}


