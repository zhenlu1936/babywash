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

uint8_t raw_mem[UINT16_MAX + 1] = {0};
uint32_t i = 0;

string program;
char* program_input;
FILE* program_file;
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
		cout << i << ": " << (int)gray_to_bin(raw_mem[gray_to_bin(i)]) << "\t";
	cout << "bypass loop: " << bypass_loop << endl;
}

void basic_inst_jit() {
	if (curr_inst == 's') {
		raw_mem[i] ^= 1;
	} else if (curr_inst == 'z') {
		raw_mem[i] ^= 1 << std::min(countr_zero(raw_mem[i]) + 1, 7);
	} else if (curr_inst == 'x') {
		i ^= 1;
	} else if (curr_inst == 'y') {
		i ^= 1 << (countr_zero(i) + 1);
	} else if (curr_inst == 'i') {
		raw_mem[i] = getchar();
	} else if (curr_inst == 'o') {
		// putchar(gray_to_bin(raw_mem[i]));
		putchar(raw_mem[i]);
	} else if (curr_inst == 'u') {
		raw_mem[i] = raw_mem[raw_mem[i]];
	} else if (curr_inst == 'v') {
		raw_mem[i] = raw_mem[raw_mem[i] + 256];
	} else {
		// printf("Unexpected Instruction %c\n", curr_inst);
		// exit(1);
	}
}

void p_jit() {
	if (popcount(raw_mem[i]) & 1) {
		if (next_inst == (uint32_t)program.length()) {
			curr_inst = fgetc(program_file);
			basic_inst_jit();
			program.push_back(curr_inst);
			next_inst++;

			program.push_back(fgetc(program_file));
			next_inst++;
		} else {
			curr_inst = program[next_inst++];
			basic_inst_jit();
			next_inst++;
		}
	} else {
		if (next_inst == (uint32_t)program.length()) {
			program.push_back(fgetc(program_file));
			next_inst++;

			curr_inst = fgetc(program_file);
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
			curr_inst = fgetc(program_file);
			basic_inst_jit();
			program.push_back(curr_inst);
			next_inst++;

			program.push_back(fgetc(program_file));
			next_inst++;
		} else {
			curr_inst = program[next_inst++];
			basic_inst_jit();
			next_inst++;
		}
	} else {
		if (next_inst == (uint32_t)program.length()) {
			program.push_back(fgetc(program_file));
			next_inst++;

			curr_inst = fgetc(program_file);
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
	if (!raw_mem[i]) {
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
		if (raw_mem[i]) {
			next_inst = loop_begin.top().BEGIN;
			loop_begin.top().END = next_inst;
		} else {
			loop_begin.pop();
		}
	}
}

int main(int argc, char* argv[]) {
	if (argc < 3 || argc > 4) {
		printf("usage: COMPILER MEMOUT_FILE <OPTIONAL>PROGRAM_INPUT\n");
		exit(1);
	}

	const char* memout_file = argv[1];

	program_input = argv[2];
	program_file = fopen(program_input, "rb");

	while (new_inst != EOF && new_inst != '$') {
		if (next_inst == (uint32_t)program.length()) {
			new_inst = fgetc(program_file);
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
	fgetc(program_file);  // zhenlu: get rid of \n

	std::ofstream out(memout_file, std::ios::binary);
	if (!out) {
		std::cerr << "Failed to open file\n";
		return 1;
	}
	out.write(reinterpret_cast<char*>(raw_mem), sizeof(raw_mem));
	out.close();

	return 0;
}