#include <stdio.h>

#include <algorithm>
#include <bit>
#include <bitset>
#include <climits>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stack>

#define BEGIN first
#define END second

using std::countr_zero;
using std::cout;
using std::endl;
using std::make_pair;
using std::pair;
using std::popcount;
using std::stack;
using std::string;

uint8_t mem[UINT16_MAX + 1] = {0};
uint32_t i = 0;

string program;
string basic_inst = "szxyiouv";

stack<pair<uint32_t, uint32_t>> loop_bound;

int bypass_loop = 0;

uint8_t gray_to_bin(uint8_t n) {
	int res = n;
	while (n > 0) {
		n >>= 1;
		res ^= n;
	}
	return res;
}

void basic_inst_execution(char inst) {
	if (inst == 's') {
		mem[i] ^= 1;
	} else if (inst == 'z') {
		mem[i] ^= 1 << std::min(countr_zero(mem[i]) + 1, 7);
	} else if (inst == 'x') {
		i ^= 1;
	} else if (inst == 'y') {
		i ^= 1 << (countr_zero(i) + 1);
	} else if (inst == 'i') {
		mem[i] = getchar();
	} else if (inst == 'o') {
		putchar(gray_to_bin(mem[i]));
	} else if (inst == 'u') {
		mem[i] = mem[mem[i]];
	} else if (inst == 'v') {
		mem[i] = mem[mem[i] + 256];
	} else {
		printf("Unexpected Instruction!\n");
		exit(1);
	}

	// cout << "ptr: " << (int)gray_to_bin(i) << "\t";
	// for (int i = 0; i < 10; i++)
	// 	cout << i << ": " << (int)gray_to_bin(mem[gray_to_bin(i)]) << "\t";
	// cout << "bypass loop: " << bypass_loop << endl;
}

int main(int argc, char* argv[]) {
	char new_inst = 0;
	char cur_inst = 0;
	uint32_t next_inst = 0;

	while (cur_inst != EOF) {
		if (next_inst == (uint32_t)program.length()) {
			new_inst = getchar();
			program.push_back(new_inst);
			cur_inst = new_inst;
		} else
			cur_inst = program[next_inst];

		next_inst++;

		switch (cur_inst) {
			case 's':
			case 'z':
			case 'x':
			case 'y':
			case 'i':
			case 'o':
			case 'u':
			case 'v':
				if (!bypass_loop) basic_inst_execution(cur_inst);
				break;

			case 'p':
				if (!bypass_loop) {
					if (popcount(mem[i]) & 1) {
						if (next_inst == (uint32_t)program.length()) {
							basic_inst_execution(cur_inst = getchar());
							program.push_back(cur_inst);
							next_inst++;

							program.push_back(getchar());
							next_inst++;
						} else {
							basic_inst_execution(program[next_inst++]);
							next_inst++;
						}
					} else {
						if (next_inst == (uint32_t)program.length()) {
							program.push_back(getchar());
							next_inst++;

							basic_inst_execution(cur_inst = getchar());
							program.push_back(cur_inst);
							next_inst++;
						} else {
							next_inst++;
							basic_inst_execution(program[next_inst++]);
						}
					}
				}
				break;

			case 'q':
				if (!bypass_loop) {
					if (popcount(i) & 1) {
						if (next_inst == (uint32_t)program.length()) {
							basic_inst_execution(cur_inst = getchar());
							program.push_back(cur_inst);
							next_inst++;

							program.push_back(getchar());
							next_inst++;
						} else {
							basic_inst_execution(program[next_inst++]);
							next_inst++;
						}
					} else {
						if (next_inst == (uint32_t)program.length()) {
							program.push_back(getchar());
							next_inst++;

							basic_inst_execution(cur_inst = getchar());
							program.push_back(cur_inst);
							next_inst++;
						} else {
							next_inst++;
							basic_inst_execution(program[next_inst++]);
						}
					}
				}
				break;

			case '[':
				if (!mem[i]) {
					if (next_inst == (uint32_t)program.length()) {
						bypass_loop++;
					} else {
						next_inst = loop_bound.top().END;
						loop_bound.pop();
					}
				} else {
					loop_bound.push(make_pair(next_inst, INT_MIN));
				}
				break;

			case ']':
				if (bypass_loop) {
					bypass_loop--;
				} else {
					if (mem[i]) {
						next_inst = loop_bound.top().BEGIN;
						loop_bound.top().END = next_inst;
					} else {
						loop_bound.pop();
					}
				}
				break;

			default:
				break;
		}
	}

	std::ofstream out(argv[1], std::ios::binary);
	if (!out) {
		std::cerr << "Failed to open file\n";
		return 1;
	}
	out.write(reinterpret_cast<char*>(mem), sizeof(mem));
	out.close();

	return 0;
}