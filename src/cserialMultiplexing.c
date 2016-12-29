/********************************************************************************************\
 *				CSERIAL MULTIPLEXING																				  *
 *	AUTHOR	|Brian Onang'o																						  *
 *	COMPANY1|Upande Ltd (http://www.upande.co.ke) 														  *
 *	COMPANY2|Circuits and Systems Engineering Company (CSECO)										  *
 *	DATE	|December 2016														  									  *
 *_________________________________________________________________________________________ *
 *										Description														  			  *
 * Software for performing modulation in audio frequency range enabling the testing of RF   *
 * modulation concepts using audio frequency spectral analysis software.						  *
 * The specific modulation scheme implemented here is cserial multiplexing. More details	  *
 * about this modulation scheme can be found here -> ___________									  *
 * 														  															  *
 * For more information please see ReadMe.txt available with this file and the documentation* 
 * in the program comments.														  							  *
 *__________________________________________________________________________________________*
 *					Dependancies														  							  *
 *														  																  *
 *__________________________________________________________________________________________*
 *					LICENSE														  									  *
 *******************************************************************************************/

 /* Environment. Uncomment if working in linux */
//#define CSCOMPILER TCC

 /*
  * 			DEPENDANCIES
  */
#include <stdio.h>      /* printf fprintf sprintf fopen fputs */
#include <stdbool.h>    /* bool type */
#include <math.h>   		/* M_PI */
#include <stdbool.h>    /* bool type */
#include <stdlib.h>		/* exit codes */
#include <stdint.h>     /* C99 types */
#include <string.h>     /* C99 types */
#include <sys/stat.h>	/*file access*/
#include <fcntl.h>

#ifdef CSCOMPILER
	#include <cs_optarg.h>
#else 
	#include <unistd.h>     /* getopt access usleep */
#endif


/*
 *	Sampling rate and buffer size
 */
const int DEFAULT_S_RATE = 44100;
#define B_SIZE  (1024)
#define B_OUT_SIZE  (48100) 		//max sampling rate = 48100

/*
 *	Length of file name
 */
#define FNAMELEN 256

#define DEFAULTAMPLITUDE 1

#define DEFAULT_BASEBANDF 1
#define DEFAULT_PASSBANDF 5000

#define OUTFILE "rawout"

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
encoding basebandCarrier_f(tym time);
encoding passbandCarrier_f(tym time);
uint16_t writeData(uint16_t i2);
byte appendData(uint16_t i2);
byte createOutFile(void);
byte getChannel(char *channelStr);
carrier basebandcarrier;
carrier passbandcarrier;
char data[B_SIZE];
encoding buffer[B_OUT_SIZE];
double datalen = 0.125;	//1 bit = 1/8 bytes

float modConstantkf;
float modIndex = 0.2;	//default

int main(int argc, char **argv) 
{
	int i;
	uint16_t j, k, i2;
	tym time;
	encoding basebandvalue, passbandvalue;	
	byte modType, bitspersym;
	int S_RATE = DEFAULT_S_RATE;
	
	char arg_s[64];
	char FileName[FNAMELEN];
	char channelStr[4];
	byte channel = 1;//default is passband
	float basebandfrequency, passbandfrequency, Tb, Tsym, Td, Symfreq, SymRate; //symbol rate is relative to frequency. How many symbols per period?
	char modTypes[8];
	int Sympos, Symposprev, BitPos;
	char bitsinSymbol[16];
	
	basebandfrequency = DEFAULT_BASEBANDF;
	passbandfrequency = DEFAULT_PASSBANDF;
	SymRate = 1;	
	///* parse command line options 
    while ((i = getopt (argc, argv, "f:b:m:p:l:r:k:s:c:h")) != -1) {
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
		case 'c':
			strcpy(channelStr, optarg);
			break;

		default:
			MSGf("ERROR: argument parsing failure, use -h option for help\n");
			usage();
			return EXIT_FAILURE;
        }
    }
	
	byte createOFile = createOutFile();
	if(createOFile == 0)csExit(1, "");
	
	readData(FileName);
	
	modType = modulationTypeIndex(modTypes);
	bitspersym = bitsperSymbol(modType);
	
	basebandcarrier.frequency = basebandfrequency;
	basebandcarrier.amplitude = DEFAULTAMPLITUDE;
	passbandcarrier.frequency = passbandfrequency;
	passbandcarrier.amplitude = DEFAULTAMPLITUDE;
	
	printf("Baseband frequency (-b) set to: %f Hz\n", basebandcarrier.frequency); //remove from here
	printf("Passband frequency (-p) set to: %f Hz\n", passbandcarrier.frequency); //remove from here
	printf("Baseband amplitude set to: %f\n", basebandcarrier.amplitude); //remove from here
	printf("Passband amplitude set to: %f\n", passbandcarrier.amplitude); //remove from here
	j = 0;
	k = (int) (8 * datalen); //number of bits of data to be sent
	printf("Length of data to be sent (-l) set to:%d bits\n", (int) (8*datalen));//remove from here
	SymRate *= basebandcarrier.frequency;
	printf("Symbol Rate (-r) set to: %f symbols/s\n", SymRate);						//remove from here
	Tsym = 1/SymRate;
	time = 0;
	int bytepos;
	
	float T_RATE = (1.0/S_RATE);
	printf("Sampling Rate (-s) set to: %d symbols/s\n", S_RATE);						//remove from here
	float basebandT = 1/basebandcarrier.frequency;
	//
	Symposprev = 0;
	byte bit;
	modConstantkf = (modIndex * basebandcarrier.frequency)/basebandcarrier.amplitude;
	printf("Modulation Index (-k) set to: %f\n", modIndex);						//remove from here
	printf("Modulation Constant set to: %f\n", modConstantkf);					//remove from here
	channel = getChannel(channelStr);
	printf("Channel (-c) set to: %d (0: BASEband, 1: PASSband)\n", channel);	//remove from here
	printf(".\n..\n...\n....\n");
	i2 = 0;
	
	while(j < k)
	{
		Sympos = floor(time/Tsym);	//symbol position
		BitPos = (Sympos * bitspersym) + 1 - 1;	//start of bits for current symbol -1(Index from zero)
		
		bytepos = floor(BitPos/8);
		
		byte beetpos = 0;
			
		byte ind = 0;
		for(ind = 0; ind < bitspersym; ind++)
		{
			bit = getBit(bytepos, beetpos++);
			bitsinSymbol[ind] = bit;
		}

		
		encoding basebandval = basebandmodulation(modType, time, bitsinSymbol);
		encoding passbandval = passbandmodulation(modType, basebandval, time);
		if(channel == 1)buffer[i2] = passbandval;
		else buffer[i2] = basebandval;
		i2++;
		i2 = writeData(i2);			//write if buffer is full
		//i2++;
		time += T_RATE;
		if(Sympos > Symposprev )
		{
			Symposprev++;
			j += bitspersym;
			
		}
		
	}
	i2++;
	if(i2 > B_OUT_SIZE)i2 = B_OUT_SIZE;
	byte writeSuccess = appendData(i2);
	if(writeSuccess == 0)csExit(1, "");
	MSGf("Data successfully written to tmp/rawout\n");
   return 0;
}

byte getBit(int bytepos, int bitpos)
{
	char buyt = data[bytepos];	
	char beet = (buyt >> bitpos) & 0x00000001;
	byte tmp = (byte) beet;
	
	return tmp;
}

void readData(char *fname)
{
	uint32_t i;
	char c;
	char ifname[FNAMELEN];
	strcat(ifname,"tmp/");
	strcat(ifname,fname);
	FILE* dataFile = fopen(ifname, "r");
	*ifname = '\0';			//clear
	if(dataFile == NULL)
	{
		usage();
		csExit(0, fname);
	}
	i = 0;
	while(i < datalen)
	{
		c = fgetc(dataFile);
		if( feof(dataFile))break ;
		data[i++] = c;
	}
	fclose(dataFile);
}

uint16_t writeData(uint16_t i2)
{
	static int packet_count = 0;
	if(i2 == B_OUT_SIZE)
	{
		char ofname[FNAMELEN];
		*ofname = '\0';			//clear
		strcat(ofname,"tmp/");
		strcat(ofname,OUTFILE);
		FILE* dataFile = fopen(ofname, "a");
		*ofname = '\0';			//clear
		if(dataFile == NULL) return false;
		fwrite(buffer, sizeof(encoding), i2, dataFile);
		fclose(dataFile);
		packet_count++;
		return 0;	//reset i2
	}
	
	return i2;
}

byte appendData(uint16_t i2)
{
	char ofname[FNAMELEN];
	*ofname = '\0';			//clear
	strcat(ofname,"tmp/");
	strcat(ofname,OUTFILE);
	FILE* dataFile = fopen(ofname, "a");
	strcat(ofname,OUTFILE);
	fwrite(buffer, sizeof(encoding), i2, dataFile);
	fclose(dataFile);
	*ofname = '\0';			//clear
	return true;
}

byte createOutFile()
{
	byte ret = true;
	char ofname[FNAMELEN];
	strcat(ofname,"tmp/");
	strcat(ofname,OUTFILE);
	int fd = open(ofname, O_RDWR|O_CREAT, 0777); 
	if(fd == -1)ret = false;
	*ofname = '\0';			//clear ofname
	ret = true;
	
	return ret;
}

void csExit(byte errorCode, char* msg)
{
	fprintf(stderr,"FATAL: ");
	switch(errorCode)
	{
		case 0:
			fprintf(stderr,"File (%s) not found.\n", msg);
			break;
		case 1:
			fprintf(stderr, "Error writing to tmp/rawout. Check that tmp/ exists and you have the permission to write to it\n");
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
			ret = basebandCarrier_f(time);
			break;
		case 13:
			ret = 0;
			break;
		case 14:
			ret = basebandCarrier_f(time);
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
		/*case 13:
			//ret = (passbandcarrier.amplitude * cosFunc(passbandcarrier.frequency * time, 0));
			ret = passbandCarrier_f(time);
			break;*/
		/*case 14:
			ret = basebandval;
			break;*/
		default:
			tmp = cosFunc(passbandcarrier.frequency * time, modConstantkf *  basebandval);
			ret = passbandcarrier.amplitude * tmp;
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

encoding basebandCarrier_f(tym t)
{	
	encoding tmp = (basebandcarrier.amplitude * cosFunc(basebandcarrier.frequency * t, 0));	
	return tmp;
}

encoding passbandCarrier_f(tym t)
{
	encoding tmp = (passbandcarrier.amplitude * cosFunc(passbandcarrier.frequency * t, 0));
	return tmp;
}


byte modulationTypeIndex(char* modtype)
{
	//char tmp[128];
	byte ret = 12;					//default
	strcmp(modtype,"")==0?strcat(modtype,"BASE"):false;
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
	
	printf("Modulation type Set to: %d (%s)\n", ret, modtype); //remove from here
	return ret;
}

byte getChannel(char *channelStr)
{
	byte channel;
	strcmp(channelStr,"BASE")==0?channel = 0:false;
	strcmp(channelStr,"PASS")==0?channel = 1:false;
	
	return channel;
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
	
	printf("Bits/Symbol Set to: %d \n", ret); //remove from here
	return ret;
}

/* describe command line options */
void usage(void) {
    MSGf("Usage: cserialMultiplexing {options}\n");
    MSGf("Available options:\n");
    MSGf(" -h print this help\n");
    MSGf(" -f <str> Name of file with data to be modulated. Stored in tmp/ folder in root dir\n");
    MSGf(" -b <float> baseband frequency in Hz\n");
    MSGf(" -m <str> Modulation type ['BPSK, 'BFSK', 'BASK', '4QAM', '8QAM',..., '2048QAM', 'PASS', 'BASE', 'SINGLT for SingleTone']\n");
    MSGf(" -p <float> passband frequency in KHz\n");
    MSGf(" -s <int> Sampling Rate in Samples per second\n");
    MSGf(" -l <int> Length of data to modulate in bytes. Zero for entire file.\n");
    MSGf(" -r <float> Symbol rate (Symbols/second).\n");
    MSGf(" -k <float> Modulation Constant.\n");
    MSGf(" -c <str> Channel {BASE(band) or PASS(band)}.\n");
}

//F:b:m:p:n:l:h

void MSGf (char* msgf)
{
	printf("%s", msgf);
}

