#include <Arduino.h>
#include <PS2X_lib.h>

PS2X ps2;
int error =0;

int mtr[][3]={
  {
    13,12,11                  }
  ,
  {
    7,8,5                  }
  ,
  {
    4,2,3                  }
  ,
};


float cnst[][3]={
  {
    0.58,-0.33,0.33            }
  ,
  {
    -0.58,-0.33,0.33            }
  ,
  {
    0,0.67, 0.33            }
};

int CLOCK = A2;
int COMMAND = A1;
int ATTENTION = A3;
int DATA = A0;

int UP, DOWN, LEFT, RIGHT, SQUARE, CIRCLE, TRIANGLE, START, SELECT, L1, L2, L3, R1, R2, R3, LX, LY, RX, RY, CROSS;

float pwm[3], comp[3],set_factor=4.0, mul_factor; //pwm[1-3] for PWM of motors 1-3 //similarly comp[1-3] for three velociy components(Vx, Vy, W)
void pwm_calc()
{
  mul_factor = set_factor;
  int i,j;
  repeat: i = 3;
  while(i--)
  {
    pwm[i]=0;
    j=3;
    while(j--)
    {
      pwm[i]+=cnst[i][j]*comp[j];
    }
   pwm[i]*=mul_factor;
  }
 i=3;while (i--) {
   if(pwm[i]>255|| pwm[i]<-255)
   {
     Serial.print("PWM-");Serial.print(i);Serial.println("is exceeding The Maximum Limit");
     Serial.println("Decreasing the Multiplication Factor by 0.4");
     mul_factor-=0.4; goto repeat;
   }
 }
}
void read_buttons(){
  UP = ps2.ButtonPressed(PSB_PAD_UP);
  DOWN= ps2.ButtonPressed(PSB_PAD_DOWN);
  LEFT = ps2.Button(PSB_PAD_LEFT);
  RIGHT = ps2.Button(PSB_PAD_RIGHT);

  SQUARE = ps2.Button(PSB_SQUARE);
  CIRCLE = ps2.Button(PSB_CIRCLE);
  CROSS = ps2.Button(PSB_CROSS);
  TRIANGLE= ps2.Button(PSB_TRIANGLE);

  START = ps2.ButtonPressed(PSB_START);
  SELECT = ps2.ButtonPressed(PSB_SELECT);

  L1 = ps2.ButtonPressed(PSB_L1);
  L2 = ps2.Button(PSB_L2);
  L3 = ps2.ButtonPressed(PSB_L3);
  R1 = ps2.ButtonPressed(PSB_R1);
  R2 = ps2.Button(PSB_R2);
  R3 = ps2.ButtonPressed(PSB_R3);

  LX = (ps2.Analog(PSS_LX)-128)*-1;
  LY = ps2.Analog(PSS_LY)-128;
  RX = (ps2.Analog(PSS_RX)-128)*-1;
  RY = ps2.Analog(PSS_RY)-128;
}

int i;
int spd=50 ,prev=0;

void motor( int mtr_nm, int pwm, int change)
{
  int i;
  if(pwm==0)
  {
    analogWrite(mtr[mtr_nm-1][2], 50);
    i=2;
    while(i--)
      digitalWrite(mtr[mtr_nm-1][i], HIGH);
    return;
  }
  if(change==1){
    if(pwm>0)
    {
      analogWrite(mtr[mtr_nm-1][2], pwm);
      i=2;
      while(i--)
        digitalWrite(mtr[mtr_nm-1][i], i^0);
    }


    else if(pwm<0)
    {
      pwm*=-1;
      analogWrite(mtr[mtr_nm-1][2], pwm);
      i=2;
      while(i--)
        digitalWrite(mtr[mtr_nm-1][i], i^1);
    }
  }
  else if(change==0)
  {
    if(pwm<0)pwm*=-1;
    analogWrite(mtr[mtr_nm-1][2], pwm);
  }
}

void setup()
{
  Serial.begin(9600);
  error = ps2.config_gamepad(CLOCK, COMMAND, ATTENTION, DATA, false, false);
  while(error!=0)
  {
    Serial.println("Attempting to Reconnect...");
    delay(200);
  }
  if(error==0)
    Serial.println("Controller Found");
  Serial.println("Started...");
  DDRB |= B11111100;
  DDRB |= B111111;
  i=3;
  while(i--)
    analogWrite(mtr[i][2], spd);
    comp[2]=0; //W(Omega) remains Zero Unless Specefied
}

int prev_lx, prev_ly;
void loop()
{
  ps2.read_gamepad();
  read_buttons();


  if(LX==0 && LY==0 && TRIANGLE==0 && R2==0 && L2==0 && prev!=0)
  {
    motor(1, 0, 1);
    motor(2, 0, 1);
    motor(3, 0, 1);
    prev=0;
    Serial.println("Stopping Motors");
  }
  else {
    if((LX!=0 || LY!=0) && (prev_lx!=LX || prev_ly!=LY) )
    {
      i=3;
      comp[0]=LY;comp[1]=LX;
      pwm_calc();
      prev_lx=LX;
      prev_ly=LY;
      while(i--)
      {
        motor(i+1, (int)pwm[i], 1);
      }
    prev=4;
    }
    else if(TRIANGLE==1 && prev!=1)
    {

      motor(1, spd*-1, 1);
      motor(2, spd, 1);
      motor(3, 0, 1);
      Serial.println("Moving Bot Forward...");
      prev=1;
    }

    else if(L2==1 && prev!=2)
    {
      i=3;
      while(i--)
        motor(i+1, spd, 1);
      Serial.println("Rotating Counter-Clockwise...");
      prev=2;
    }

    else if(R2==1 && prev!=3)
    {
      i=3;
      while(i--)
        motor(i+1, spd*-1, 1);
      Serial.println("Rotating Clockwise...");
      prev=3;
    }
    }
    if(R1==1)
    {
      spd+=10;
      if(spd>255)
        spd=255;
      Serial.print("Increasing Speed to ");
      Serial.println(spd);
      i=3;
      while(i--)
        if(LX==0 && LY==0) motor(i, spd, 0);


    }
    else if(L1==1)
    {
      spd-=10;
      if(spd<0)
        spd=0;
      Serial.print("Decreasing Speed to ");
      Serial.println(spd);
      i=3;
      while(i--)
        if(LX==0 && LY==0) motor(i, spd, 0);
    }
  else if(UP==1)
  {
   set_factor+=0.4;
   Serial.print("Increasing the Set Factor to");
   Serial.println(set_factor);
 }
 else if(DOWN==1)
 {
  set_factor-=0.4;
  Serial.print("Decreasing the Set Factor to");
  Serial.println(set_factor);
}
}
