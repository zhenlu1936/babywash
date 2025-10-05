#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace std;

int main() {
	char inst;

	ifstream file("test/bw/initialize.bw");
	if (!file) {
		cerr << "Failed to open file\n";
		return 1;
	}

	string line;
	while (getline(file, line)) {
		cout << line;
	}
	cout << endl;

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
	// cout << endl;
	cout << '$';
	return 0;
}