// In this version, I remove the dependencies on the Cryptopp library to make the code work on microcontroller.
#include <LPC214X.H>
#include "16X2_LCD.h"
#include <string>
//#include "crypt3.h"

#define DEFAULT_GOOSE_MAX_LENGTH 50
#define DEFAULT_CRC_LENGTH 4
typedef unsigned char byte;
struct keys {
	byte encrypt_key[DEFAULT_GOOSE_MAX_LENGTH+DEFAULT_CRC_LENGTH+16];
	string pad_key_bits;
};
struct checks {
	string recovered_goose;
	string recovered_crc;
};

int main(int argc, char* argv[])
{	
	/*
	string goose = "hellohgdsafjsdak;fhadslkfhsdkjfh;lsdkjfskdhfhelloji";
	int len = 50;
	string payload = goose.substr(0, len);
	Crypt crypt1(payload);
	clock_t start_whole, end_whole;
	struct keys twokeys1 = crypt1.keygen(
							"this is seed",
							"this is date",
							"this is time slot",
							"this is stNum",
							"this is sqNum"
							);
	byte crc_bytes[DEFAULT_CRC_LENGTH];
	start_whole = clock();	
	crypt1.calculateCRC32(crc_bytes, payload.c_str(), len);
	byte padded_crc_bytes[DEFAULT_CRC_LENGTH];
	crypt1.pad(padded_crc_bytes, twokeys1.pad_key_bits, crc_bytes);
	byte ciphertext[DEFAULT_GOOSE_MAX_LENGTH+DEFAULT_CRC_LENGTH]={0};
	crypt1.encrypt(ciphertext, padded_crc_bytes, twokeys1.encrypt_key);
	byte decrypted[DEFAULT_GOOSE_MAX_LENGTH+DEFAULT_CRC_LENGTH]={0};
	crypt1.decrypt(decrypted, ciphertext, twokeys1.encrypt_key, padded_crc_bytes);
	byte recovered_crc_bytes[DEFAULT_CRC_LENGTH]={0};
	crypt1.depad(recovered_crc_bytes, decrypted, twokeys1.pad_key_bits);
	bool flag =  crypt1.check(decrypted, recovered_crc_bytes);
	end_whole = clock();
	double duration_whole = (double)(end_whole-start_whole) / CLOCKS_PER_SEC;
	*/
	
	IO0DIR=0xfdbf6dcd;      
	IO1DIR=0xfdffffff;
	lcd_int();
	lcd_command(0x80);lcd_display_string("MY TechnoCare");lcd_1ms(3000);
	lcd_command(0xc0);lcd_display_string("RF Commu. Test");
  return 0;
}
