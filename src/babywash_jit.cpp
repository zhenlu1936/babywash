#include <stdio.h>
#include <sys/mman.h>

#include <algorithm>
#include <bit>
#include <bitset>
#include <climits>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stack>
#include <vector>

// #define BEGIN first
// #define END second
#define CODE_SIZE (sizeof(uint32_t) * (UINT16_MAX + 1))

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

stack<uint32_t> loop_begin;

int bypass_loop = 0;

char new_inst = 0;

std::vector<uint32_t> cur_buffer;
// std::vector<uint32_t> single_inst_length;

uint32_t code_length = 0;
uint32_t code_buffer[UINT16_MAX + 1] = {0};

static inline uint32_t to_be32(uint32_t x) {
	return ((x & 0x000000FF) << 24) | ((x & 0x0000FF00) << 8) |
	       ((x & 0x00FF0000) >> 8) | ((x & 0xFF000000) >> 24);
}

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

void push_code() {
	// single_inst_length.push_back(cur_buffer.size());
	for (auto code : cur_buffer) {
		code_buffer[code_length++] = code;
	}
	cur_buffer.clear();
}

// x2: base pointer
// x3: index
// x4: temporary
// w0: return value
// w1: mem[index]
// w5: temporary
// w6: temporary
uint32_t encode_mov(uint8_t rd, uint16_t imm16, uint8_t shift, bool keep) {
	// keep = false → MOVZ, keep = true → MOVK
	uint32_t opcode = keep ? 0xF2800000 : 0xD2800000;
	// shift: 0 = LSL 0, 1 = LSL 16, 2 = LSL 32, 3 = LSL 48
	return opcode | ((imm16 & 0xFFFF) << 5) | (rd & 0x1F) | (shift << 21);
}

void initialize_jit() {
	cur_buffer.push_back(
		encode_mov(2, (uint64_t)raw_mem & 0xFFFF, 0, false));  // MOVZ X2, imm16
	cur_buffer.push_back(encode_mov(2, ((uint64_t)raw_mem >> 16) & 0xFFFF, 1,
	                                true));  // MOVK X2, imm16, LSL16
	cur_buffer.push_back(encode_mov(2, ((uint64_t)raw_mem >> 32) & 0xFFFF, 2,
	                                true));  // MOVK X2, imm16, LSL32
	cur_buffer.push_back(encode_mov(2, ((uint64_t)raw_mem >> 48) & 0xFFFF, 3,
	                                true));     // MOVK X2, imm16, LSL48
	cur_buffer.push_back(to_be32(0x030080D2));  // MOV X3, #0
	push_code();
}

int basic_inst_jit() {
	if (new_inst == 's') {
		// mem[i] ^= 1;
		cur_buffer.push_back(to_be32(0x41686338));  // LDRB w1, [x2, x3]
		cur_buffer.push_back(to_be32(0x21000052));  // EOR  w1, w1, #1
		cur_buffer.push_back(to_be32(0x41682338));  // STRB w1, [x2, x3]
	} else if (new_inst == 'z') {
		// mem[i] ^= 1 << std::min(countr_zero(mem[i]) + 1, 7);
		cur_buffer.push_back(to_be32(0x41686338));  // LDRB w1, [x2, x3]
		cur_buffer.push_back(to_be32(0x2500C05A));  // RBIT w5, w1
		cur_buffer.push_back(to_be32(0xA510C05A));  // CLZ  w5, w5
		cur_buffer.push_back(to_be32(0xA5040011));  // ADD  w5, w5, #1
		cur_buffer.push_back(to_be32(0xE6008052));  // MOV  w6, #7
		cur_buffer.push_back(to_be32(0xBF00066B));  // CMP  w5, w6
		cur_buffer.push_back(to_be32(0xA5D0861A));  // CSEL w5, w5, w6, LE
		cur_buffer.push_back(to_be32(0x26008052));  // MOV  w6, #1
		cur_buffer.push_back(to_be32(0xC620C51A));  // LSL  w6, w6, w5
		cur_buffer.push_back(to_be32(0x2100064A));  // EOR  w1, w1, w6
		cur_buffer.push_back(to_be32(0x41682338));  // STRB w1, [x2, x3]
	} else if (new_inst == 'x') {
		// i ^= 1;
		cur_buffer.push_back(to_be32(0x630040D2));  // EOR x3, x3, #1
	} else if (new_inst == 'y') {
		// i ^= 1 << (countr_zero(i) + 1);
		cur_buffer.push_back(to_be32(0x6400C0DA));  // RBIT x4, x3
		cur_buffer.push_back(to_be32(0x8410C0DA));  // CLZ  x4, x4
		cur_buffer.push_back(to_be32(0x84040091));  // ADD  x4, x4, #1
		cur_buffer.push_back(to_be32(0x250080D2));  // MOV  x5, #1
		cur_buffer.push_back(to_be32(0xA420C49A));  // LSL  x4, x5, x4
		cur_buffer.push_back(to_be32(0x630004CA));  // EOR  x3, x3, x4
	} else if (new_inst == 'i') {
		// mem[i] = getchar();
		cur_buffer.push_back(to_be32(0xE20FBFA9));  // STP x2, x3, [SP, #-16]!
		cur_buffer.push_back(
			to_be32(0xFE7FBFA9));  // STP x30,xzr,[SP,#-16]! // zhenlu: LR,
		                           // address for returning

		uint64_t addr = (uint64_t)(uintptr_t)getchar;
		// cout << (void*)getchar << endl;

		// Use X16 as temporary register
		cur_buffer.push_back(
			encode_mov(16, addr & 0xFFFF, 0, false));  // MOVZ X16, imm16
		cur_buffer.push_back(encode_mov(16, (addr >> 16) & 0xFFFF, 1,
		                                true));  // MOVK X16, LSL16
		cur_buffer.push_back(encode_mov(16, (addr >> 32) & 0xFFFF, 2,
		                                true));  // MOVK X16, LSL32
		cur_buffer.push_back(encode_mov(16, (addr >> 48) & 0xFFFF, 3,
		                                true));  // MOVK X16, LSL48

		// BLR X16
		cur_buffer.push_back(to_be32(0x00023FD6));

		cur_buffer.push_back(to_be32(0xFE7FC1A8));  //  LDP x30,xzr,[SP],#16
		cur_buffer.push_back(to_be32(0xE20FC1A8));  //  LDP x2, x3, [SP], #16

		cur_buffer.push_back(to_be32(0x40682338));  // STRB w0, [x2, x3]

		// bin_to_gray 
		// cur_buffer.push_back(to_be32(0x057C0153));  // LSR  w5, w0, #1
		// cur_buffer.push_back(to_be32(0x0000054A));  // EOR  w0, w0, w5
		// cur_buffer.push_back(to_be32(0x40682338));  // STRB w0, [x2, x3]
	} else if (new_inst == 'o') {
		// putchar(gray_to_bin(mem[i]));
		cur_buffer.push_back(to_be32(0x41686338));  // LDRB w1, [x2, x3]
		cur_buffer.push_back(to_be32(0xE503012A));  // MOV  w5, w1

		// gray_to_bin 
		// cur_buffer.push_back(to_be32(0xA504454A));  // EOR  w5, w5, w5, LS #1
		// cur_buffer.push_back(to_be32(0xA508454A));  // EOR  w5, w5, w5, LSR #2
		// cur_buffer.push_back(to_be32(0xA510454A));  // EOR  w5, w5, w5, LSR #4

		cur_buffer.push_back(to_be32(0xE003052A));  // MOV  w0, w5
		push_code();

		cur_buffer.push_back(to_be32(0xE20FBFA9));  // STP x2, x3, [SP, #-16]!
		cur_buffer.push_back(
			to_be32(0xFE7FBFA9));  // STP x30,xzr,[SP,#-16]! // zhenlu: LR,
		                           // address for returning

		uint64_t addr = (uint64_t)(uintptr_t)putchar;
		// cout << (void*)putchar << endl;

		// Use X16 as temporary register
		cur_buffer.push_back(
			encode_mov(16, addr & 0xFFFF, 0, false));  // MOVZ X16, imm16
		cur_buffer.push_back(encode_mov(16, (addr >> 16) & 0xFFFF, 1,
		                                true));  // MOVK X16, LSL16
		cur_buffer.push_back(encode_mov(16, (addr >> 32) & 0xFFFF, 2,
		                                true));  // MOVK X16, LSL32
		cur_buffer.push_back(encode_mov(16, (addr >> 48) & 0xFFFF, 3,
		                                true));  // MOVK X16, LSL48

		// BLR X16
		cur_buffer.push_back(to_be32(0x00023FD6));

		cur_buffer.push_back(to_be32(0xFE7FC1A8));  //  LDP x30,xzr,[SP],#16
		cur_buffer.push_back(to_be32(0xE20FC1A8));  //  LDP x2, x3, [SP], #16
	} else if (new_inst == 'u') {
		// mem[i] = mem[mem[i]];
		cur_buffer.push_back(to_be32(0x41686338));  // LDRB w1, [x2, x3]
		cur_buffer.push_back(to_be32(0x4440218B));  // ADD  x4, x2, w1, UXTW
		cur_buffer.push_back(to_be32(0x85004039));  // LDRB w5, [x4]
		cur_buffer.push_back(to_be32(0x45682338));  // STRB w5, [x2, x3]
	} else if (new_inst == 'v') {
		// mem[i] = mem[mem[i] + 256];
		cur_buffer.push_back(to_be32(0x41686338));  // LDRB w1, [x2, x3]
		cur_buffer.push_back(to_be32(0x4440218B));  // ADD  x4, x2, w1, UXTW
		cur_buffer.push_back(to_be32(0x84000491));  // ADD  x4, x4, #256
		cur_buffer.push_back(to_be32(0x85004039));  // LDRB w5, [x4]
		cur_buffer.push_back(to_be32(0x45682338));  // STRB w5, [x2, x3]
	} else {
		printf("Unexpected Instruction!\n");
		exit(1);
	}
	int length = cur_buffer.size();
	push_code();
	return length;
}

void p_jit() {
	cur_buffer.push_back(to_be32(0x41686338));  // LDRB w1, [x2, x3]
	cur_buffer.push_back(to_be32(0x201C014E));  // INS V0.B[0], W1
	cur_buffer.push_back(to_be32(0x0158204E));  // CNT v1.16B, v0.16B
	cur_buffer.push_back(to_be32(0x243C010E));  // UMOV W4, v1.B[0]
	cur_buffer.push_back(to_be32(0x85000012));  // AND  w5, w4, #1
	cur_buffer.push_back(to_be32(0x05000034));  // CBZ  w5, do_b
	push_code();

	// do A
	new_inst = fgetc(program_file);
	program.push_back(new_inst);
	int length_a = basic_inst_jit();
	code_buffer[code_length - length_a - 1] |= (length_a + 2) << 5;
	cur_buffer.push_back(to_be32(0x00000014));  // B end_if
	push_code();

	// do B
	new_inst = fgetc(program_file);
	program.push_back(new_inst);
	int length_b = basic_inst_jit();
	code_buffer[code_length - length_b - 1] |= length_b + 1;
	// end_if
}

void q_jit() {
	cur_buffer.push_back(to_be32(0x601C024E));  // MOV V0.H[0], W3
	cur_buffer.push_back(to_be32(0x0158200E));  // CNT V1.8B, V0.8B
	cur_buffer.push_back(to_be32(0x263C010E));  // UMOV W6, V1.B[0]
	cur_buffer.push_back(to_be32(0x273C030E));  // UMOV W7, V1.B[1]
	cur_buffer.push_back(to_be32(0xC400070B));  // ADD W4, W6, W7

	cur_buffer.push_back(to_be32(0x85000012));  // AND w5, w4, #1
	cur_buffer.push_back(to_be32(0x05000034));  // CBZ w5, do_b
	push_code();

	// do A
	new_inst = fgetc(program_file);
	program.push_back(new_inst);
	int length_a = basic_inst_jit();
	code_buffer[code_length - length_a - 1] |= (length_a + 2) << 5;
	cur_buffer.push_back(to_be32(0x00000014));  // B end_if
	push_code();

	// do B
	new_inst = fgetc(program_file);
	program.push_back(new_inst);
	int length_b = basic_inst_jit();
	code_buffer[code_length - length_b - 1] |= length_b + 1;
	// end_if
}

void loop_begin_jit() {
	cur_buffer.push_back(to_be32(0x41686338));  // LDRB w1, [x2, x3]
	cur_buffer.push_back(to_be32(0x01000034));  // CBZ w1, 0 => tbd
	push_code();
	loop_begin.push(code_length);
}

void loop_end_jit() {
	cur_buffer.push_back(to_be32(0x41686338));  // LDRB w1, [x2, x3]
	cur_buffer.push_back(to_be32(0x01000035));  // CBNZ w1, 0 => tbd
	push_code();

	code_buffer[loop_begin.top() - 1] |=
		((code_length - loop_begin.top() + 1) << 5) & 0x00FFFFFF;
	code_buffer[code_length - 1] |=
		((loop_begin.top() - code_length + 1) << 5) & 0x00FFFFFF;
	loop_begin.pop();
}

uint32_t* alloc_executable(size_t size) {
	void* buf = mmap(nullptr, size, PROT_READ | PROT_WRITE,
	                 MAP_PRIVATE | MAP_ANON | MAP_JIT, -1, 0);

	if (buf == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}

	return reinterpret_cast<uint32_t*>(buf);
}

void prepare_jit() {
	cur_buffer.push_back(to_be32(0xC0035FD6));
	push_code();
}

void jit_run() {
	uint32_t* code_executable = alloc_executable(CODE_SIZE);

	memcpy(code_executable, code_buffer, CODE_SIZE);
	__builtin___clear_cache((char*)code_executable,
	                        (char*)code_executable + CODE_SIZE);
	mprotect(code_executable, CODE_SIZE, PROT_READ | PROT_EXEC);

	void (*execute_code)() = (void (*)())code_executable;

	execute_code();
}

int main(int argc, char* argv[]) {
	if (argc < 3 || argc > 4) {
		printf("usage: COMPILER MEMOUT_FILE <OPTIONAL>PROGRAM_INPUT\n");
		exit(1);
	}

	const char* memout_file = argv[1];

	program_input = argv[2];
	program_file = fopen(program_input, "rb");

	initialize_jit();

	while (new_inst != EOF && new_inst != '$') {
		new_inst = fgetc(program_file);
		program.push_back(new_inst);

		switch (new_inst) {
			case 's':
			case 'z':
			case 'x':
			case 'y':
			case 'i':
			case 'o':
			case 'u':
			case 'v':
				basic_inst_jit();
				break;

			case 'p':
				p_jit();
				break;

			case 'q':
				q_jit();
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

	prepare_jit();
	jit_run();

	std::ofstream out(memout_file, std::ios::binary);
	if (!out) {
		std::cerr << "Failed to open file\n";
		return 1;
	}
	out.write(reinterpret_cast<char*>(raw_mem), sizeof(raw_mem));
	out.close();

	return 0;
}