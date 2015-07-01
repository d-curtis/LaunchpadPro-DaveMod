#include <iostream>
#include <fstream>

// Ported from the original Delphi version.
// CLI parameters for ID, ByteWidth and BaseAddress removed for simplicity

#include "intelhex.h"

static const int ByteWidth = 32;
static const int ID = 0x0051;

static const unsigned char RESET[] = {0xf0, 0x00, 0x20, 0x29, 0x00, 0x71};

// must match unpacking code in the bootloader, obviously
static void eight_to_seven(unsigned char * Output, const int OOffset,
			intelhex::hex_data& Input, const unsigned long IOffset)
{
	for (int i = 0; i < 7; ++i)
	{
		if (!Input.is_set(IOffset + i))
		{
			// pad unset addresses
			Input.set(IOffset + i, 0xff);
		}
	}
	
	// seven bytes of eight-bit data converted to
	// eight bytes of seven-bit data
	// render 8-bit words as 16-bit words
	Output[OOffset+0] =                           Input[IOffset+0] >> 1;
	Output[OOffset+1] = (Input[IOffset+0] << 6) + (Input[IOffset+1] >> 2);
	Output[OOffset+2] = (Input[IOffset+1] << 5) + (Input[IOffset+2] >> 3);
	Output[OOffset+3] = (Input[IOffset+2] << 4) + (Input[IOffset+3] >> 4);
	Output[OOffset+4] = (Input[IOffset+3] << 3) + (Input[IOffset+4] >> 5);
	Output[OOffset+5] = (Input[IOffset+4] << 2) + (Input[IOffset+5] >> 6);
	Output[OOffset+6] = (Input[IOffset+5] << 1) + (Input[IOffset+6] >> 7);
	Output[OOffset+7] = (Input[IOffset+6]);

	for (int i = 0; i < 8; ++i)
	{
		Output[OOffset+i] &= 0x7f;
	}
}

static void write_block(intelhex::hex_data& data, std::ofstream& ofs, const unsigned long addr, const unsigned char type)
{
	// packet header
	ofs.write(reinterpret_cast<const char*>(RESET), 5);
	ofs.put(type);
	
	int incount = 0;
	int outcount = 0;
	int outn = 1 + (ByteWidth * 8) / 7;
	
	// payload
	unsigned char payload[41];
	
	while (incount < ByteWidth)
	{
		eight_to_seven(payload, outcount, data, addr+incount);
		
		incount += 7;
		outcount += 8;
	}
	
	ofs.write(reinterpret_cast<const char*>(payload), outn);
	ofs.put(0xf7);
}

static void write_header(intelhex::hex_data& data, std::ofstream& ofs, size_t BaseAddress)
{
	// human-readable version number & header block
	ofs.write(reinterpret_cast<const char*>(RESET), 6);
	ofs.put(ID >> 8);
	ofs.put(ID & 0x7f);
	
	ofs.put(data[BaseAddress + 0x132] >> 4);
	ofs.put(data[BaseAddress + 0x132] & 0x0f);
	ofs.put(data[BaseAddress + 0x131] >> 4);
	ofs.put(data[BaseAddress + 0x131] & 0x0f);
	ofs.put(data[BaseAddress + 0x130] >> 4);
	ofs.put(data[BaseAddress + 0x130] & 0x0f);
	
	ofs.put(0xf7);
}

static void write_checksum(intelhex::hex_data& data, std::ofstream& ofs)
{
	// device doesn't respect the checksum, but we still need this block!
	ofs.write(reinterpret_cast<const char*>(RESET), 5);
	unsigned char payload[19];
	
	const char *FIRMWARE = "Firmware";
	
	payload[0] = 0x76;
	payload[1] = 0x00;
	for (int i = 0; i < 8; ++i)
	{
		payload[i+2] = FIRMWARE[i];
	}
	payload[10] = 0x00;
	payload[11] = 0x00;
	payload[12] = 0x00;
	payload[13] = 0x00;
	payload[14] = 0x00;
	payload[15] = 0x00;
	payload[16] = 0x00;
	payload[17] = 0x00;
	payload[18] = 0xf7;
	
	ofs.write(reinterpret_cast<const char*>(payload), 19);
}

int main(int argc, char *argv[])
{
	std::cout << "converting " << argv[1] << " to sysex file: " << argv[2] << std::endl;
	
	// read the hex file input
	intelhex::hex_data data;
	data.load(argv[1]);
	
	size_t BaseAddress = data.min_address();
	size_t MaxAddress = data.max_address();
	
	std::cout << "max addr: " << std::hex << data.max_address() << " min_addr: " << std::hex << data.min_address() << std::endl;
	
	// create output file
	std::ofstream ofs(argv[2] , std::ios::out | std::ios::binary);
	if( !ofs )
		return -1;
	
	write_header(data, ofs, BaseAddress);
	
	// payload blocks...
	unsigned long i = BaseAddress + ByteWidth;
	
	while (i < MaxAddress)
	{
		write_block(data, ofs, i, 0x72);
		
		i += ByteWidth;
	}
	
	write_block(data, ofs, BaseAddress, 0x73);
	
	// footer/checksum block
	write_checksum(data, ofs);
	
	ofs.close();
	
	return 0;
}