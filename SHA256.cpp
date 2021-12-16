#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <fstream>

#define LIMIT 0x100000000 //2^32    

uint32_t h0 = 0x6a09e667;
uint32_t h1 = 0xbb67ae85;
uint32_t h2 = 0x3c6ef372;
uint32_t h3 = 0xa54ff53a;
uint32_t h4 = 0x510e527f;
uint32_t h5 = 0x9b05688c;
uint32_t h6 = 0x1f83d9ab;
uint32_t h7 = 0x5be0cd19;

uint32_t k[64] = { 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };

uint32_t right_rotate(uint32_t n, uint8_t t)
{
	while (t != 0)
	{
		uint32_t mask = n << 31;
		n = (uint32_t)(n >> 1);
		n = (uint32_t)(n | mask);
		t--;
	}
	return n;
}
uint32_t right_shift(uint32_t n, uint8_t t)
{
	n = n >> t;
	return n;
}
std::vector<uint8_t> pre_process(std::string message)
{
	uint64_t uint64MessageLength = (uint64_t)message.size();
	uint64_t uint64MessageLengthBits = (uint64_t)message.size() * 8;
	std::vector<uint8_t> uint8Message;
	std::for_each(message.begin(), message.end(), [&](const char letter) { uint8Message.push_back((uint8_t)letter); });
	uint8Message.push_back((uint8_t)0x80);
	uint8_t k = 0;
	while ((k + uint64MessageLength + 9) % 64 != 0)
	{
		k++;
	}
	for (size_t i = 0; i < k; i++)
	{
		uint8Message.push_back((uint8_t)0x0);
	}
	uint8_t temp[8];
	for (size_t i = 0; i < 8; i++)
	{
		temp[7 - i] = uint64MessageLengthBits & 0xFF;
		uint64MessageLengthBits >>= 8;
	}
	std::for_each(&temp[0], &temp[8], [&](uint8_t var) {uint8Message.push_back(var); });
	return uint8Message;
}
std::vector<std::vector<uint32_t>> schedule_message(std::vector<uint8_t> message)
{
	std::vector<std::vector<uint8_t>> uint8MessageChunks(message.size() / 64, std::vector<uint8_t>(0, 0));
	size_t c = 0;
	for (size_t i = 0; i < message.size(); i++)
	{
		if (i % 64 == 0 && i != 0)
		{
			c++;
		}
		uint8MessageChunks[c].push_back(message[i]);
	}
	c = 0;
	std::vector<std::vector<uint32_t>> uint32ScheduleChunks(message.size() / 64, std::vector<uint32_t>(0, 0));
	for (size_t n = 0; n < uint32ScheduleChunks.size(); n++)
	{
		uint32_t temp = 0;
		for (size_t i = 0; i < uint8MessageChunks[c].size(); i++)
		{
			if (i % 4 == 0 && i != 0)
			{
				uint32ScheduleChunks[n].push_back(temp);
				temp = 0;
			}
			temp = (temp << 8) | uint8MessageChunks[c][i];
			if (i == uint8MessageChunks[c].size() - 1)
			{
				uint32ScheduleChunks[n].push_back(temp);
			}
		}
		if (uint32ScheduleChunks[n].size() < 64)
		{
			while (uint32ScheduleChunks[n].size() != 64)
			{
				uint32ScheduleChunks[n].push_back((0x0));
			}
		}
		for (size_t i = 16; i < 64; i++)
		{
			uint32_t s0 = (right_rotate(uint32ScheduleChunks[n][i - 15], 7)) ^ (right_rotate(uint32ScheduleChunks[n][i - 15], 18)) ^ (right_shift(uint32ScheduleChunks[n][i - 15], 3));
			uint32_t s1 = (right_rotate(uint32ScheduleChunks[n][i - 2], 17)) ^ (right_rotate(uint32ScheduleChunks[n][i - 2], 19))  ^ (right_shift(uint32ScheduleChunks[n][i - 2], 10));
			uint32ScheduleChunks[n][i] = ((uint64_t)uint32ScheduleChunks[n][i - 16] + s0 + uint32ScheduleChunks[n][i - 7] + s1) % LIMIT;
		}
		c++;
	}
	return uint32ScheduleChunks;
}
std::string compress(std::vector<std::vector<uint32_t>> w)
{
	uint32_t a = h0;
	uint32_t b = h1;
	uint32_t c = h2;
	uint32_t d = h3;
	uint32_t e = h4;
	uint32_t f = h5;
	uint32_t g = h6;
	uint32_t h = h7;
	for (size_t n = 0; n < w.size(); n++)
	{
		a = h0;
		b = h1;
		c = h2;
		d = h3;
		e = h4;
		f = h5;
		g = h6;
		h = h7;
		for (size_t i = 0; i < 64; i++)
		{
			uint32_t s0 = (right_rotate(a, 2)) ^ (right_rotate(a, 13)) ^ (right_rotate(a, 22));
			uint32_t s1 = (right_rotate(e, 6)) ^ (right_rotate(e, 11)) ^ (right_rotate(e, 25));
			uint32_t ch = (e & f) ^ ((~e) & g);
			uint32_t temp0 = ((uint64_t)h + s1 + ch + k[i] + w[n][i]) % LIMIT;
			uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
			uint32_t temp1 = ((uint64_t)s0 + maj) % LIMIT;
			h = g;
			g = f;
			f = e;
			e = ((uint64_t)d + temp0) % LIMIT;
			d = c;
			c = b;
			b = a;
			a = ((uint64_t)temp0 + temp1) % LIMIT;
		}
		h0 = ((uint64_t)h0 + a) % LIMIT;
		h1 = ((uint64_t)h1 + b) % LIMIT;
		h2 = ((uint64_t)h2 + c) % LIMIT;
		h3 = ((uint64_t)h3 + d) % LIMIT;
		h4 = ((uint64_t)h4 + e) % LIMIT;
		h5 = ((uint64_t)h5 + f) % LIMIT;
		h6 = ((uint64_t)h6 + g) % LIMIT;
		h7 = ((uint64_t)h7 + h) % LIMIT;
	}

	std::stringstream ss0;
	ss0 << std::hex << h0;
	std::stringstream ss1;
	ss1 << std::hex << h1;
	std::stringstream ss2;
	ss2 << std::hex << h2;
	std::stringstream ss3;
	ss3 << std::hex << h3;
	std::stringstream ss4;
	ss4 << std::hex << h4;
	std::stringstream ss5;
	ss5 << std::hex << h5;
	std::stringstream ss6;
	ss6 << std::hex << h6;
	std::stringstream ss7;
	ss7 << std::hex << h7;
	std::string shaHash = ss0.str() + ss1.str() + ss2.str() + ss3.str() + ss4.str() + ss5.str() + ss6.str() + ss7.str();
	return shaHash;
}

std::string SHA256Hash(std::string message)
{
	std::vector<uint8_t> preprocessedMessage = pre_process(message);
	std::vector<std::vector<uint32_t>> scheduledMessage = schedule_message(preprocessedMessage);
	std::string shaHash = compress(scheduledMessage);
	return shaHash;
}

int main(int argc, char* argv[])
{
	std::string message = "";
	std::string argument = "";
	if (argc <= 2)
	{
		std::cerr << "Use -f for files and -h for a given block. Example : -f file.txt or -h message !" << std::endl;
		return 0;
	}
	else
	{
		argument = argv[1];
		if (argument == "-f")
		{
			//Get File Hash
			std::ifstream inputFile;
			inputFile.open(argv[2]);
			std::stringstream strStream;
			strStream << inputFile.rdbuf();
			message = strStream.str();
			inputFile.close();
		}
		else if (argument == "-h")
		{
			//Hash the following message
			for (size_t i = 2; i < argc; i++)
			{
				message = message + argv[i] + " ";
			}
			message.erase(message.size() - 1);
		}
		else
		{
			std::cerr << "Use -f for files and -h for a given block. Example : -f file.txt or -h message !" << std::endl;
			return 0;
		}
	}

	std::cout << "SHA256 : " << SHA256Hash(message) << std::endl;

	return 0;
}