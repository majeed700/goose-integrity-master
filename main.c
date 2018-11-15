#include <LPC214X.H>
#include "16X2_LCD.h"	
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "sha1.h"

#define DEFAULT_GOOSE_MAX_LENGTH 400
#define DEFAULT_CRC_LENGTH 4

#define PLOCK 0x00000400
#define PRESCALE 60 //60000 PCLK clock cycles to increment TC by 1

typedef unsigned char byte;
typedef enum {false, true} bool;
struct keys {
	byte encrypt_key[DEFAULT_GOOSE_MAX_LENGTH+DEFAULT_CRC_LENGTH+16];
	char pad_key_bits[2*DEFAULT_CRC_LENGTH+1];
};

struct keys keystreams;

const char candidate[] = "hellohgdsafjsdak;fhadslkfhsdkjfh;lsdkjfskdhfhellojifdsfasjkf;ljasdkf;ljsadkfhasdojkfjdsal;kfjsakl;fjsakl;fjsal;fjksahellohgdsafjsdak;fhadslkfhsdkjfh;lsdkjfskdhfhellojifdsfasjkf;ljasdkf;ljsadkfhasdojkfjdsal;kfjsakl;fjsakl;fjsal;fjksahellohgdsafjsdak;fhadslkfhsdkjfh;lsdkjfskdhfhellojifdsfasjkf;ljasdkf;ljsadkfhasdojkfjdsal;kfjsakl;fjsakl;fjsal;fjksfdasfasdfsdafsdafsadfsdafsdafasdfsdafsadfwqekjhkjhgfda";
int length = 300;
char goose[DEFAULT_GOOSE_MAX_LENGTH+1];


void initTimer0(void)
{
	/*Assuming that PLL0 has been setup with CCLK = 60Mhz and PCLK also = 60Mhz.*/

	T0CTCR = 0x0;
	T0TCR = 0x00;
	T0PR = PRESCALE - 1; //(Value in Decimal!) - Increment T0TC at every 60000 clock cycles
	//Count begins from zero hence subtracting 1
	//60000 clock cycles @60Mhz = 1 mS

	T0TCR = 0x02; //Reset Timer
}


//---------PLL Related Functions :---------------

//Using PLL settings as shown in : http://www.ocfreaks.com/lpc214x-pll-tutorial-for-cpu-and-peripheral-clock/

void setupPLL0(void)
{
	//Note : Assuming 12Mhz Xtal is connected to LPC2148.

	PLL0CON = 0x01; // PPLE=1 & PPLC=0 so it will be enabled
					// but not connected after FEED sequence

	PLL0CFG = 0x24; // set the multipler to 5 (i.e actually 4)
					// i.e 12x5 = 60 Mhz (M - 1 = 4)!!!
					// Set P=2 since we want FCCO in range!!!
					// So , Assign PSEL =01 in PLL0CFG as per the table.
}

void feedSeq(void)
{
	PLL0FEED = 0xAA;
	PLL0FEED = 0x55;
}

void connectPLL0(void)
{
	// check whether PLL has locked on to the  desired freq by reading the lock bit
	// in the PPL0STAT register

	while( !( PLL0STAT & PLOCK ));

	// now enable(again) and connect
	PLL0CON = 0x03;
}

void initClocks(void)
{
    setupPLL0();
    feedSeq(); //sequence for locking PLL to desired freq.
    connectPLL0();
    feedSeq(); //sequence for connecting the PLL as system clock
   
    //SysClock is now ticking @ 60Mhz!
       
    VPBDIV = 0x01; // PCLK is same as CCLK i.e 60Mhz
    
    //Using PLL settings as shown in : http://www.ocfreaks.com/lpc214x-pll-tutorial-for-cpu-and-peripheral-clock/
    //PLL0 Now configured!
}



void CRCByteToString(char* buffer, byte* crc_bytes, int length) {
	char index = 0;
	int factor[8] = {128, 64, 32, 16, 8, 4, 2, 1};
	int i, j;
	for(i=0; i < length; i++) {
		byte cur_byte = crc_bytes[i];
		byte rem = cur_byte;
		for(j=0; j < 8; j++) {
			byte quo = rem / factor[j];
			rem = rem - factor[j] * quo;
			buffer[index++] = quo;
		}
	}
}


void calculateCRC32(byte* crc_bytes, const char *buf, size_t len)
{
	int crc = 0;
	static int table[256];
	static int have_table = 0;
	int rem;
	byte octet;
	int i, j;
	const char *p, *q;
 
	if (have_table == 0) {
		for (i = 0; i < 256; i++) {
			rem = i;  
			for (j = 0; j < 8; j++) {
				if (rem & 1) {
					rem >>= 1;
					rem ^= 0xedb88320;
				} else
					rem >>= 1;
			}
			table[i] = rem;
		}
		have_table = 1;
	}
 
	crc = ~crc;
	q = buf + len;
	for (p = buf; p < q; p++) {
		octet = *p;
		crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
	}
	crc = ~crc;
	crc_bytes[0] = crc >> 24;
	crc_bytes[1] = (crc & 16711680) >> 16;
	crc_bytes[2] = (crc & 65280) >> 8;
	crc_bytes[3] = crc & 255;
}


void keygen(char* secret_key, char* date, char* tsi, char* stNum, char* sqNum, int secret_key_len) {//specify the expected goose length, if 0, set to max
	char salt[2];
	int cnt, keystream_index, i, j;
	unsigned int digest_num;
	byte pad[2*DEFAULT_CRC_LENGTH]={0};
	SHA1_CTX sha;
	
	int payload_len = length;
	int encrypt_key_len = payload_len + DEFAULT_CRC_LENGTH + secret_key_len; 
	size_t keystream_len = encrypt_key_len + 2*DEFAULT_CRC_LENGTH; // the keys for encryption and padding
	int digest_len = 20;
	
	byte keystream_bytes[DEFAULT_GOOSE_MAX_LENGTH+16+2*DEFAULT_CRC_LENGTH];
	double tt = (double)keystream_len/digest_len - keystream_len/digest_len;
	
	//struct keys* keystreams = (struct keys*)malloc(keystream_len+1);
	
	// the times of hash operations
	if(tt > 0) {
		digest_num = keystream_len/digest_len + 1;
	} else {
		digest_num = keystream_len/digest_len;
	}
	memcpy(salt, stNum, 2);
	strcat(salt, sqNum);
	strcat(salt, date);
	strcat(salt, tsi);
	
	//printf("%s\n", salt);
	SHA1Init(&sha);
	
	for(cnt = 0, keystream_index=0; cnt < digest_num; cnt++, keystream_index += digest_len) {
		char cnt_str[3];
		unsigned char to_be_hashed[17];
		byte digest[20];
		char salt_temp[12];		
		memcpy(salt_temp, salt, strlen(salt));
		salt_temp[11] = '\0';
		sprintf(cnt_str, "%d", cnt);
		strcat(salt_temp, cnt_str);
		//printf("salt_temp: %s\n", salt_temp);
		
		//sprintf(snum, "%d", strlen(secret_key));
		//********************************
		memcpy(to_be_hashed, secret_key, strlen(secret_key));
		to_be_hashed[16] = '\0';	
		strcat((char*)to_be_hashed, salt_temp);	
		//********************************
		
		SHA1Update(&sha, to_be_hashed, strlen((char*)to_be_hashed)); SHA1Final(digest, &sha);
		for(i=0, j=keystream_index; i < digest_len; i++, j++) {
			keystream_bytes[j] = digest[i];
		}
	}
	
	
	// did not consider the secret key in the following, its default length is 0
	for(i=0; i < payload_len + DEFAULT_CRC_LENGTH + secret_key_len; i++) {
		keystreams.encrypt_key[i] = keystream_bytes[i];
	}
	for(i=0; i < 2*DEFAULT_CRC_LENGTH; i++) {
		pad[i] = keystream_bytes[payload_len+DEFAULT_CRC_LENGTH+secret_key_len+i];
	}
	
	CRCByteToString(keystreams.pad_key_bits, pad, 2*DEFAULT_CRC_LENGTH);
	keystreams.pad_key_bits[2*DEFAULT_CRC_LENGTH] = '\0';
	//lcd_command(0xc0);lcd_display_string("here");
	//return keystreams;
}


byte flip(byte cur_byte, byte key_pos) {
	byte factor[8] = {128, 64, 32, 16, 8, 4, 2, 1};
	byte result = cur_byte ^ factor[key_pos];
	return result;
}

byte flip_other(byte cur_byte, byte key_pos) {
	byte factor[8] = {127, 191, 223, 239, 247, 251, 253, 254};
	byte result = cur_byte ^ factor[key_pos];
	return result;
}

byte rotate(byte cur_byte) {
	byte result;
	byte first_bit = (cur_byte & 128) / 128;
	result = cur_byte << 1;
	result += first_bit;
	return result;
}

byte reverse_rotate(byte cur_byte) {
	byte result;
	byte last_bit = cur_byte & 1;
	result = cur_byte >> 1;
	result += last_bit * 128;
	return result;
}


void pad(byte* padded_crc_bytes, char* pad_key_bits, byte* crc_bytes)
{
	int bit_index;
	memcpy(padded_crc_bytes, crc_bytes, DEFAULT_CRC_LENGTH);	
	for(bit_index=0; bit_index < 2*DEFAULT_CRC_LENGTH; bit_index+=2) {
		byte cur_key_bit1 = pad_key_bits[bit_index];
		byte cur_key_bit2 = pad_key_bits[bit_index+1];
		byte key_bits_pos = (bit_index % 8)/2;		
		byte cur_crc_index = bit_index/16;
		byte cur_crc_byte = padded_crc_bytes[cur_crc_index];		
		if(cur_key_bit1 == '0' && cur_key_bit2 == '0') { //00
		}
		else if (cur_key_bit1 == '0' && cur_key_bit2 == '1') { //01
			padded_crc_bytes[cur_crc_index] = flip(cur_crc_byte, key_bits_pos);
		}
		else if (cur_key_bit1 == '1' && cur_key_bit2 == '0') { //10
			padded_crc_bytes[cur_crc_index] = flip_other(cur_crc_byte, key_bits_pos);
		}
		else if (cur_key_bit1 == '1' && cur_key_bit2 == '1') { //11
			padded_crc_bytes[cur_crc_index] = rotate(cur_crc_byte); 
		}
	}
};


void encrypt(byte* cipher, byte* padded_crc_bytes, byte* encrypt_key)
{
	int i;
	int payload_len = length;
	for(i=0; i < payload_len; i++) {
		//cout << encrypt_key[i];
		cipher[i] = goose[i] ^ encrypt_key[i];
	}
	
	for(i=payload_len; i < payload_len+DEFAULT_CRC_LENGTH; i++) {
		//cout << encrypt_key[i];
		cipher[i] = padded_crc_bytes[i-payload_len] ^ encrypt_key[i];
	}
};


void decrypt(byte* plain, byte* cipher_goose, byte* decrypt_key, byte* original_padded_crc)
{
	int i;
	int cipher_len = length + DEFAULT_CRC_LENGTH;
	for(i = 0; i < cipher_len; i++) {
		plain[i] = cipher_goose[i] ^ decrypt_key[i];
	}
};


void depad(byte* recovered_crc_bytes, byte* decipher_goose, char* depad_key_bits)
{
	int payload_len = length;
	int j, bit_index;
	byte padded_crc_bytes[DEFAULT_CRC_LENGTH];	
	for(j=0; j < DEFAULT_CRC_LENGTH; j++) {
		padded_crc_bytes[j] = decipher_goose[payload_len+j];
	}	
	memcpy(recovered_crc_bytes, padded_crc_bytes, DEFAULT_CRC_LENGTH);	
	for(bit_index = 2*DEFAULT_CRC_LENGTH-2; bit_index >=0; bit_index-=2) {
		byte cur_key_bit1 = depad_key_bits[bit_index];
		byte cur_key_bit2 = depad_key_bits[bit_index+1];
		byte key_bits_pos = (bit_index%8)/2;
		
		byte cur_crc_index = bit_index/16;
		byte cur_crc_byte = recovered_crc_bytes[cur_crc_index];

		if(cur_key_bit1 == '0' && cur_key_bit2 == '0') { //00
			//do nothing
		}
		else if (cur_key_bit1 == '0' && cur_key_bit2 == '1') { //01
			recovered_crc_bytes[cur_crc_index] = flip(cur_crc_byte, key_bits_pos);
		}
		else if (cur_key_bit1 == '1' && cur_key_bit2 == '0') { //10
			recovered_crc_bytes[cur_crc_index] = flip_other(cur_crc_byte, key_bits_pos);
		}
		else if (cur_key_bit1 == '1' && cur_key_bit2 == '1') { //11
			recovered_crc_bytes[cur_crc_index] = reverse_rotate(cur_crc_byte); 
		}
	}
};

bool check(byte* decipher_goose, byte* recovered_crc_bytes) {
	int i;
	bool result = true;
	int payload_len = length;
	
	byte newCRC[DEFAULT_CRC_LENGTH];
	calculateCRC32(newCRC, (const char*)decipher_goose, payload_len);
	
	for(i=0; i < DEFAULT_CRC_LENGTH; i++) {
		if(recovered_crc_bytes[i] != newCRC[i]) {
			result = false;
		}
	}
	return result;
};

	
int main() {
	int keygen_start, keygen_end, keygen_duration;
	char keygen_num[8];
	char keygen_display[3] = "k::";
	char display[1] = "";
	int embed_start, embed_end, embed_duration;
	char embed_num[12];
	char embed_display[3] = "e:";
	int verify_start, verify_end, verify_duration;
	char verify_num[12];
	char verify_display[3] = "v:";
	
	byte crc_bytes[DEFAULT_CRC_LENGTH];
	byte padded_crc_bytes[DEFAULT_CRC_LENGTH];
	byte ciphertext[DEFAULT_GOOSE_MAX_LENGTH+DEFAULT_CRC_LENGTH];
	byte decrypted[DEFAULT_GOOSE_MAX_LENGTH+DEFAULT_CRC_LENGTH];
	byte recovered_crc_bytes[DEFAULT_CRC_LENGTH];
	bool flag;
	
	strncpy(goose, candidate, length);

	initClocks(); //Initialize CPU and Peripheral Clocks @ 60Mhz
	initTimer0(); //Initialize Timer0
	//IO0DIR = 0xFFFFFFFF; //Configure all pins on Port 0 as Output
	
	IO0DIR=0xfdbf6dcd;      
	IO1DIR=0xfdffffff;
	lcd_int();
	
	lcd_command(0x80);lcd_display_string("The results are");lcd_1ms(3000);

	T0CTCR = 0x0;
	T0TCR = 0x00;
	T0PR = 59;
	T0TCR = 0x02; //Reset Timer
	T0TCR = 0x01; //Enable timer
	keygen_start = T0TC;
	keygen(
								"this is seedadeg\0",
								"datdat\0",
								"ts\0",
								"st\0",
								"q\0",
								16
								);
	keygen_end = T0TC;
	keygen_duration = keygen_end - keygen_start;
	sprintf(keygen_num, "%d", keygen_duration);
	strcat(keygen_display, keygen_num);
	strcat(display, "k");
	strcat(display, keygen_num);
	strcat(display, "k");
	lcd_command(0xc0);lcd_display_string(display);
	
	/*
	T0CTCR = 0x0;
	T0TCR = 0x00;
	T0PR = 59;*/
	T0TCR = 0x02; //Reset Timer
	T0TCR = 0x01; //Enable timer
	embed_start = T0TC;
	calculateCRC32(crc_bytes, goose, length);
	pad(padded_crc_bytes, keystreams.pad_key_bits, crc_bytes);
	encrypt(ciphertext, padded_crc_bytes, keystreams.encrypt_key);	
	embed_end = T0TC;
	embed_duration = embed_end - embed_start;
	sprintf(embed_num, "%d", embed_duration);
	strcat(embed_display, embed_num);
	strcat(display, "e");
	strcat(display, embed_num);
	strcat(display, "e");
	
	
	/*
	T0CTCR = 0x0;
	T0TCR = 0x00;
	T0PR = 59999;*/
	T0TCR = 0x02; //Reset Timer
	T0TCR = 0x01; //Enable timer
	verify_start = T0TC;
	decrypt(decrypted, ciphertext, keystreams.encrypt_key, padded_crc_bytes);	
	depad(recovered_crc_bytes, decrypted, keystreams.pad_key_bits);
	flag = check(decrypted, recovered_crc_bytes);
	verify_end = T0TC;
	verify_duration = verify_end - verify_start;
	sprintf(verify_num, "%d", verify_duration);
	strcat(verify_display, verify_num);
	strcat(display, "v");
	strcat(display, verify_num);

  return 0;
}
