/*******************************************************************************************
 *				CSERIAL MULTIPLEXING
 *	AUTHOR	|Brian Onang'o
 *	COMPANY1|Upande Ltd (http://www.upande.co.ke)
 *	COMPANY2|Circuits and Systems Engineering Company (CSECO)
 *	DATE	|December 2016
 *_________________________________________________________________________________________
 *										Description
 * Software for performing modulation in audio frequency range enabling the testing of RF 
 * modulation concepts using audio frequency spectral analysis software.
 * The specific modulation scheme implemented here is cserial multiplexing. More details
 * about this modulation scheme can be found here -> ___________
 * 
 * For more information please see ReadMe.txt available with this file and the documentation 
 * in the program comments.
 *__________________________________________________________________________________________
 *										Dependancies
 *
 *__________________________________________________________________________________________
 *										LICENSE
 *******************************************************************************************/
//./cserialmux -f TrialFile.txt -b 1 -m  PASS -p 12.5 -l 1 -r 1 -k 0.5 |sox -t raw -r 44100 -s -L -b 16 -c2 - "outputp.wav" &&  sox outputp.wav outputp.dat &&  plot "outputp.dat" 

 /* Environment. Uncomment if compiling in linux */
//#define CSCOMPILER TCC

 /*
  * 			DEPENDANCIES
  */
#include <stdio.h>      /* printf fprintf sprintf fopen fputs */
#include <stdbool.h>    /* bool type */
#include <math.h>   	/* M_PI */
#include <stdbool.h>    /* bool type */
#include <stdlib.h>		/* exit codes */
#include <stdint.h>     /* C99 types */
#include <string.h>     /* C99 types */

#ifdef CSCOMPILER
	#include <cs_optarg.h>
#else 
	#include <unistd.h>     /* getopt access usleep */
#endif


/*
 *	Sampling rate and buffer size
 */
//#define S_RATE  44100
const int DEFAULT_S_RATE = 44100;
#define B_SIZE  (10481048)
#define B_OUT_SIZE  (4096)

/*
 *	Length of file name
 */
#define FNAMELEN 256

#define DEFAULTAMPLITUDE 1


typedef int int1;


/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */
//#define max(a,b) (a>b?a:b)
/*
 *	carriers to be modulated (look like this)
 */	
struct carrier_struct
{
	float frequency;
	float phase;
	float amplitude;
};
typedef struct carrier_struct carrier;

struct data_struct
{
	int a;
};

typedef unsigned char byte;
//typedef uint16_t encoding;
typedef float encoding;
typedef double tym;

float cosFunc(float pos,float theta) { return cos(pos*2*M_PI+theta); }
float sinFunc(float pos,float theta) { return sin(pos*2*M_PI+theta); }
float squareFunc(float pos,float theta) { return cos(pos*2*M_PI+theta)<=0?-1:1; }
float triangleFunc(float pos,float theta) { return 1-fabs(fmod(pos+theta,1.0)-0.5)*4; }

byte modulationTypeIndex(char* modType);
byte bitsperSymbol(byte modType);
encoding basebandmodulation(byte modType, tym time, char* bits);
encoding passbandmodulation(byte modType, encoding basebandval, tym time);
void csExit(byte errorCode, char* msg);
void usage (void);
void MSGf (char* MSGf);
void readData(char *fname);
byte getBit(int bytepos, int beetpos);
encoding BPSK(tym t, char* bits);
encoding BFSK(tym t, char* bits);
encoding BASK(tym t, char* bits);
encoding QAM4(tym t, char* bits);
encoding basebandCarrier(tym time);
encoding passbandCarrier(tym time);

carrier basebandcarrier;
carrier passbandcarrier;
char data[B_SIZE];
encoding buffer[B_OUT_SIZE];
uint32_t datalen;

float modConstantkf, modIndex;

int main(int argc, char **argv) 
{
	int i;
	uint16_t j, k, i2;
	tym time;
	encoding basebandvalue, passbandvalue;			//confirm data type
	byte modType, bitspersym;
	int S_RATE = DEFAULT_S_RATE;
	
	char arg_s[64];
	char FileName[FNAMELEN];
	float basebandfrequency, passbandfrequency, Tb, Tsym, Td, Symfreq, SymRate; //symbol rate is relative to frequency. How many symbols per period?
	char modTypes[8];
	int Sympos, Symposprev, BitPos;
	char bitsinSymbol[16];
	
	
	///* parse command line options 
    while ((i = getopt (argc, argv, "f:b:m:p:l:r:k:s:h")) != -1) {
	switch (i) {
		case 'h':
			usage();
			return EXIT_FAILURE;
			break;
				
		case 'f': 
			strcpy(FileName, optarg);
			break;
				
		case 'b': 
			basebandfrequency = /*2 * M_PI **/ atof(optarg);
			break;
				
		case 'm': 
			strcpy(modTypes, optarg);
       	break;

		case 'p': 
			passbandfrequency = /*2 * M_PI **/ atof(optarg) /** 1000*/;
			break;
				
		case 'l': 
			datalen = atoi(optarg);
			break;
		case 'r': 
			SymRate = atof(optarg);
			break;
				
		case 'k': 
			modIndex = atof(optarg);
			break;
		
		case 's': 
			S_RATE = atof(optarg);
			break;

		default:
			MSGf("ERROR: argument parsing failure, use -h option for help\n");
			usage();
			return EXIT_FAILURE;
        }
    }
	
	readData(FileName);
	modType = modulationTypeIndex(modTypes);
	bitspersym = bitsperSymbol(modType);
	//		printf("BitsperSym: %d\n", bitspersym);
	
	basebandcarrier.frequency = basebandfrequency;
	basebandcarrier.amplitude = DEFAULTAMPLITUDE;
	passbandcarrier.frequency = passbandfrequency;
	passbandcarrier.amplitude = (float)DEFAULTAMPLITUDE;
	
	j = 0;
	k = 8 * datalen; //number of bits of data to be sent
	SymRate *= basebandcarrier.frequency;
	Tsym = 1/SymRate;
	time = 0;
	int bytepos;
	float T_RATE = (1.0/S_RATE);
	//		printf("amplitude: %f\n", basebandcarrier.amplitude);
	//		printf("Sampling Rate: %d\n", S_RATE);
	//		printf("SymRate: %f\n", SymRate);
	//		printf("Tsym: %f\n", Tsym);
	//		printf("NumBits: %d\n", k);
	//		printf("time: %f\n", time);
	//		printf("T_RATE: %f\n", T_RATE);
	//exit(0);
	float basebandT = 1/basebandcarrier.frequency;
	Symposprev = 0;
	byte bit;
	modConstantkf = (modIndex * basebandcarrier.frequency)/basebandcarrier.amplitude;
	i2 = 0;

	while(j < k)
	{
		Sympos = floor(time/Tsym);	//symbol position
		BitPos = (Sympos * bitspersym) + 1 - 1;	//start of bits for current symbol -1(Index from zero)
				
		bytepos = floor(BitPos/8);
		//printf("BitPos: %d\n", BitPos);
		
		byte beetpos = 0;
			
		byte ind = 0;
		for(ind = 0; ind < bitspersym; ind++)
		{
			bit = getBit(bytepos, beetpos++);
			bitsinSymbol[ind] = bit;
			//printf("%d", bitsinSymbol[ind]);
		}

		
		encoding basebandval = basebandmodulation(modType, time, bitsinSymbol);
		//printf("B_val: %f\n", basebandval);
		encoding passbandval = passbandmodulation(modType, basebandval, time);
		buffer[i2] = passbandval;
		//printf("Pas_val: %f\n", passbandval);
		//printf("%f\n", buffer[i2]);
		i2++;
		
			
		//time += 0.2;//(tym) T_RATE;
		time += T_RATE;
		//		printf("time: %f\n", time);
		//if(i2 ==2)		exit(0);
		//		printf("k: %d, j: %d, bitspersym: %d\n", k, j, bitspersym);
		if(Sympos > Symposprev )
		{
			Symposprev++;
			j += bitspersym;
			//printf("Sympos: %d, t: %f, Tsym: %f, bytepos: %d, k: %d, j:%d\n", Sympos, t, Tsym, bytepos,k,j);
			
		}
		
//*/		
	}
	//cserialmultiplexing.exe -f TrialFile.txt -b 10.4 -m  4QAM -p 12.5 -l 12 -r 1 -k 0.5
	fwrite(buffer, sizeof(encoding), i2, stdout);


    return 0;
}

byte getBit(int bytepos, int bitpos)
{
	char buyt = data[bytepos];
	//printf("%c\n", buyt);//exit(0);
	//printf("%s\n", data);
	//printf("%c\n", buyt);
	
	char beet = (buyt >> bitpos) & 0x00000001;
	byte tmp = (byte) beet;
	
	return tmp;
}

void readData(char *fname)
{
	uint32_t i;
	char c;
	FILE* dataFile = fopen(fname, "r");
	if(dataFile == NULL)csExit(0, fname);
	i = 0;
	while(i < datalen)
	{
		c = fgetc(dataFile);
		if( feof(dataFile))break ;
		data[i++] = c;
	}
	//data[i] = '\n';
	//printf("%s\n", data);
	//exit(0);
}

void csExit(byte errorCode, char* msg)
{
	fprintf(stderr,"FATAL: ");
	switch(errorCode)
	{
		case 0:
			fprintf(stderr,"File (%s) not found.\n", msg);
			break;
	}
	exit(0);
}

encoding basebandmodulation(byte modType, tym time, char* bits)
{
	encoding ret = 0;
	switch(modType)
	{
		case 0:
			ret = BPSK(time, bits);
			break;
		case 1:
			ret = BFSK(time, bits);
			break;
		case 2:
			ret = BASK(time, bits);
			break;
		case 3:
			ret = QAM4(time, bits);
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		case 8:
			break;
		case 9:
			break;
		case 10:
			break;
		case 11:
			break;
		case 12:
			ret = basebandCarrier(time);
			break;
		case 13:
			ret = passbandCarrier(time);
			break;
		case 14:
			ret = basebandCarrier(time);
			break;
			
	}
	
	return ret;
}


encoding passbandmodulation(byte modType, encoding basebandval, tym time)
{
	encoding ret = 0;
	float tmp = 0;
	switch(modType)
	{
		case 12:
			ret = basebandval;
			break;
		case 13:
			ret = (passbandcarrier.amplitude * cosFunc(passbandcarrier.frequency * time, 0));
			break;
		/*case 14:
			ret = basebandval;
			break;*/
		default:
			tmp = cosFunc(passbandcarrier.frequency * time, modConstantkf *  basebandval);
			ret = passbandcarrier.amplitude * tmp;//0;//(encoding) (passbandcarrier.amplitude * );
	}
	
	return ret;
}



encoding BPSK(tym t, char* bits)
{
	double tmp = cosFunc(basebandcarrier.frequency * t, 180 + ((2 * M_PI)/360)*(0 + bits[0] * 180));
	return (encoding) (basebandcarrier.amplitude * tmp);
}

encoding BFSK(tym t, char* bits)
{
	double tmp = cosFunc(basebandcarrier.frequency * (0.5 + bits[0] * 0.5) * t, 0);
	return (encoding) (basebandcarrier.amplitude * tmp);
}

encoding BASK(tym t, char* bits)
{
	double tmp = cosFunc(basebandcarrier.frequency * t, 0);
	return (encoding) (basebandcarrier.amplitude * (0.5 + bits[0] * 0.5) * tmp);
}

encoding QAM4(tym t, char* bits)
{
	double cause = cosFunc(basebandcarrier.frequency * t, 0);
	double sign = sinFunc(basebandcarrier.frequency * t, 0);
	
	cause = ((bits[0]%2) * - cause) + (((bits[0]+1)%2) * cause);
	sign = ((bits[1]%2) * - sign) + (((bits[1]+1)%2) * sign);
	
	encoding tmp = (encoding) (basebandcarrier.amplitude * (cause + sign));

	return tmp;
}

encoding basebandCarrier(tym t)
{	
	encoding tmp = (basebandcarrier.amplitude * cosFunc(basebandcarrier.frequency * t, 0));
	//	printf("Cosval: %f, then:%f\n", cosFunc(basebandcarrier.frequency * t, 0), tmp);	
	return tmp;
}

encoding passbandCarrier(tym t)
{
	encoding tmp = (passbandcarrier.amplitude * cosFunc(passbandcarrier.frequency * t, 0));
	return tmp;
}


byte modulationTypeIndex(char* modtype)
{
	byte ret = 12;
	strcmp(modtype,"BPSK")==0?ret = 0:false;
	strcmp(modtype,"BFSK")==0?ret = 1:false;
	strcmp(modtype,"BASK")==0?ret = 2:false;
	strcmp(modtype,"4QAM")==0?ret = 3:false;
	strcmp(modtype,"8QAM")==0?ret = 4:false;
	strcmp(modtype,"16QAM")==0?ret = 5:false;
	strcmp(modtype,"32QAM")==0?ret = 6:false;
	strcmp(modtype,"64QAM")==0?ret = 7:false;
	strcmp(modtype,"128QAM")==0?ret = 8:false;
	strcmp(modtype,"512QAM")==0?ret = 9:false;
	strcmp(modtype,"1024QAM")==0?ret = 10:false;
	strcmp(modtype,"2048QAM")==0?ret = 11:false;
	strcmp(modtype,"BASE")==0?ret = 12:false;
	strcmp(modtype,"PASS")==0?ret = 13:false;
	strcmp(modtype,"SINGLT")==0?ret = 14:false;
	return ret;
}

byte bitsperSymbol(byte modType)
{
	byte ret;
	switch(modType)
	{
		case 14:
		case 13:
		case 12:		
		case 0:
		case 1:
		case 2:
			ret = 0x1;
			break;
		case 3:
			ret =0x2;
			break;
		case 4:
			ret = 0x3;
			break;
		case 5:
			ret = 4;
			break;
		case 6:
			ret = 5;
			break;
		case 7:
			ret = 6;
			break;
		case 8:
			ret = 7;
			break;
		case 9:
			ret = 8;
			break;
		case 10:
			ret = 9;
			break;
		case 11:
			ret = 10;
			break;
		default:
			ret = 0;
	}
	return ret;
}

/* describe command line options */
void usage(void) {
    MSGf("Usage: cserialMultiplexing {options}\n");
    MSGf("Available options:\n");
    MSGf(" -h print this help\n");
    MSGf(" -f <str> Name of file with data to be modulated\n");
    MSGf(" -b <float> baseband frequency in Hz\n");
    MSGf(" -m <str> Modulation type ['BPSK, 'BFSK', 'BASK', '4QAM', '8QAM',..., '2048QAM', 'PASS', 'BASE', 'SINGLT for SingleTone']\n");
    MSGf(" -p <float> passband frequency in KHz\n");
    MSGf(" -s <int> Sampling Rate in Samples per second\n");
    MSGf(" -l <int> Length of data to modulate in bytes. Zero for entire file.\n");
    MSGf(" -r <float> Symbol rate (Symbols/second).\n");
    MSGf(" -k <float> Modulation Constant.\n");
}

//F:b:m:p:n:l:h

void MSGf (char* MSGf)
{
	printf(MSGf);
}

/*

./cserialmux -f TrialFile.txt -b 0.159154943 -m  BASE -p 12.5 -l 1 -r 1 -k 0.5 -s 8|sox -b 32 -t raw -r 8 -s -L -c1 -e float -b 32 - "outpute.wav" &&  sox outpute.wav outpute.dat

./cserialmux -f TrialFile.txt -b 1591.54943 -m  BASE -p 12.5 -l 10 -r 1 -k 0.5 -s 44100|sox -b 32 -t raw -r 44100 -s -L -c1 -e float -b 32 - "outpute.wav" &&  sox outpute.wav outpute.dat && sox outpute.wav -n spectrogram

signal at 10 Khz = 591.54943 rad/s
./cserialmux -f TrialFile.txt -b 1591.54943 -m  BASE -p 12.5 -l 10000 -r 1 -k 0.5 -s 44100|sox -b 32 -t raw -r 44100 -s -L -c1 -e float -b 32 - "signal10k.wav" && sox signal10k.wav -n spectrogram -l -o signal10k.png

signal at 5KHz
./cserialmux -f TrialFile.txt -b 795.774715 -m  BASE -p 12.5 -l 10000 -r 1 -k 0.5 -s 44100|sox -b 32 -t raw -r 44100 -s -L -c1 -e float -b 32 - "signal5k.wav" && sox signal5k.wav -n spectrogram -l -o signal5k.png

signal at 15k
./cserialmux -f TrialFile.txt -b 2387.324145 -m  BASE -p 12.5 -l 10000 -r 1 -k 0.5 -s 44100|sox -b 32 -t raw -r 44100 -s -L -c1 -e float -b 32 - "signal15k.wav" && sox signal15k.wav -n spectrogram -l -o signal15k.png

carrier at 10k
./cserialmux -f TrialFile.txt -b 1591.54943 -m  PASS -p 1.59154943 -l 10000 -r 1 -k 0.5 -s 44100|sox -b 32 -t raw -r 44100 -s -L -c1 -e float -b 32 - "carrier10k.wav" && sox carrier10k.wav -n spectrogram -l -o carrier10k.png

carrier at 5KHz
./cserialmux -f TrialFile.txt -b 1 -m  PASS -p 0.795774715 -l 1 -r 1 -k 0.5 -s 44100|sox -b 32 -t raw -r 44100 -s -L -c1 -e float -b 32 - "carrier5k.wav" && sox carrier5k.wav -n spectrogram -l -o carrier5k.png

carrier at 15k
./cserialmux -f TrialFile.txt -b 1 -m  PASS -p 2.387324145 -l 1 -r 1 -k 0.5 -s 44100|sox -b 32 -t raw -r 44100 -s -L -c1 -e float -b 32 - "carrier15k.wav" && sox carrier15k.wav -n spectrogram -l -o carrier15k.png



./cserialmux -f TrialFile.txt -b 5 -m  BASE -p 10 -l 100 -r 1 -k 0.5 -s 44100|sox -b 32 -t raw -r 44100 -s -L -c1 -e float -b 32 - "ctemp.wav" && sox ctemp.wav -n rate 20 spectrogram -o ctemp.png


./cserialmux -f TrialFile.txt -b 2 -m  SINGLT -p 12 -l 100 -r 1 -k 0.2 -s 30|sox -b 32 -t raw -r 30 -s -L -c1 -e float -b 32 - "ctemp.wav" && sox ctemp.wav -n rate 40 spectrogram -o ctemp.png

./cserialmux -f TrialFile.txt -b 2 -m  BASK -p 12 -l 100 -r 1 -k 0.2 -s 30|sox -b 32 -t raw -r 30 -s -L -c1 -e float -b 32 - "ctemp.wav" && sox ctemp.wav -n rate 40 spectrogram -o ctemp.png

./cserialmux -f TrialFile.txt -b 2 -m  BPSK -p 12 -l 100 -r 1 -k 0.2 -s 30|sox -b 32 -t raw -r 30 -s -L -c1 -e float -b 32 - "ctemp.wav" && sox ctemp.wav -n rate 40 spectrogram -o ctemp.png


./cserialmux -f TrialFile.txt -b 2 -m  BFSK -p 12 -l 100 -r 1 -k 0.2 -s 30|sox -b 32 -t raw -r 30 -s -L -c1 -e float -b 32 - "ctemp.wav" && sox ctemp.wav -n rate 40 spectrogram -o ctemp.png

./cserialmux -f TrialFile.txt -b 2 -m  4QAM -p 12 -l 10 -r 1 -k 0.2 -s 30|sox -b 32 -t raw -r 30 -s -L -c1 -e float -b 32 - "ctemp.wav" && sox ctemp.wav -n rate 40 spectrogram -o ctemp.png

-----------------
sox -n -r 48000 -c1 tmp.wav synth 1 sine 100 sine 200 && sox tmp.wav tmp.dat && sox tmp.wav -n rate 1000 spectrogram -o tmp.png

sox -n -r 48000 -c1 tmp1.wav synth 1 sine 100 && sox -n -r 48000 -c1 tmp2.wav synth 1 sine 200 && sox -m tmp1.wav tmp2.wav tmp.wav && sox tmp.wav tmp.dat && sox tmp.wav -n rate 1000 spectrogram -o tmp.png
---------------
gnuplot -p -e "plot 'tmp.dat' using 1:2"

gnuplot -p -e "set terminal png" -e "set output 'tmpx.png'" -e "set yrange [-1:1]" -e "set xrange [0:0.05]" -e "plot 'tmp.dat' using 1:2 with lines"

*/
