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

#define INTEPRETER_MODE 0
#define JIT_MODE 1

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

stack<pair<uint32_t, uint32_t>> loop_begin;

int bypass_loop = 0;

char new_inst = 0;
char curr_inst = 0;
uint32_t next_inst = 0;

uint8_t gray_to_bin(uint8_t n) {
	int res = n;
	while (n > 0) {
		n >>= 1;
		res ^= n;
	}
	return res;
}

void mem_lookup() {
	cout << "ptr: " << (int)gray_to_bin(i) << "\t";
	for (int i = 0; i < 10; i++)
		cout << i << ": " << (int)gray_to_bin(mem[gray_to_bin(i)]) << "\t";
	cout << "bypass loop: " << bypass_loop << endl;
}

void basic_inst_jit() {
	if (curr_inst == 's') {
		mem[i] ^= 1;
	} else if (curr_inst == 'z') {
		mem[i] ^= 1 << std::min(countr_zero(mem[i]) + 1, 7);
	} else if (curr_inst == 'x') {
		i ^= 1;
	} else if (curr_inst == 'y') {
		i ^= 1 << (countr_zero(i) + 1);
	} else if (curr_inst == 'i') {
		mem[i] = getchar();
	} else if (curr_inst == 'o') {
		putchar(gray_to_bin(mem[i]));
	} else if (curr_inst == 'u') {
		mem[i] = mem[mem[i]];
	} else if (curr_inst == 'v') {
		mem[i] = mem[mem[i] + 256];
	} else {
		printf("Unexpected Instruction!\n");
		exit(1);
	}
}

void p_jit() {
	if (popcount(mem[i]) & 1) {
		if (next_inst == (uint32_t)program.length()) {
			curr_inst = getchar();
			basic_inst_jit();
			program.push_back(curr_inst);
			next_inst++;

			program.push_back(getchar());
			next_inst++;
		} else {
			curr_inst = program[next_inst++];
			basic_inst_jit();
			next_inst++;
		}
	} else {
		if (next_inst == (uint32_t)program.length()) {
			program.push_back(getchar());
			next_inst++;

			curr_inst = getchar();
			basic_inst_jit();
			program.push_back(curr_inst);
			next_inst++;
		} else {
			next_inst++;
			curr_inst = program[next_inst++];
			basic_inst_jit();
		}
	}
}

void q_jit() {
	if (popcount(i) & 1) {
		if (next_inst == (uint32_t)program.length()) {
			curr_inst = getchar();
			basic_inst_jit();
			program.push_back(curr_inst);
			next_inst++;

			program.push_back(getchar());
			next_inst++;
		} else {
			curr_inst = program[next_inst++];
			basic_inst_jit();
			next_inst++;
		}
	} else {
		if (next_inst == (uint32_t)program.length()) {
			program.push_back(getchar());
			next_inst++;

			curr_inst = getchar();
			basic_inst_jit();
			program.push_back(curr_inst);
			next_inst++;
		} else {
			next_inst++;
			curr_inst = program[next_inst++];
			basic_inst_jit();
		}
	}
}

void loop_begin_jit() {
	if (!mem[i]) {
		if (next_inst == (uint32_t)program.length()) {
			bypass_loop++;
		} else {
			next_inst = loop_begin.top().END;
			loop_begin.pop();
		}
	} else {
		loop_begin.push(make_pair(next_inst, INT_MIN));
	}
}

void loop_end_jit() {
	if (bypass_loop) {
		bypass_loop--;
	} else {
		if (mem[i]) {
			next_inst = loop_begin.top().BEGIN;
			loop_begin.top().END = next_inst;
		} else {
			loop_begin.pop();
		}
	}
}

int main(int argc, char* argv[]) {
	const char* memout_file = argv[1];

	while (curr_inst != EOF) {
		if (next_inst == (uint32_t)program.length()) {
			new_inst = getchar();
			program.push_back(new_inst);
			curr_inst = new_inst;
		} else
			curr_inst = program[next_inst];

		next_inst++;

		switch (curr_inst) {
			case 's':
			case 'z':
			case 'x':
			case 'y':
			case 'i':
			case 'o':
			case 'u':
			case 'v':
				if (!bypass_loop) basic_inst_jit();
				break;

			case 'p':
				if (!bypass_loop) p_jit();
				break;

			case 'q':
				if (!bypass_loop) q_jit();
				break;

			case '[':
				loop_begin_jit();
				break;

			case ']':
				loop_end_jit();
				break;

			default:
				break;
		}
	}

	std::ofstream out(memout_file, std::ios::binary);
	if (!out) {
		std::cerr << "Failed to open file\n";
		return 1;
	}
	out.write(reinterpret_cast<char*>(mem), sizeof(mem));
	out.close();

	return 0;
}