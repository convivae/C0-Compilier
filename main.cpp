#include "argparse.hpp"
#include "fmt/core.h"

#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"
#include "fmts.hpp"

#include "vm/vm.h"
#include "vm/file.h"
#include "vm/exception.h"
#include "vm/util/print.hpp"

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <exception>

/*VM 相关*/

void disassemble_binary(std::ifstream* in, std::ostream* out) {
	try {
		File f = File::parse_file_binary(*in);
		f.output_text(*out);
	}
	catch (const std::exception & e) {
		println(std::cerr, e.what());
	}
}

void assemble_text(std::ifstream* in, std::ofstream* out, bool run = false) {
	try {
		File f = File::parse_file_text(*in);
		// f.output_text(std::cout);
		f.output_binary(*out);
		if (run) {
			auto avm = std::move(vm::VM::make_vm(f));
			avm->start();
		}
	}
	catch (const std::exception & e) {
		println(std::cerr, e.what());
	}
}

void execute(std::ifstream* in, std::ostream* out) {
	try {
		File f = File::parse_file_binary(*in);
		auto avm = std::move(vm::VM::make_vm(f));
		avm->start();
	}
	catch (const std::exception & e) {
		println(std::cerr, e.what());
	}
}

/*VM 相关*/

std::vector<cc0::Token> _tokenize(std::istream& input) {
	cc0::Tokenizer tkz(input);
	auto p = tkz.AllTokens();
	if (p.second.has_value()) {
		fmt::print(stderr, "Tokenization error: {}\n", p.second.value());
		exit(2);
	}
	return p.first;
}

void Tokenize(std::istream& input, std::ostream& output) {
	auto v = _tokenize(input);
	for (auto& it : v)
		output << fmt::format("{}\n", it);
	return;
}

void Analyse(std::istream& input, std::ostream& output) {
	auto tks = _tokenize(input);
	cc0::Analyser analyser(tks);
	auto p = analyser.Analyse();
	if (p.second.has_value()) {
		fmt::print(stderr, "Syntactic analysis error: {}\n", p.second.value());
		exit(2);
	}

	// .constants
	output << fmt::format("{}\n", ".constants:");
	auto _index = 0;
	auto con = p.first._constants;

	for (auto& it : con) {
		auto name = std::get<std::string>(it.GetValue());
		output << fmt::format("{} {} \"{}\"\n", _index++, it.GetType(), name);
	}

	// .start
	output << fmt::format("{}\n", ".start:");
	_index = 0;
	auto sta = p.first._start;
	for (auto& it : sta) {
		output << fmt::format("{} {}\n", _index++, it);
	}

	std::vector<std::string> names;
	// .functions
	output << fmt::format("{}\n", ".functions:");
	_index = 0;
	auto fun = p.first._functions;
	for (auto& it : fun) {
		std::string name = std::get<std::string>(p.first._constants.at(it.GetNameIndex()).GetValue());
		names.push_back(name);
		output << fmt::format("{} {} {} {}\n", _index++, it.GetNameIndex(), it.GetParamSize(), it.GetLevel());
	}

	// .funN
	auto _fun_index = 0;
	for (auto iter : p.first._funN) {
		output << fmt::format(".F{}: #{}\n", _fun_index, names.at(_fun_index));
		_fun_index++;
		_index = 0;
		for (auto j : iter) {
			output << fmt::format("{} {}\n", _index++, j);
		}
	}

	return;
}

void Analyse(std::istream& input, std::stringstream& output) {
	auto tks = _tokenize(input);
	cc0::Analyser analyser(tks);
	auto p = analyser.Analyse();
	if (p.second.has_value()) {
		fmt::print(stderr, "Syntactic analysis error: {}\n", p.second.value());
		exit(2);
	}

	// .constants
	output << fmt::format("{}\n", ".constants:");
	auto _index = 0;
	auto con = p.first._constants;

	for (auto& it : con) {
		auto name = std::get<std::string>(it.GetValue());
		output << fmt::format("{} {} \"{}\"\n", _index++, it.GetType(), name);
	}

	// .start
	output << fmt::format("{}\n", ".start:");
	_index = 0;
	auto sta = p.first._start;
	for (auto& it : sta) {
		output << fmt::format("{} {}\n", _index++, it);
	}

	std::vector<std::string> names;
	// .functions
	output << fmt::format("{}\n", ".functions:");
	_index = 0;
	auto fun = p.first._functions;
	for (auto& it : fun) {
		std::string name = std::get<std::string>(p.first._constants.at(it.GetNameIndex()).GetValue());
		names.push_back(name);
		output << fmt::format("{} {} {} {}\n", _index++, it.GetNameIndex(), it.GetParamSize(), it.GetLevel());
	}

	// .funN
	auto _fun_index = 0;
	for (auto iter : p.first._funN) {
		output << fmt::format(".F{}: #{}\n", _fun_index, names.at(_fun_index));
		_fun_index++;
		_index = 0;
		for (auto j : iter) {
			output << fmt::format("{} {}\n", _index++, j);
		}
	}

	return;
}

void syntactic_and_assemble_text(std::ifstream* in, std::ofstream* out, bool run = false) {
	std::stringstream ss;
	Analyse(*in, ss);
	
	try {
		File f = File::parse_file_text(ss);
		// f.output_text(std::cout);
		f.output_binary(*out);
		if (run) {
			auto avm = std::move(vm::VM::make_vm(f));
			avm->start();
		}
	}
	catch (const std::exception & e) {
		println(std::cerr, e.what());
	}
}

int main(int argc, char** argv) {
	argparse::ArgumentParser program("cc0");
	program.add_argument("input")
		.required()
		.help("speicify the file to be compiled/executed.");
	program.add_argument("-s")
		.default_value(false)
		.implicit_value(true)
		.help("perform syntactic analysis for the input file.(.c0-->.s)");
	program.add_argument("-c")
		.default_value(false)
		.implicit_value(true)
		.help("assemble the text input file.(.c0-->.o)");
	program.add_argument("-o", "--output")
		.required()
		.default_value(std::string("-"))
		.help("specify the output file. if not provided, it will be \"-o out\".");
	program.add_argument("-t")
		.default_value(false)
		.implicit_value(true)
		.help("perform tokenization for the input file.");
	program.add_argument("-a")
		.default_value(false)
		.implicit_value(true)
		.help("assemble the text input file.(.s-->.o)");
	program.add_argument("-r")
		.default_value(false)
		.implicit_value(true)
		.help("interpret the binary input file.");
	program.add_argument("-d")
		.default_value(false)
		.implicit_value(true)
		.help("disassemble the binary input file.");

	try {
		program.parse_args(argc, argv);
	}
	catch (const std::runtime_error & err) {
		program.print_help();
		exit(2);
	}

	auto input_file = program.get<std::string>("input");
	auto output_file = program.get<std::string>("--output");

	std::istream* input;
	std::ostream* output;
	std::ifstream* inputf;
	std::ifstream inf;
	std::ofstream outf;

	if (program["-d"] == true) {
		if (program["-c"] == true || program["-r"] == true || program["-s"] == true || program["-t"] == true) {
			fmt::print(stderr, "You can only perform one move at one time.");
			exit(2);
		}

		inf.open(input_file, std::ios::binary | std::ios::in);
		if (!inf) {
			fmt::print(stderr, "Fail to open {} for reading.\n", input_file);
			exit(2);
		}
		inputf = &inf;

		if (output_file != "-") {
			if (input_file == output_file) {
				output_file += ".out";
			}
			outf.open(output_file, std::ios::out | std::ios::trunc);
			if (!outf) {
				inf.close();
				fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
				exit(2);
			}
			output = &outf;
		}
		else {
			output = &std::cout;
		}
		disassemble_binary(inputf, output);
	}
	else if (program["-c"] == true) {
		if (program["-d"] == true) {
			fmt::print(stderr, "You can only perform assemble or disassemble at one time.");
			exit(2);
		}

		inf.open(input_file, std::ios::in);
		if (!inf) {
			fmt::print(stderr, "Fail to open {} for reading.\n", input_file);
			exit(2);
		}
		inputf = &inf;

		if (output_file == "-")
			output_file = "out";
		
		if (input_file == output_file) {
			fmt::print(stderr, "Input file equals to Output file:{}\n", input_file);
		}
		
		outf.open(output_file, std::ios::binary | std::ios::out | std::ios::trunc);
		if (!outf) {
			inf.close();
			fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
			exit(2);
		}
		output = &outf;
		syntactic_and_assemble_text(inputf, dynamic_cast<std::ofstream*>(output), program["-r"] == true);
	}
	else if (program["-a"] == true) {
		if (program["-d"] == true) {
			fmt::print(stderr, "You can only perform assemble or disassemble at one time.");
			exit(2);
		}

		inf.open(input_file, std::ios::in);
		if (!inf) {
			fmt::print(stderr, "Fail to open {} for reading.\n", input_file);
			exit(2);
		}
		inputf = &inf;

		if (output_file == "-" || input_file == output_file) {
			output_file = input_file + ".out";
		}
		outf.open(output_file, std::ios::binary | std::ios::out | std::ios::trunc);
		if (!outf) {
			inf.close();
			fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
			exit(2);
		}
		output = &outf;
		assemble_text(inputf, dynamic_cast<std::ofstream*>(output), program["-r"] == true);
	}
	else if (program["-r"] == true) {
		inf.open(input_file, std::ios::binary | std::ios::in);
		if (!inf) {
			fmt::print(stderr, "Fail to open {} for reading.\n", input_file);
			exit(2);
		}
		inputf = &inf;

		if (output_file != "-") {
			if (input_file == output_file) {
				output_file += ".out";
			}
			outf.open(output_file, std::ios::out | std::ios::trunc);
			if (!outf) {
				inf.close();
				fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
				exit(2);
			}
			output = &outf;
		}
		else {
			output = &std::cout;
		}

		execute(inputf, output);
	}
	else {
		if (input_file != "-") {
			inf.open(input_file, std::ios::in);
			if (!inf) {
				fmt::print(stderr, "Fail to open {} for reading.\n", input_file);
				exit(2);
			}
			input = &inf;
		}
		else
			input = &std::cin;

		if (output_file == "-")	//给出默认的输出文件名
			output_file = "out";
		
		if (output_file != "-") {
			outf.open(output_file, std::ios::out | std::ios::trunc);
			if (!outf) {
				fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
				exit(2);
			}
			output = &outf;
		}
		else
			output = &std::cout;

		
		if (program["-t"] == true && program["-s"] == true) {
			fmt::print(stderr, "You can only perform tokenization or syntactic analysis at one time.");
			exit(2);
		}
		if (program["-t"] == true) {
			Tokenize(*input, *output);
		}
		else if (program["-s"] == true) {
			Analyse(*input, *output);
		}
		else {
			fmt::print(stderr, "You must choose tokenization or syntactic analysis.");
			exit(2);
		}
	}

	inf.close();
	outf.close();
	return 0;
}