#include <cstdlib>
#include <fstream>
#include <iostream>
#include <cstring>

using namespace std;

uint8_t raw_mem[65536] = {0};
uint8_t raw_mem2[65536] = {0};
uint8_t mem[65536] = {0};

uint16_t bin_to_gray(uint16_t n) { return n ^ (n >> 1); }

uint16_t gray_to_bin(uint16_t n) {
	uint16_t res = n;
	while (n > 0) {
		n >>= 1;
		res ^= n;
	}
	return res;
}

int main() {
	for (int i = 0; i <= 255; i++) {
		raw_mem[i] = bin_to_gray(i);
	}
	for (int i = 256; i <= 511; i++) {
		raw_mem[i] = gray_to_bin(i - 256);
	}
	for (int i = 0; i <= 511; i++) {
		mem[i] = raw_mem[bin_to_gray(i)];
	}

	// mem[513] = 0;
	strcpy(reinterpret_cast<char *>(mem + 513), "Hello, World!\n");

	std::ofstream out("build/initialize.bin", std::ios::binary);
	if (!out) {
		std::cerr << "Failed to open file\n";
		return 1;
	}

	for (int i = 0; i <= 600; i++) {
		out.write(reinterpret_cast<const char *>(&mem[i]), sizeof(mem[i]));
	}

	out.close();

	return 0;
}