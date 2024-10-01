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

static int check_argument(const int argc, char* argv[], int& index, const std::string& option)
{
	if (strcmp(argv[index], ("-" + option).c_str()) == 0) {
		if (++index >= argc) {
			std::cout << "Error: Missing parameter for \"-" << option << "\"\n";
			return -1;
		}
		return 1;
	}
	return 0;
}

static bool string_starts_with(const std::string& str, const std::string& prefix)
{
	return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

static bool string_ends_with(const std::string& str, const std::string& suffix)
{
	return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static std::string string_to_upper(const std::string& str)
{
	std::string lower_str = str;
	std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), [](unsigned char c) { return std::toupper(c); });
	return lower_str;
}

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
	std::vector<symbol>      symbols;
	std::string              input_file = "";
	std::string              output_file = "";
	std::vector<std::string> prefixes;
	std::vector<std::string> suffixes;
	
	if (argc < 2) {
		std::cout << "Usage: extract-psyq-symbols -i [input] -o [output] <-p [prefix]> <-s [suffix]>\n\n"
		             "[input]       - Input symbol file\n"
		             "[output]      - Output file\n"
		             "<-p [prefix]> - Add prefix to search for\n"
		             "<-s [suffix]> - Add suffix to search for\n";
		return -1;
	}

	for (int i = 1; i < argc; i++) {
		int success;
		if ((success = check_argument(argc, argv, i, "i")) < 0) {
			return -1;
		} else if (success > 0) {
			if (!input_file.empty()) {
				std::cout << "Error: Input symbol file already defined.\n";
				return -1;
			}
			input_file = argv[i];
			continue;
		}

		if ((success = check_argument(argc, argv, i, "o")) < 0) {
			return -1;
		} else if (success > 0) {
			if (!output_file.empty()) {
				std::cout << "Error: Output file already defined.\n";
				return -1;
			}
			output_file = argv[i];
			continue;
		}

		if ((success = check_argument(argc, argv, i, "p")) < 0) {
			return -1;
		} else if (success > 0) {
			prefixes.push_back(string_to_upper(argv[i]));
			continue;
		}

		if ((success = check_argument(argc, argv, i, "s")) < 0) {
			return -1;
		} else if (success > 0) {
			suffixes.push_back(string_to_upper(argv[i]));
			continue;
		}

		std::cout << "Error: Unknown argument \"" << argv[i] << "\".\n";
		return -1;
	}

	if (input_file.empty()) {
		std::cout << "Error: Input symbol file not defined.\n";
		return -1;
	}
	if (output_file.empty()) {
		std::cout << "Error: Output symbol file not defined.\n";
		return -1;
	}

	std::ifstream input(input_file, std::ios::in | std::ios::binary);
	if (!input.is_open()) {
		std::cout << "Error: Cannot open \"" << input_file << "\" for reading.\n";
		return -1;
	}

	std::ofstream output(output_file, std::ios::out);
	if (!output.is_open()) {
		std::cout << "Error: Cannot open \"" << output_file << "\" for writing.\n";
		return -1;
	}

	char read_buffer[4];
	if (!read_input(input, read_buffer, 3)) {
		return -1;
	}
	read_buffer[3] = '\0';

	if (strcmp(read_buffer, "MND")) {
		std::cout << "Error: \"" << input_file << "\" is not a value Psy-Q symbol file.\n";
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

		char symbol_type;
		if (!read_input(input, &symbol_type, 1)) {
			return -1;
		}

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
		name_buffer[value_buffer] = '\0';

		std::string name = string_to_upper(name_buffer);
		delete[] name_buffer;

		bool add = symbol_type == 2 && prefixes.empty() && suffixes.empty();
		if (!prefixes.empty()) {
			for (const auto& prefix : prefixes) {
				if (string_starts_with(name, prefix)) {
					add = true;
					break;
				}
			}
		}
		if (!suffixes.empty()) {
			for (const auto& suffix : suffixes) {
				if (string_ends_with(name, suffix)) {
					add = true;
					break;
				}
			}
		}

		if (add) {
			symbols.push_back({ name, value });
		}
	}

	if ((line_length & 7) != 0) {
		line_length &= ~7;
	}
	line_length += 8;

	std::sort(symbols.begin(), symbols.end(), compare_symbols);

	output << "; ------------------------------------------------------------------------------\n"
	          "; Symbols extracted from\n; " << input_file << "\n"
	          "; ------------------------------------------------------------------------------\n\n";

	for (auto& symbol : symbols) {
		output << std::left << std::setw(line_length) << symbol.name << "equ ";
		if (symbol.value >= 10) {
			output << "$";
		}
		output << std::hex << std::uppercase << symbol.value << "\n";
	}

	output << "\n; ------------------------------------------------------------------------------\n";
	return 0;
}