/*
	Copyright(c) 2024 Devon Artmeier

	Permission to use, copy, modify, and /or distribute this software
	for any purpose with or without fee is hereby granted.

	THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
	WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIE
	WARRANTIES OF MERCHANTABILITY AND FITNESS.IN NO EVENT SHALL THE
	AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
	DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
	PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.
*/

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

struct symbol {
	std::string   name;
	unsigned long value;
};

static bool compare_symbols(const symbol symbol_1, const symbol symbol_2)
{
	return symbol_1.value < symbol_2.value;
}

static bool read_input(std::ifstream& input, char* const read_buffer, const std::streamsize read_count)
{
	if (read_buffer == nullptr) {
		return false;
	}

	input.read(read_buffer, read_count);
	std::streamsize actual_read_count = input.gcount();

	if (actual_read_count == 0) {
		std::cout << "Error: Reached end of symbol file prematurely.\n";
		return false;
	} else if (actual_read_count < 0) {
		std::cout << "Error: Failed to read from symbol file.\n";
		return false;
	}
	return true;
}

int main(int argc, char* argv[])
{
	std::vector<symbol> symbols;
	
	if (argc < 3) {
		std::cout << "Usage: extract-psyq-symbols [input] [output]\n";
		return -1;
	}

	std::ifstream input(argv[1], std::ios::in | std::ios::binary);
	if (!input.is_open()) {
		std::cout << "Error: Cannot open \"" << argv[1] << "\" for reading.\n";
		return -1;
	}

	std::ofstream output(argv[2], std::ios::out);
	if (!output.is_open()) {
		std::cout << "Error: Cannot open \"" << argv[2] << "\" for writing.\n";
		return -1;
	}

	char read_buffer[4];
	if (!read_input(input, read_buffer, 3)) {
		return -1;
	}
	read_buffer[3] = '\0';

	if (strcmp(read_buffer, "MND")) {
		std::cout << "Error: \"" << argv[1] << "\" is not a value PSY-Q symbol file.\n";
		return -1;
	}

	unsigned int line_length = 0;

	input.seekg(8, std::ios_base::beg);
	while (true) {
		if (input.peek() == -1) {
			break;
		}

		unsigned long value = 0;
		unsigned char value_buffer;
		for (int i = 0; i < 4; i++) {
			if (!read_input(input, reinterpret_cast<char*>(&value_buffer), 1)) {
				return -1;
			}
			value |= value_buffer << (i << 3);
		}

		input.seekg(1, std::ios_base::cur);
		if (!read_input(input, reinterpret_cast<char*>(&value_buffer), 1)) {
			return -1;
		}
		if (value_buffer > line_length) {
			line_length = value_buffer;
		}

		char* name_buffer = new char[value_buffer + 1];
		if (!read_input(input, name_buffer, value_buffer)) {
			return -1;
		}
		for (int i = 0; i < value_buffer; i++) {
			name_buffer[i] = std::toupper(name_buffer[i]);
		}
		name_buffer[value_buffer] = '\0';

		symbols.push_back({ name_buffer, value });
		delete[] name_buffer;
	}

	if ((line_length & 7) != 0) {
		line_length &= ~7;
	}
	line_length += 8;

	std::sort(symbols.begin(), symbols.end(), compare_symbols);

	output << "; ------------------------------------------------------------------------------\n"
		      "; Symbols extracted from\n; " << argv[1] << "\n"
		      "; ------------------------------------------------------------------------------\n\n";

	for (auto& symbol : symbols) {
		output << std::left << std::setw(line_length) << symbol.name << "equ ";
		if (symbol.value >= 10) {
			output << "$";
		}
		output << std::hex << std::uppercase << symbol.value << "\n";
	}

	output << "\n; ------------------------------------------------------------------------------\n";
}