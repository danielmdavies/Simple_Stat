// defines for setting and clearing register bits

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

int precision = 3;

int out = 0;
int fakeDAC = 9;
int fakeGND = 10;
char SIS[100];
char sendMe[100];
int sign = 1;
int outvolt = 1023;
int setting = 0;
int const num_reads_rise = 600; //initially
int const num_reads_flat = 25000;
int num_reads = 600;
int adc = 0;
int dac = 0;
int adcgnd = 0;
int adcref = 0;
int diff = 0;
int pdiff = 0;

int goldsign = 1;
int goldsetting = 0;
int poldsign = 1;
int poldsetting = 0;

void setup()
{
  // set prescale to 16
  
sbi(ADCSRA,ADPS2) ;
cbi(ADCSRA,ADPS1) ;
cbi(ADCSRA,ADPS0) ;

  Serial.begin(57600);
  pinMode(fakeDAC,INPUT);
  delay(1000);
  
}


void loop()
{
 readSerialString(SIS);
 sign = 1;
 
 for (int i = 0; i < 98; i++)
 {
  sendMe[i] = SIS[i+1]; 
 }
 out = stringToNumber(sendMe,4);
 
 
 if(SIS[0] == 43) //+
 {
   outvolt = out;
   write_dac(out);
 }
 if(SIS[0] == 100) //d
 {
   
  num_reads = num_reads_rise; 
   
  outvolt = out;
  dacs_on();
  write_gnd(outvolt);
 }
 
 if(SIS[0] == 45) //-
 {
  dacs_on();
 }
 
 if(SIS[0] == 103) //g
 {
  pinMode(fakeDAC, INPUT);
  
  flip_sign();

  setting = out;
  if (outvolt > 1023) outvolt = 1023;
  if (outvolt < 0) outvolt = 0;
  write_dac(outvolt);
  gstat(); 
 }
 
 if(SIS[0] == 112) //p
 {
   flip_sign();

   setting = out;
   if (outvolt > 1023) outvolt = 1023;
   if (outvolt < 0) outvolt = 0;
   write_dac(outvolt);   
   pstat();
 }
 
   sendout();
}

int dd_read(int channel)
{
 unsigned long total = 0;
 int avg = 0;
 for(int i = 0; i < num_reads; i++)
 {
   total += analogRead(channel);
 } 
 avg = total / num_reads;
 return avg; 
}

void write_dac(int val)
{
 pinMode(fakeDAC, OUTPUT);
 val = constrain(val,0,1023);
 analogWrite(fakeDAC,val/4); 
}

void write_gnd(int val)
{
 //pinMode(fakeGND,OUTPUT);
 val = constrain(val,0,1023);
 analogWrite(fakeGND,val/4); 
}

void dacs_on()
{
 pinMode(fakeGND,INPUT);
 pinMode(fakeDAC,INPUT); 
}

void flip_sign()
{
 if (out > 2000)
        {
          out = out - 2000;
          sign = -1;
        }
        else if (out < 2000)
        {
          out = out;
          sign = 1;
        }
}


//Flush String
void flushSerialString(char *strArray)
{
  int i=0;
  if (strArray[i] != 0) {
    while(strArray[i] != 0) {
      strArray[i] = 0;                  // optional: flush the content
      i++;
    }
  }
}

//Read String In
void readSerialString (char *strArray) {
  int i = 0;
  if(Serial.available() > 4) { 
    //Serial.println("    ");  //optional: for confirmation
    while (Serial.available()){
      strArray[i] = Serial.read();
      delay(5);
      i++;

    }
  }
}

int stringToNumber(char thisString[], int length)
{
  int thisChar = 0;
  int value = 0;

  for (thisChar = length-1; thisChar >=0; thisChar--) {
    char thisByte = thisString[thisChar] - 48;
    value = value + powerOfTen(thisByte, (length-1)-thisChar);
  }
  return value;
}

/*
 This method takes a number between 0 and 9,
 and multiplies it by ten raised to a second number.
 */

long powerOfTen(char digit, int power)
{
  long val = 1;
  if (power == 0) 
  {
    return digit;
  }
  else 
  {
    for (int i = power; i >=1 ; i--) 
    {
      val = 10 * val;
    }
    return digit * val;
  }
}

void pstat()
{
  adc = dd_read(0);
  adcref = dd_read(3);
  
  //Serial.println(num_reads);
  if((abs(setting-pdiff)) < precision) {
   num_reads = num_reads_flat;
  } 
  //Serial.println(num_reads);
  
  if((abs(setting-pdiff)) < precision) {
   num_reads = num_reads_flat;
  } 
  

  if((sign != poldsign) || (abs(setting-poldsetting)>5)) {
   pzirk( abs(setting) / abs(poldsetting) ); 
  }
  

  int move = 1;
  if (sign > 0)
  {
    pdiff = adc-adcref;
    if ((pdiff) > (setting))
    {

      move = gainer(pdiff,setting);
      outvolt = outvolt-move;
      write_dac(outvolt);

    }

    if ((pdiff) <(setting))
    {
      move = gainer(pdiff,setting);
      outvolt = outvolt+move;
      write_dac(outvolt);
    }
  }

  //if discharge current
  if (sign < 0)
  {
    pdiff = adcref-adc;
    if( (pdiff) > (setting))
    {
      move = gainer(pdiff,setting);
      outvolt =outvolt+move;
      write_dac(outvolt);
    }

    if ((pdiff) < (setting) )
    {
      move = gainer(pdiff,setting);
      outvolt = outvolt-move;
      write_dac(outvolt);

    }
  }
  poldsign = sign;
  poldsetting = setting;
}



void gstat()
{
  adc = dd_read(0);
  dac = dd_read(1);

  //Serial.println(num_reads);
  if((abs(setting-diff)) < precision) {
   num_reads = num_reads_flat;
  } 

  if((sign != goldsign) || (abs(setting-goldsetting)>5)) {
   gzirk( abs(setting) / abs(goldsetting) ); 
  }

  int move = 1;
   
  if (sign > 0)
  {
    diff = dac - adc;
    //if over current step dac down
    if ((diff) > (setting))
    {

      move = gainer(diff,setting);
      outvolt = outvolt-move;
      write_dac(outvolt);

    }

    //if under current step dac up
    if ((diff) <(setting))
    {
      move = gainer(diff,setting);
      outvolt = outvolt+move;
      write_dac(outvolt);
    }
  }

  //if discharge current
  if (sign < 0)
  {
    diff = adc - dac;
    //if over current step dac up
    if( (diff) > (setting))
    {
      move = gainer(diff,setting);
      outvolt =outvolt+move;
      write_dac(outvolt);
    }

    //if under current step dac down
    if ((diff) < (setting) )
    {
      move = gainer(diff,setting);
      outvolt = outvolt-move;
      write_dac(outvolt);

    }
  }
  goldsign = sign;
  goldsetting = setting;
}



void sendout()
{
 adc = dd_read(0);
  dac = dd_read(1);
  adcgnd = dd_read(2);
  adcref = dd_read(3);

  int refvolt = analogRead(5);
  int setout = sign*setting;
  Serial.print("GO.");
  //trying to figure out what is being called
  //if(dan_last == 1) Serial.print("dan_pot ");
  //else if(dan_last == 0) Serial.print("potentiostat");
  //Serial.print(",");
  Serial.print(constrain(outvolt,0,1023),DEC);
  Serial.print(",");
  Serial.print(adc);
  Serial.print(",");
  Serial.print(dac);
  Serial.print(",");
  Serial.print(0);
  Serial.print(",");
  Serial.print(setout);
  Serial.print(",");
  Serial.print(0);
  Serial.print(",");
  Serial.print(SIS[0]);
  Serial.print(SIS[1]);
  Serial.print(SIS[2]);
  Serial.print(SIS[3]);
  Serial.print(SIS[4]);
  Serial.print(",");
  Serial.print(adcgnd);
  Serial.print(",");
  Serial.print(adcref);
  Serial.print(",");
  unsigned long time = millis();
  Serial.print(time);
  Serial.println(",ST");
    //probably not necessaray but oh well.
}
int gainer(int wii, int wisb)
{
 int mover = abs(wii-wisb);
 return mover;
}

void gzirk(float ratio) 
{
 Serial.println("Entered gzirk");
 if((goldsign > 0) && (sign < 0) && (abs(goldsetting) == abs(setting))) {
  //write_gnd(1023);
  write_dac(adcgnd-abs(diff));
 } 
 else if((goldsign < 0) && (sign > 0) && (abs(goldsetting) == abs(setting))) {
  //write_gnd(0); 
  write_dac(adcgnd+abs(diff));
 }
 else if ((goldsetting != setting) && (goldsign == sign)) {
  write_dac(adcgnd+(int)(diff*ratio));
 }
 else if ((goldsetting != setting) && (goldsign > 0) && (sign < 0)) {
  //write_gnd(1023);
  write_dac(adcgnd-(int)(abs(diff)*ratio));
 }
 else if ((goldsetting != setting) && (goldsign < 0) && (sign > 0)) {
  //write_gnd(0);
  write_dac(adcgnd+(int)(abs(diff)*ratio));
 }
 num_reads = num_reads_rise;
}

void pzirk(float ratio) 
{
 Serial.println("Entered pzirk");
 if((poldsign > 0) && (sign < 0) && (abs(poldsetting) == abs(setting))) {
  //write_gnd(1023);
  write_dac(adcgnd-abs(pdiff));
 } 
 else if((poldsign < 0) && (sign > 0) && (abs(poldsetting) == abs(setting))) {
  //write_gnd(0); 
  write_dac(adcgnd+abs(pdiff));
 }
 else if ((poldsetting != setting) && (poldsign == sign)) {
  write_dac(adcgnd+(int)(pdiff*ratio));
 }
 else if ((poldsetting != setting) && (poldsign > 0) && (sign < 0)) {
  //write_gnd(1023);
  write_dac(adcgnd-(int)(abs(pdiff)*ratio));
 }
 else if ((poldsetting != setting) && (poldsign < 0) && (sign > 0)) {
  //write_gnd(0);
  write_dac(adcgnd+(int)(abs(pdiff)*ratio));
 }
 num_reads = num_reads_rise;
}
