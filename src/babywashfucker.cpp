#include <fstream>
#include <iostream>

using namespace std;

uint8_t gray_to_bin(uint8_t n) {
	int res = n;
	while (n > 0) {
		n >>= 1;
		res ^= n;
	}
	return res;
}

int main(int argc, char* argv[]) {
	ifstream infile(argv[1]);
	ofstream outfile(argv[2]);
	if (!infile) {
		cerr << "Failed to open input file\n";
		return 1;
	}

	uint8_t gray;
	while (infile.read(reinterpret_cast<char*>(&gray), 1)) {  // read 1 byte
		uint8_t binary = gray_to_bin(gray);
		outfile.write(reinterpret_cast<char*>(&binary), 1);  // write 1 byte
	}

	infile.close();
	outfile.close();
	return 0;
}