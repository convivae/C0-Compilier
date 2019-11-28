#include "catch2/catch.hpp"
#include "simple_vm.hpp"
#include "instruction/instruction.h"
#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"

#include <sstream>

auto format(const c0::Operation& p, int num) {
	std::string name;
	std::string ss = std::to_string(num);

	switch (p) {
	case c0::ILL:
		name = "ILL"; name += "\t" + ss;
		break;
	case c0::ADD:
		name = "ADD";
		break;
	case c0::SUB:
		name = "SUB";
		break;
	case c0::MUL:
		name = "MUL";
		break;
	case c0::DIV:
		name = "DIV";
		break;
	case c0::WRT:
		name = "WRT";
		break;
	case c0::LIT:
		name = "LIT"; name += "\t" + ss;
		break;
	case c0::LOD:
		name = "LOD"; name += "\t" + ss;
		break;
	case c0::STO:
		name = "STO"; name += "\t" + ss;
		break;
	}
	return name;
}

/*自己造的样例*/
TEST_CASE("Test Base Analyser") {
	std::string input =
		"begin\n"
		"	var a = 3;\n"
		"	var c;\n"
		"	c = a + -100;\n"
		"	print(a + c);\n"
		"	print(a + c * 10 / (3 + 5) - 4);\n"
		"end\n";
	std::stringstream ss;
	ss.str(input);
	c0::Tokenizer tkz(ss);
	std::vector<int32_t> output = {
		-94,-122
	};
	//词法分析
	auto result = tkz.AllTokens();
	if (result.second.has_value()) {
		FAIL();
	}

	//语法分析
	c0::Analyser anz(result.first);
	auto anz_res = anz.Analyse();
	if (anz_res.second.has_value()) {
		FAIL();
	}

	//虚拟机
	auto vm = c0::VM(anz_res.first);
	auto vm_res = vm.Run();

	//输出
	// for (auto i : anz_res.first) {
	// 	std::cout << format(i.GetOperation(), i.GetX()) << std::endl;
	// }
	// for(auto i : vm_res) {
	// 	std::cout << "output:" << std::endl;
	// 	std::cout << '\t' << i << std::endl;
	// }

	REQUIRE(vm_res == output);
}

TEST_CASE("Test Analyser CONST NEED VALUE") {
	std::string input =
		"begin\n"
		"	const c;\n"
		"	print(c);\n"
		"end\n";
	std::stringstream ss;
	ss.str(input);

	//词法分析
	c0::Tokenizer tkz(ss);
	auto result = tkz.AllTokens();
	if (result.second.has_value()) {
		FAIL();
	}

	//语法分析
	c0::Analyser anz(result.first);
	auto anz_res = anz.Analyse();
	if (!anz_res.second.has_value()) {
		FAIL();
	}

	REQUIRE(anz_res.second->GetCode() == c0::ErrConstantNeedValue);
}

TEST_CASE("Test Analyser Duplicate Declaration") {
	std::string input =
		"begin\n"
		"	const c = 2;\n"
		"	var c;\n"
		"	print(c);\n"
		"end\n";
	std::stringstream ss;
	ss.str(input);

	//词法分析
	c0::Tokenizer tkz(ss);
	auto result = tkz.AllTokens();
	if (result.second.has_value()) {
		FAIL();
	}

	//语法分析
	c0::Analyser anz(result.first);
	auto anz_res = anz.Analyse();
	if (!anz_res.second.has_value()) {
		FAIL();
	}

	REQUIRE(anz_res.second->GetCode() == c0::ErrDuplicateDeclaration);
}

TEST_CASE("Test Analyser Not Declared") {
	std::string input =
		"begin\n"
		"	var c = 0;"
		"	c = 0 * 2;\n"
		"	print(c + a);\n"
		"end\n";
	std::stringstream ss;
	ss.str(input);

	//词法分析
	c0::Tokenizer tkz(ss);
	auto result = tkz.AllTokens();
	if (result.second.has_value()) {
		FAIL();
	}

	//语法分析
	c0::Analyser anz(result.first);
	auto anz_res = anz.Analyse();
	if (!anz_res.second.has_value()) {
		FAIL();
	}

	REQUIRE(anz_res.second->GetCode() == c0::ErrNotDeclared);
}

TEST_CASE("Test Analyser Assign To Constant") {
	std::string input =
		"begin\n"
		"	const c = 0;"
		"	c = 0 * 2;\n"
		"	print(c);\n"
		"end\n";
	std::stringstream ss;
	ss.str(input);

	//词法分析
	c0::Tokenizer tkz(ss);
	auto result = tkz.AllTokens();
	if (result.second.has_value()) {
		FAIL();
	}

	//语法分析
	c0::Analyser anz(result.first);
	auto anz_res = anz.Analyse();
	if (!anz_res.second.has_value()) {
		FAIL();
	}

	REQUIRE(anz_res.second->GetCode() == c0::ErrAssignToConstant);
}

TEST_CASE("Test Analyser Not Initialized") {
	std::string input =
		"begin\n"
		"	var c;"
		"	c = 100 * c + 10;"
		"	print(c);\n"
		"end\n";
	std::stringstream ss;
	ss.str(input);

	//词法分析
	c0::Tokenizer tkz(ss);
	auto result = tkz.AllTokens();
	if (result.second.has_value()) {
		FAIL();
	}

	//语法分析
	c0::Analyser anz(result.first);
	auto anz_res = anz.Analyse();
	if (!anz_res.second.has_value()) {
		FAIL();
	}

	REQUIRE(anz_res.second->GetCode() == c0::ErrNotInitialized);
}

//下面是助教给的样例
//valid
TEST_CASE("assign") {
	std::string input =
		"begin\n"
		"var a = 1;\n"
		"var b;\n"
		"var c;\n"
		"var d;\n"
		"var e;\n"
		"b = a;\n"
		"e = b;\n"
		"d = e;\n"
		"c = a;\n"
		"print(c);\n"
		"end";
	std::stringstream ss;
	ss.str(input);

	//词法分析
	c0::Tokenizer tkz(ss);
	auto result = tkz.AllTokens();
	if (result.second.has_value()) {
		FAIL();
	}

	//语法分析
	c0::Analyser anz(result.first);
	auto anz_res = anz.Analyse();
	if (anz_res.second.has_value()) {
		FAIL();
	}

	//虚拟机
	auto vm = c0::VM(anz_res.first);
	auto vm_res = vm.Run();

	//输出
	// for (auto i : anz_res.first) {
	// 	std::cout << format(i.GetOperation(), i.GetX()) << std::endl;
	// }
	// for(auto i : vm_res) {
	// 	std::cout << "output:" << std::endl;
	// 	std::cout << '\t' << i << std::endl;
	// }
	//
	std::vector<int32_t> output = {
		1
	};

	REQUIRE(vm_res == output);
}

TEST_CASE("declaration") {
	std::string input =
		"begin\n"
		"const abc = 123;\n"
		"var ABC = 456;\n"
		"print(abc);\n"
		"print(ABC);\n"
		"end";
	std::stringstream ss;
	ss.str(input);

	//词法分析
	c0::Tokenizer tkz(ss);
	auto result = tkz.AllTokens();
	if (result.second.has_value()) {
		FAIL();
	}

	//语法分析
	c0::Analyser anz(result.first);
	auto anz_res = anz.Analyse();
	if (anz_res.second.has_value()) {
		FAIL();
	}

	//虚拟机
	auto vm = c0::VM(anz_res.first);
	auto vm_res = vm.Run();

	//输出
	// for (auto i : anz_res.first) {
	// 	std::cout << format(i.GetOperation(), i.GetX()) << std::endl;
	// }
	// for(auto i : vm_res) {
	// 	std::cout << "output:" << std::endl;
	// 	std::cout << '\t' << i << std::endl;
	// }
	//
	std::vector<int32_t> output = {
		123,456
	};

	REQUIRE(vm_res == output);
}

TEST_CASE("init") {
	std::string input =
		"begin\n"
		"var a = 0;\n"
		"var b = 1;\n"
		"var c = a+b;\n"
		"a = b;\n"
		"c = c;\n"
		"c = a+b;\n"
		"a = b;\n"
		"b = c;\n"
		"print(a);\n"
		"print(b);\n"
		"print(c);\n"
		"end";
	std::stringstream ss;
	ss.str(input);

	//词法分析
	c0::Tokenizer tkz(ss);
	auto result = tkz.AllTokens();
	if (result.second.has_value()) {
		FAIL();
	}

	//语法分析
	c0::Analyser anz(result.first);
	auto anz_res = anz.Analyse();
	if (anz_res.second.has_value()) {
		FAIL();
	}

	//虚拟机
	auto vm = c0::VM(anz_res.first);
	auto vm_res = vm.Run();

	//输出
	// for (auto i : anz_res.first) {
	// 	std::cout << format(i.GetOperation(), i.GetX()) << std::endl;
	// }
	// for(auto i : vm_res) {
	// 	std::cout << "output:" << std::endl;
	// 	std::cout << '\t' << i << std::endl;
	// }
	//
	std::vector<int32_t> output = {
		1,2,2
	};

	REQUIRE(vm_res == output);
}

//invalid
TEST_CASE("missing_begin_end") {
	std::string input =
		"var a = 1;\n"
		"print(a);";
	std::stringstream ss;
	ss.str(input);

	//词法分析
	c0::Tokenizer tkz(ss);
	auto result = tkz.AllTokens();
	if (result.second.has_value()) {
		FAIL();
	}

	//语法分析
	c0::Analyser anz(result.first);
	auto anz_res = anz.Analyse();
	if (!anz_res.second.has_value()) {
		FAIL();
	}

	REQUIRE(anz_res.second->GetCode() == c0::ErrNoBegin);
}

TEST_CASE("missing_semicolon") {
	std::string input =
		"begin\n"
		"const A = 1;\n"
		"var B = A;\n"
		"print(A)\n"
		"end";
	std::stringstream ss;
	ss.str(input);

	//词法分析
	c0::Tokenizer tkz(ss);
	auto result = tkz.AllTokens();
	if (result.second.has_value()) {
		FAIL();
	}

	//语法分析
	c0::Analyser anz(result.first);
	auto anz_res = anz.Analyse();
	if (!anz_res.second.has_value()) {
		FAIL();
	}

	REQUIRE(anz_res.second->GetCode() == c0::ErrNoSemicolon);
}

TEST_CASE("redeclaration") {
	std::string input =
		"begin\n"
		"const A = 1;\n"
		"var A;\n"
		"end";
	std::stringstream ss;
	ss.str(input);

	//词法分析
	c0::Tokenizer tkz(ss);
	auto result = tkz.AllTokens();
	if (result.second.has_value()) {
		FAIL();
	}

	//语法分析
	c0::Analyser anz(result.first);
	auto anz_res = anz.Analyse();
	if (!anz_res.second.has_value()) {
		FAIL();
	}

	REQUIRE(anz_res.second->GetCode() == c0::ErrDuplicateDeclaration);
}

TEST_CASE("uninit") {
	std::string input =
		"begin\n"
		"var a;\n"
		"print(a);\n"
		"end";
	std::stringstream ss;
	ss.str(input);

	//词法分析
	c0::Tokenizer tkz(ss);
	auto result = tkz.AllTokens();
	if (result.second.has_value()) {
		FAIL();
	}

	//语法分析
	c0::Analyser anz(result.first);
	auto anz_res = anz.Analyse();
	if (!anz_res.second.has_value()) {
		FAIL();
	}

	REQUIRE(anz_res.second->GetCode() == c0::ErrNotInitialized);
}

TEST_CASE("var_const") {
	std::string input =
		"begin\n"
		"var a;\n"
		"const b = 1;\n"
		"end";
	std::stringstream ss;
	ss.str(input);

	//词法分析
	c0::Tokenizer tkz(ss);
	auto result = tkz.AllTokens();
	if (result.second.has_value()) {
		FAIL();
	}

	//语法分析
	c0::Analyser anz(result.first);
	auto anz_res = anz.Analyse();
	if (!anz_res.second.has_value()) {
		FAIL();
	}

	REQUIRE(anz_res.second->GetCode() == c0::ErrNoEnd);
}