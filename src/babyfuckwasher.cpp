#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace std;

int main() {
	char inst;
	// ofstream cout("wash_program.txt");
	// if (!cout) {
	// 	cerr << "Failed to open wash_program.txt\n";
	// 	return 1;
	// }

	while ((inst = getchar()) != EOF) {
		switch (inst) {
			case '<':
				cout << "qxy";
				break;
			case '>':
				cout << "qyx";
				break;
			case '+':
				cout << "pzs";
				break;
			case '-':
				cout << "psz";
				break;
			case '.':
				cout << "o";
				break;
			case ',':
				cout << "i";
				break;
			case '[':
				cout << "[";
				break;
			case ']':
				cout << "]";
				break;
			default:
				break;
		}
	}

	// cout.close();
	return 0;
}