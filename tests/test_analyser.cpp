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
        "void hanoi(int n, int a, int b, int c) {\n"
        "\tif (n == 1) {\n"
        "\t\tprint(a, 0x00, c);\n"
        "\t}\n"
        "\telse {\n"
        "\t\thanoi(n-1, a, c, b);\n"
        "\t\tprint(a, 0x00, c);\n"
        "\t\thanoi(n-1, b, a, c);\n"
        "\t\t\n"
        "\t}\n"
        "}\n"
        "\n"
        "int main() {\n"
        "\thanoi(3, 1, 2, 3);\n"
		"\tprint(1);\n"
        "\treturn 0;\n"
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
//
//TEST_CASE("Test Analyser CONST NEED VALUE") {
//	std::string input =
//		"begin\n"
//		"	const c;\n"
//		"	print(c);\n"
//		"end\n";
//	std::stringstream ss;
//	ss.str(input);
//
//	//词法分析
//	cc0::Tokenizer tkz(ss);
//	auto result = tkz.AllTokens();
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	//语法分析
//	cc0::Analyser anz(result.first);
//	auto anz_res = anz.Analyse();
//	if (!anz_res.second.has_value()) {
//		FAIL();
//	}
//
//	REQUIRE(anz_res.second->GetCode() == cc0::ErrConstantNeedValue);
//}
//
//TEST_CASE("Test Analyser Duplicate Declaration") {
//	std::string input =
//		"begin\n"
//		"	const c = 2;\n"
//		"	var c;\n"
//		"	print(c);\n"
//		"end\n";
//	std::stringstream ss;
//	ss.str(input);
//
//	//词法分析
//	cc0::Tokenizer tkz(ss);
//	auto result = tkz.AllTokens();
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	//语法分析
//	cc0::Analyser anz(result.first);
//	auto anz_res = anz.Analyse();
//	if (!anz_res.second.has_value()) {
//		FAIL();
//	}
//
//	REQUIRE(anz_res.second->GetCode() == cc0::ErrDuplicateDeclaration);
//}
//
//TEST_CASE("Test Analyser Not Declared") {
//	std::string input =
//		"begin\n"
//		"	var c = 0;"
//		"	c = 0 * 2;\n"
//		"	print(c + a);\n"
//		"end\n";
//	std::stringstream ss;
//	ss.str(input);
//
//	//词法分析
//	cc0::Tokenizer tkz(ss);
//	auto result = tkz.AllTokens();
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	//语法分析
//	cc0::Analyser anz(result.first);
//	auto anz_res = anz.Analyse();
//	if (!anz_res.second.has_value()) {
//		FAIL();
//	}
//
//	REQUIRE(anz_res.second->GetCode() == cc0::ErrNotDeclared);
//}
//
//TEST_CASE("Test Analyser Assign To Constant") {
//	std::string input =
//		"begin\n"
//		"	const c = 0;"
//		"	c = 0 * 2;\n"
//		"	print(c);\n"
//		"end\n";
//	std::stringstream ss;
//	ss.str(input);
//
//	//词法分析
//	cc0::Tokenizer tkz(ss);
//	auto result = tkz.AllTokens();
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	//语法分析
//	cc0::Analyser anz(result.first);
//	auto anz_res = anz.Analyse();
//	if (!anz_res.second.has_value()) {
//		FAIL();
//	}
//
//	REQUIRE(anz_res.second->GetCode() == cc0::ErrAssignToConstant);
//}
//
//TEST_CASE("Test Analyser Not Initialized") {
//	std::string input =
//		"begin\n"
//		"	var c;"
//		"	c = 100 * c + 10;"
//		"	print(c);\n"
//		"end\n";
//	std::stringstream ss;
//	ss.str(input);
//
//	//词法分析
//	cc0::Tokenizer tkz(ss);
//	auto result = tkz.AllTokens();
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	//语法分析
//	cc0::Analyser anz(result.first);
//	auto anz_res = anz.Analyse();
//	if (!anz_res.second.has_value()) {
//		FAIL();
//	}
//
//	REQUIRE(anz_res.second->GetCode() == cc0::ErrNotInitialized);
//}
//
////下面是助教给的样例
////valid
//TEST_CASE("assign") {
//	std::string input =
//		"begin\n"
//		"var a = 1;\n"
//		"var b;\n"
//		"var c;\n"
//		"var d;\n"
//		"var e;\n"
//		"b = a;\n"
//		"e = b;\n"
//		"d = e;\n"
//		"c = a;\n"
//		"print(c);\n"
//		"end";
//	std::stringstream ss;
//	ss.str(input);
//
//	//词法分析
//	cc0::Tokenizer tkz(ss);
//	auto result = tkz.AllTokens();
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	//语法分析
//	cc0::Analyser anz(result.first);
//	auto anz_res = anz.Analyse();
//	if (anz_res.second.has_value()) {
//		FAIL();
//	}
//
//	//虚拟机
//	auto vm = cc0::VM(anz_res.first);
//	auto vm_res = vm.Run();
//
//	//输出
//	// for (auto i : anz_res.first) {
//	// 	std::cout << format(i.GetOperation(), i.GetX()) << std::endl;
//	// }
//	// for(auto i : vm_res) {
//	// 	std::cout << "output:" << std::endl;
//	// 	std::cout << '\t' << i << std::endl;
//	// }
//	//
//	std::vector<int32_t> output = {
//		1
//	};
//
//	REQUIRE(vm_res == output);
//}
//
//TEST_CASE("declaration") {
//	std::string input =
//		"begin\n"
//		"const abc = 123;\n"
//		"var ABC = 456;\n"
//		"print(abc);\n"
//		"print(ABC);\n"
//		"end";
//	std::stringstream ss;
//	ss.str(input);
//
//	//词法分析
//	cc0::Tokenizer tkz(ss);
//	auto result = tkz.AllTokens();
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	//语法分析
//	cc0::Analyser anz(result.first);
//	auto anz_res = anz.Analyse();
//	if (anz_res.second.has_value()) {
//		FAIL();
//	}
//
//	//虚拟机
//	auto vm = cc0::VM(anz_res.first);
//	auto vm_res = vm.Run();
//
//	//输出
//	// for (auto i : anz_res.first) {
//	// 	std::cout << format(i.GetOperation(), i.GetX()) << std::endl;
//	// }
//	// for(auto i : vm_res) {
//	// 	std::cout << "output:" << std::endl;
//	// 	std::cout << '\t' << i << std::endl;
//	// }
//	//
//	std::vector<int32_t> output = {
//		123,456
//	};
//
//	REQUIRE(vm_res == output);
//}
//
//TEST_CASE("init") {
//	std::string input =
//		"begin\n"
//		"var a = 0;\n"
//		"var b = 1;\n"
//		"var c = a+b;\n"
//		"a = b;\n"
//		"c = c;\n"
//		"c = a+b;\n"
//		"a = b;\n"
//		"b = c;\n"
//		"print(a);\n"
//		"print(b);\n"
//		"print(c);\n"
//		"end";
//	std::stringstream ss;
//	ss.str(input);
//
//	//词法分析
//	cc0::Tokenizer tkz(ss);
//	auto result = tkz.AllTokens();
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	//语法分析
//	cc0::Analyser anz(result.first);
//	auto anz_res = anz.Analyse();
//	if (anz_res.second.has_value()) {
//		FAIL();
//	}
//
//	//虚拟机
//	auto vm = cc0::VM(anz_res.first);
//	auto vm_res = vm.Run();
//
//	//输出
//	// for (auto i : anz_res.first) {
//	// 	std::cout << format(i.GetOperation(), i.GetX()) << std::endl;
//	// }
//	// for(auto i : vm_res) {
//	// 	std::cout << "output:" << std::endl;
//	// 	std::cout << '\t' << i << std::endl;
//	// }
//	//
//	std::vector<int32_t> output = {
//		1,2,2
//	};
//
//	REQUIRE(vm_res == output);
//}
//
////invalid
//TEST_CASE("missing_begin_end") {
//	std::string input =
//		"var a = 1;\n"
//		"print(a);";
//	std::stringstream ss;
//	ss.str(input);
//
//	//词法分析
//	cc0::Tokenizer tkz(ss);
//	auto result = tkz.AllTokens();
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	//语法分析
//	cc0::Analyser anz(result.first);
//	auto anz_res = anz.Analyse();
//	if (!anz_res.second.has_value()) {
//		FAIL();
//	}
//
//	REQUIRE(anz_res.second->GetCode() == cc0::ErrNoBegin);
//}
//
//TEST_CASE("missing_semicolon") {
//	std::string input =
//		"begin\n"
//		"const A = 1;\n"
//		"var B = A;\n"
//		"print(A)\n"
//		"end";
//	std::stringstream ss;
//	ss.str(input);
//
//	//词法分析
//	cc0::Tokenizer tkz(ss);
//	auto result = tkz.AllTokens();
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	//语法分析
//	cc0::Analyser anz(result.first);
//	auto anz_res = anz.Analyse();
//	if (!anz_res.second.has_value()) {
//		FAIL();
//	}
//
//	REQUIRE(anz_res.second->GetCode() == cc0::ErrNoSemicolon);
//}
//
//TEST_CASE("redeclaration") {
//	std::string input =
//		"begin\n"
//		"const A = 1;\n"
//		"var A;\n"
//		"end";
//	std::stringstream ss;
//	ss.str(input);
//
//	//词法分析
//	cc0::Tokenizer tkz(ss);
//	auto result = tkz.AllTokens();
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	//语法分析
//	cc0::Analyser anz(result.first);
//	auto anz_res = anz.Analyse();
//	if (!anz_res.second.has_value()) {
//		FAIL();
//	}
//
//	REQUIRE(anz_res.second->GetCode() == cc0::ErrDuplicateDeclaration);
//}
//
//TEST_CASE("uninit") {
//	std::string input =
//		"begin\n"
//		"var a;\n"
//		"print(a);\n"
//		"end";
//	std::stringstream ss;
//	ss.str(input);
//
//	//词法分析
//	cc0::Tokenizer tkz(ss);
//	auto result = tkz.AllTokens();
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	//语法分析
//	cc0::Analyser anz(result.first);
//	auto anz_res = anz.Analyse();
//	if (!anz_res.second.has_value()) {
//		FAIL();
//	}
//
//	REQUIRE(anz_res.second->GetCode() == cc0::ErrNotInitialized);
//}
//
//TEST_CASE("var_const") {
//	std::string input =
//		"begin\n"
//		"var a;\n"
//		"const b = 1;\n"
//		"end";
//	std::stringstream ss;
//	ss.str(input);
//
//	//词法分析
//	cc0::Tokenizer tkz(ss);
//	auto result = tkz.AllTokens();
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	//语法分析
//	cc0::Analyser anz(result.first);
//	auto anz_res = anz.Analyse();
//	if (!anz_res.second.has_value()) {
//		FAIL();
//	}
//
//	REQUIRE(anz_res.second->GetCode() == cc0::ErrNoEnd);
//}
//
//
//

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