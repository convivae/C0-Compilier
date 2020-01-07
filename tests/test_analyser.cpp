#include "catch2/catch.hpp"
#include "vm/vm.h"
#include "fmts.hpp"
#include "instruction/instruction.h"
#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"

#include <sstream>

/*Analyser*/
void printAnalyser(std::pair<cc0::Output, std::optional<cc0::CompilationError>>& p, std::stringstream& output);

/*VM*/
void assemble_and_run(std::stringstream& in);

/*自己造的样例*/
TEST_CASE("Test Base Analyser") {
	std::string input =
        "int fun()\n"
        "{\n"
        "    int A;\n"
        "    scan(A);\n"
        "    while(A < 10){\n"
        "        A = A + 1;\n"
        "        print(A);\n"
        "    }\n"
        "    return A;\n"
        "}\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int A;\n"
        "    scan(A);\n"
        "    fun();\n"
        "    return 0;\n"
        "}";
	std::stringstream ss;
	ss.str(input);
	cc0::Tokenizer tkz(ss);

	//词法分析
	std::cout << "\nTokenizer\n" << std::endl;
	auto result = tkz.AllTokens();

	for (auto i : result.first) {
		std::cout << fmt::format("{}\n", i);
	}

	if (result.second.has_value()) {
		fmt::print(stderr, "Tokenization error: {}\n", result.second.value());
		FAIL();
	}

	//语法分析
	std::cout << "\nAnalyser\n" << std::endl;
	cc0::Analyser anz(result.first);
	auto anz_res = anz.Analyse();
	if (anz_res.second.has_value()) {
		fmt::print(stderr, "Syntactic analysis error: {}\n", anz_res.second.value());
		FAIL();
	}

	ss.clear();
	ss.str("");
	printAnalyser(anz_res, ss);
	std::cout << ss.str() << std::endl;

	std::cout << "Result:" << std::endl;
	assemble_and_run(ss);
	std::cout << std::endl;
}

/*Analyser*/
void printAnalyser(std::pair<cc0::Output, std::optional<cc0::CompilationError>>& p, std::stringstream& output)
{
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
}

void assemble_and_run(std::stringstream& in) {
	try {
		File f = File::parse_file_text(in);
		auto avm = std::move(vm::VM::make_vm(f));
		avm->start();
	}
	catch (const std::exception & e) {
		println(std::cerr, e.what());
	}
}