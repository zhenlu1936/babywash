#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace std;

int bin_to_gray(int n) { return n ^ (n >> 1); }

int main() {
	for (int i = 0; i <= 255; i++) {
		cout << (char)bin_to_gray(i);
	}
    // for (int i = 0; i <= 255; i++) {
	// 	cout << (char)i;
	// }
    for (int i = 255; i >= 0; i--) {
		cout << (char)i;
	}
    cout << "1145141919810";

	cout << '$';
	return 0;
}