#define BANK_SIZE 2048

unsigned char avg(unsigned char *buffer, int n) {
  int result = 0;
  int k = n;
  while (k-- > 0) {
    result += *(buffer+k);
  }
  return (unsigned char)(result/n);
}

//PERFORMS THE CROSSCORRELATION BETWEEN TWO DATA ARRAYS
//IT ALSO DOES TIME-DELAY ANALYSIS FOR POSITIVE CORRELATED SIGNALS, SAVING THE MAXIMUM VALUE AND OFFSET
//THIS REALLY SHOULD BE OPTIMIZED,FOR EXAMPLE, VIA FFT CROSSCORRELATION METHODS

double sp_table[BANK_SIZE*2];
double sp_max;
int sp_maxindex;
  
int crossCorrelation(unsigned char *data1, unsigned char *data2) {
  
  int j,i;
  sp_max = -99999999999;
  sp_maxindex = -1;
  
  double average1 = 0;
  double average2 = 0;
  for (j = 0; j < BANK_SIZE; j++) {
    average1 += data1[j];
  }
  for (j = 0; j < BANK_SIZE; j++) {
    average2 += data2[j];
  }
  average1 /= 1.0*BANK_SIZE;
  average2 /= 1.0*BANK_SIZE;
  int c=0;
  for (j = -BANK_SIZE; j < BANK_SIZE;j++) {
    for (i = 0; i < BANK_SIZE; i++) {
      if ((i+j < BANK_SIZE) && (i+j >= 0)) {
        sp_table[c] += ((data1[i]-average1) * (data2[i+j]-average2));
      }
    }
    if (sp_table[c] > sp_max) {
      sp_max = sp_table[c];
      sp_maxindex = c-BANK_SIZE;
    }
    c++;
  }
  return sp_maxindex;
}

int crossCorrelation2(unsigned char *x, unsigned char *y, int maxdelay) {
   int i,j,delay,delayAtMaxCorrelation;
   int n = BANK_SIZE;
   double mx,my,sx,sy,sxy,denom,r,maxCorrelation;
   
   maxCorrelation = 0;
   delayAtMaxCorrelation = 0;
   
   /* Calculate the mean of the two series x[], y[] */
   mx = 0;
   my = 0;   
   for (i=0;i<n;i++) {
      mx += x[i];
      my += y[i];
   }
   mx /= n;
   my /= n;

   /* Calculate the denominator */
   sx = 0;
   sy = 0;
   for (i=0;i<n;i++) {
      sx += (x[i] - mx) * (x[i] - mx);
      sy += (y[i] - my) * (y[i] - my);
   }
   denom = sqrt(sx*sy);

   /* Calculate the correlation series */
   for (delay=-maxdelay;delay<maxdelay;delay++) {
      sxy = 0;
      for (i=0;i<n;i++) {
         j = i + delay;
         if (j < 0 || j >= n)
            continue;
         else
            sxy += (x[i] - mx) * (y[j] - my);
      }
      r = sxy / denom;
      if (r > maxCorrelation) {
        maxCorrelation = r;
        delayAtMaxCorrelation = delay;
    }
      /* r is the correlation coefficient at "delay" */
   }
  return delayAtMaxCorrelation;
}

int crossCorrelation3(unsigned char *x, unsigned char *y, int maxdelay) {
   int i,j,delay,delayAtMaxCorrelation;
   int n = BANK_SIZE;
   double sxy,r,maxCorrelation;
   
   maxCorrelation = 0;
   delayAtMaxCorrelation = 0;

   /* Calculate the correlation series */
   for (delay=-maxdelay;delay<maxdelay;delay++) {
      sxy = 0;
      for (i=0;i<n;i++) {
         j = i + delay;
         if (j < 0 || j >= n)
            continue;
         else
            sxy += (x[i] * y[j]);
      }
      r = sxy;
      if (r > maxCorrelation) {
        /* r is the correlation coefficient at "delay" */
         maxCorrelation = r;
         delayAtMaxCorrelation = delay;
    }
   }
  return delayAtMaxCorrelation;
}


unsigned char test_signal_1[BANK_SIZE];
unsigned char test_signal_2[BANK_SIZE];
unsigned long startt,stopt;
int result;

void setup() {
  Serial.begin(115200);
  Serial.println("Init...");
  //skew by 499 samples
  test_signal_1[0] = 0xff; 
  test_signal_2[499] = 0xff; 
}

void loop() {
  Serial.print("Test crossCorrelation: ");
  startt = millis();
  result = crossCorrelation(test_signal_1,test_signal_2);
  stopt = millis();  
  Serial.print("offset ");
  Serial.print(result);
  Serial.print(", took ");
  Serial.print(stopt-startt);
  Serial.println("ms");
  
  Serial.print("Test crossCorrelation2: ");
  startt = millis();
  result = crossCorrelation2(test_signal_1,test_signal_2,1000);
  stopt = millis();  
  Serial.print("offset ");
  Serial.print(result);
  Serial.print(", took ");
  Serial.print(stopt-startt);
  Serial.println("ms");

  Serial.print("Test crossCorrelation3: ");
  startt = millis();
  result = crossCorrelation3(test_signal_1,test_signal_2,1000);
  stopt = millis();  
  Serial.print("offset ");
  Serial.print(result);
  Serial.print(", took ");
  Serial.print(stopt-startt);
  Serial.println("ms");

}

/*
On ESP32 running at 160Mhz, output is:

Test crossCorrelation: offset 499, took 9111ms
Test crossCorrelation2: offset 499, took 6583ms
Test crossCorrelation3: offset 499, took 1706ms
*/
