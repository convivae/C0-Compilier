#include "catch2/catch.hpp"
#include "tokenizer/tokenizer.h"
#include "fmt/core.h"

#include <sstream>
#include <vector>

/* 自己构造的测试用例 */
TEST_CASE("Token Base Test") {
	std::string input =
		"begin\n"
		"	const b == a > b <= c != a;\n"
		"	var a = 1;\n"
		"	print(a+b);\n"
		"end\n";
	std::stringstream ss;
	ss.str(input);
	c0::Tokenizer tkz(ss);
//	std::vector<c0::Token> output = {
//		c0::Token(c0::BEGIN,std::string("begin"),0,0,0,5),
//		c0::Token(c0::CONST,std::string("const"),1,1,1,6),
//		c0::Token(c0::IDENTIFIER,std::string("b"),1,7,1,8),
//		c0::Token(c0::EQUAL_SIGN,'=',1,9,1,10),
//		c0::Token(c0::UNSIGNED_INTEGER,std::string("2147483647"),1,11,1,21),
//		c0::Token(c0::SEMICOLON,';',1,21,1,22),
//		c0::Token(c0::VAR,std::string("var"),2,1,2,4),
//		c0::Token(c0::IDENTIFIER,std::string("a"),2,5,2,6),
//		c0::Token(c0::EQUAL_SIGN,'=',2,7,2,8),
//		c0::Token(c0::UNSIGNED_INTEGER,std::string("1"),2,9,2,10),
//		c0::Token(c0::SEMICOLON,';',2,10,2,11),
//		c0::Token(c0::PRINT,std::string("print"),3,1,3,6),
//		c0::Token(c0::LEFT_BRACKET,'(',3,6,3,7),
//		c0::Token(c0::IDENTIFIER,std::string("a"),3,7,3,8),
//		c0::Token(c0::PLUS_SIGN,'+',3,8,3,9),
//		c0::Token(c0::IDENTIFIER,std::string("b"),3,9,3,10),
//		c0::Token(c0::RIGHT_BRACKET,')',3,10,3,11),
//		c0::Token(c0::SEMICOLON,';',3,11,3,12),
//		c0::Token(c0::END,std::string("end"),4,0,4,3),
//	};
	auto result = tkz.AllTokens();

	if (result.second.has_value()) {
		std::cout << result.second->GetCode() << std::endl;
		FAIL();
	}

	//调试输出
	 for (auto i : result.first) {
	 	std::cout << i.GetType() << '\t' << "start " << i.GetStartPos().first << '\t' << i.GetStartPos().second \
	 		<< "\tend " << i.GetEndPos().first << '\t' << i.GetEndPos().second << '\t' << i.GetValueString() << std::endl;
	 }

	//REQUIRE(result.first == output);
}

/* 助教给的测试用例 */
TEST_CASE("empty_file") {
	std::string input =
		"";
	std::stringstream ss;
	ss.str(input);
	c0::Tokenizer tkz(ss);
	std::vector<c0::Token> output = {};
	auto result = tkz.AllTokens();

	if (result.second.has_value()) {
		FAIL();
	}

	REQUIRE(result.first.empty());
}

TEST_CASE("whitespaces") {
    std::string input =
            "   \t     \t   \t            \n\r"
            " \n"
            "\t\t  \t\t\t\n"
            " \n"
            "\n"
            " \n"
            "\t\n"
            " \n"
            "  \n"
            " \n"
            "\t  \t\n"
            "\t  \n"
            "\t \n"
            " \n"
            "\n"
            " \n"
            "\n"
            " \n"
            "\n"
            " \t \t \n"
            " \n"
            " \t \t \t \n"
            "\t \t \t \n"
            " \n"
            "\n"
            " \n"
            "\t \t \n"
            "\n"
            "\n"
            " ";
    std::stringstream ss;
    ss.str(input);
    c0::Tokenizer tkz(ss);
    std::vector<c0::Token> output = {};
    auto result = tkz.AllTokens();

    if (result.second.has_value()) {
        FAIL();
    }

    REQUIRE(result.first.empty());
}

//TEST_CASE("identifiers") {
//	std::string input =
//		"a\n"
//		"A\n"
//		"abc\n"
//		"ABC\n"
//		"Abc\n"
//		"aBc\n"
//		"aaawiogfpiusaGPIFsbfbpiweifgbpIAEGPFIewpifgpibpijgbpijbgpbijpbIPJabipPDP\n"
//		"a1\n"
//		"a0989852\n"
//		"A5\n"
//		"A21646452\n"
//		"a2431A\n"
//		"a5s6sa89sa9asf5asf98asf5789asf5789asf9587\n"
//		"a7dt b87TR8D sr780sA7D089 TS87tdxb08 TX08tn \n"
//		"d70SADT087 satdx697R  NX9X2141sga2asfEN08qw\n"
//		"\n"
//		"\n"
//		"BEGIN\n"
//		"END\n"
//		"CONST\n"
//		"VAR\n"
//		"PRINT\n"
//		"\n"
//		"BeGiN\n"
//		"eNd\n"
//		"CONst\n"
//		"vaR\n"
//		"priNT\n"
//		"\n"
//		"beginend\n"
//		"beginEND\n"
//		"CONSTvar\n"
//		"begin123456end\n"
//		"print987654321\n"
//		"const0\n"
//		"var1";
//	std::stringstream ss;
//	ss.str(input);
//	c0::Tokenizer tkz(ss);
//	std::vector<c0::Token> output = {
//		c0::Token(c0::IDENTIFIER,std::string("a"),0,0,0,1),
//		c0::Token(c0::IDENTIFIER,std::string("A"),1,0,1,1),
//		c0::Token(c0::IDENTIFIER,std::string("abc"),2,0,2,3),
//		c0::Token(c0::IDENTIFIER,std::string("ABC"),3,0,3,3),
//		c0::Token(c0::IDENTIFIER,std::string("Abc"),4,0,4,3),
//		c0::Token(c0::IDENTIFIER,std::string("aBc"),5,0,5,3),
//		c0::Token(c0::IDENTIFIER,std::string("aaawiogfpiusaGPIFsbfbpiweifgbpIAEGPFIewpifgpibpijgbpijbgpbijpbIPJabipPDP"),6,0,6,72),
//		c0::Token(c0::IDENTIFIER,std::string("a1"),7,0,7,2),
//		c0::Token(c0::IDENTIFIER,std::string("a0989852"),8,0,8,8),
//		c0::Token(c0::IDENTIFIER,std::string("A5"),9,0,9,2),
//		c0::Token(c0::IDENTIFIER,std::string("A21646452"),10,0,10,9),
//		c0::Token(c0::IDENTIFIER,std::string("a2431A"),11,0,11,6),
//		c0::Token(c0::IDENTIFIER,std::string("a5s6sa89sa9asf5asf98asf5789asf5789asf9587"),12,0,12,41),
//		c0::Token(c0::IDENTIFIER,std::string("a7dt"),13,0,13,4),
//		c0::Token(c0::IDENTIFIER,std::string("b87TR8D"),13,5,13,12),
//		c0::Token(c0::IDENTIFIER,std::string("sr780sA7D089"),13,13,13,25),
//		c0::Token(c0::IDENTIFIER,std::string("TS87tdxb08"),13,26,13,36),
//		c0::Token(c0::IDENTIFIER,std::string("TX08tn"),13,37,13,43),
//		c0::Token(c0::IDENTIFIER,std::string("d70SADT087"),14,0,14,10),
//		c0::Token(c0::IDENTIFIER,std::string("satdx697R"),14,11,14,20),
//		c0::Token(c0::IDENTIFIER,std::string("NX9X2141sga2asfEN08qw"),14,22,14,43),
//		c0::Token(c0::IDENTIFIER,std::string("BEGIN"),17,0,17,5),
//		c0::Token(c0::IDENTIFIER,std::string("END"),18,0,18,3),
//		c0::Token(c0::IDENTIFIER,std::string("CONST"),19,0,19,5),
//		c0::Token(c0::IDENTIFIER,std::string("VAR"),20,0,20,3),
//		c0::Token(c0::IDENTIFIER,std::string("PRINT"),21,0,21,5),
//		c0::Token(c0::IDENTIFIER,std::string("BeGiN"),23,0,23,5),
//		c0::Token(c0::IDENTIFIER,std::string("eNd"),24,0,24,3),
//		c0::Token(c0::IDENTIFIER,std::string("CONst"),25,0,25,5),
//		c0::Token(c0::IDENTIFIER,std::string("vaR"),26,0,26,3),
//		c0::Token(c0::IDENTIFIER,std::string("priNT"),27,0,27,5),
//		c0::Token(c0::IDENTIFIER,std::string("beginend"),29,0,29,8),
//		c0::Token(c0::IDENTIFIER,std::string("beginEND"),30,0,30,8),
//		c0::Token(c0::IDENTIFIER,std::string("CONSTvar"),31,0,31,8),
//		c0::Token(c0::IDENTIFIER,std::string("begin123456end"),32,0,32,14),
//		c0::Token(c0::IDENTIFIER,std::string("print987654321"),33,0,33,14),
//		c0::Token(c0::IDENTIFIER,std::string("const0"),34,0,34,6),
//		c0::Token(c0::IDENTIFIER,std::string("var1"),35,0,35,4),
//	};
//	auto result = tkz.AllTokens();
//
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	//调试输出
//	// for (auto i : result.first) {
//	// 	std::cout << i.GetType() << '\t' << "start " << i.GetStartPos().first << '\t' << i.GetStartPos().second \
//	// 		<< "\tend " << i.GetEndPos().first << '\t' << i.GetEndPos().second << '\t' << i.GetValueString() << std::endl;
//	// }
//
//	REQUIRE(result.first == output);
//}
//
//TEST_CASE("integers") {
//	std::string input =
//		"0\n"
//		"000000000000\n"
//		"9\n"
//		"01\n"
//		"23456\n"
//		"098765\n"
//		"1111111111\n"
//		"2147483647\n"
//		"0000000000000000000000000000000000000000000000000000000000000000000000000012345";
//	std::stringstream ss;
//	ss.str(input);
//	c0::Tokenizer tkz(ss);
//	std::vector<c0::Token> output = {
//		c0::Token(c0::UNSIGNED_INTEGER,std::string("0"),0,0,0,1),
//		c0::Token(c0::UNSIGNED_INTEGER,std::string("0"),1,0,1,12),
//		c0::Token(c0::UNSIGNED_INTEGER,std::string("9"),2,0,2,1),
//		c0::Token(c0::UNSIGNED_INTEGER,std::string("1"),3,0,3,2),
//		c0::Token(c0::UNSIGNED_INTEGER,std::string("23456"),4,0,4,5),
//		c0::Token(c0::UNSIGNED_INTEGER,std::string("98765"),5,0,5,6),
//		c0::Token(c0::UNSIGNED_INTEGER,std::string("1111111111"),6,0,6,10),
//		c0::Token(c0::UNSIGNED_INTEGER,std::string("2147483647"),7,0,7,10),
//		c0::Token(c0::UNSIGNED_INTEGER,std::string("12345"),8,0,8,79),
//	};
//	auto result = tkz.AllTokens();
//
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	// for (auto i : result.first) {
//	// std::cout << i.GetType() << '\t' << "start " << i.GetStartPos().first << '\t' << i.GetStartPos().second \
//	// 		<< "\tend " << i.GetEndPos().first << '\t' << i.GetEndPos().second << '\t' << i.GetValueString() << std::endl;
//	// }
//
//	REQUIRE(result.first == output);
//}
//
//TEST_CASE("keywords") {
//	std::string input =
//		"begin\n"
//		"end\n"
//		"const\n"
//		"var\n"
//		"print";
//	std::stringstream ss;
//	ss.str(input);
//	c0::Tokenizer tkz(ss);
//	std::vector<c0::Token> output = {
//		c0::Token(c0::BEGIN,std::string("begin"),0,0,0,5),
//		c0::Token(c0::END,std::string("end"),1,0,1,3),
//		c0::Token(c0::CONST,std::string("const"),2,0,2,5),
//		c0::Token(c0::VAR,std::string("var"),3,0,3,3),
//		c0::Token(c0::PRINT,std::string("print"),4,0,4,5),
//	};
//	auto result = tkz.AllTokens();
//
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	// for (auto i : result.first) {
//	// 	std::cout << i.GetType() << '\t' << "start " << i.GetStartPos().first << '\t' << i.GetStartPos().second \
//	// 		<< "\tend " << i.GetEndPos().first << '\t' << i.GetEndPos().second << '\t' << i.GetValueString() << std::endl;
//	// }
//
//	REQUIRE(result.first == output);
//}

//TEST_CASE("no_ws_at_the_end1") {
//	std::string input =
//		"begin";
//	std::stringstream ss;
//	ss.str(input);
//	c0::Tokenizer tkz(ss);
//	std::vector<c0::Token> output = {
//		c0::Token(c0::BEGIN,std::string("begin"),0,0,0,5),
//	};
//	auto result = tkz.AllTokens();
//
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	REQUIRE(result.first == output);
//}
//
//TEST_CASE("no_ws_at_the_end2") {
//	std::string input =
//		";";
//	std::stringstream ss;
//	ss.str(input);
//	c0::Tokenizer tkz(ss);
//	std::vector<c0::Token> output = {
//		c0::Token(c0::SEMICOLON,std::string(";"),0,0,0,1),
//	};
//	auto result = tkz.AllTokens();
//
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	REQUIRE(result.first == output);
//}
//
//TEST_CASE("operators") {
//	std::string input =
//		"- + * /\n"
//		"+-*/\n"
//		"++ --\n"
//		"= += -= *= /= ==\n"
//		"//\n"
//		"**\n"
//		"/**/\n"
//		"/*/";
//	std::stringstream ss;
//	ss.str(input);
//	c0::Tokenizer tkz(ss);
//	std::vector<c0::Token> output = {
//		c0::Token(c0::MINUS_SIGN,std::string("-"),0,0,0,1),
//		c0::Token(c0::PLUS_SIGN,std::string("+"),0,2,0,3),
//		c0::Token(c0::MULTIPLICATION_SIGN,std::string("*"),0,4,0,5),
//		c0::Token(c0::DIVISION_SIGN,std::string("/"),0,6,0,7),
//		c0::Token(c0::PLUS_SIGN,std::string("+"),1,0,1,1),
//		c0::Token(c0::MINUS_SIGN,std::string("-"),1,1,1,2),
//		c0::Token(c0::MULTIPLICATION_SIGN,std::string("*"),1,2,1,3),
//		c0::Token(c0::DIVISION_SIGN,std::string("/"),1,3,1,4),
//		c0::Token(c0::PLUS_SIGN,std::string("+"),2,0,2,1),
//		c0::Token(c0::PLUS_SIGN,std::string("+"),2,1,2,2),
//		c0::Token(c0::MINUS_SIGN,std::string("-"),2,3,2,4),
//		c0::Token(c0::MINUS_SIGN,std::string("-"),2,4,2,5),
//		c0::Token(c0::EQUAL_SIGN,std::string("="),3,0,3,1),
//		c0::Token(c0::PLUS_SIGN,std::string("+"),3,2,3,3),
//		c0::Token(c0::EQUAL_SIGN,std::string("="),3,3,3,4),
//		c0::Token(c0::MINUS_SIGN,std::string("-"),3,5,3,6),
//		c0::Token(c0::EQUAL_SIGN,std::string("="),3,6,3,7),
//		c0::Token(c0::MULTIPLICATION_SIGN,std::string("*"),3,8,3,9),
//		c0::Token(c0::EQUAL_SIGN,std::string("="),3,9,3,10),
//		c0::Token(c0::DIVISION_SIGN,std::string("/"),3,11,3,12),
//		c0::Token(c0::EQUAL_SIGN,std::string("="),3,12,3,13),
//		c0::Token(c0::EQUAL_SIGN,std::string("="),3,14,3,15),
//		c0::Token(c0::EQUAL_SIGN,std::string("="),3,15,3,16),
//		c0::Token(c0::DIVISION_SIGN,std::string("/"),4,0,4,1),
//		c0::Token(c0::DIVISION_SIGN,std::string("/"),4,1,4,2),
//		c0::Token(c0::MULTIPLICATION_SIGN,std::string("*"),5,0,5,1),
//		c0::Token(c0::MULTIPLICATION_SIGN,std::string("*"),5,1,5,2),
//		c0::Token(c0::DIVISION_SIGN,std::string("/"),6,0,6,1),
//		c0::Token(c0::MULTIPLICATION_SIGN,std::string("*"),6,1,6,2),
//		c0::Token(c0::MULTIPLICATION_SIGN,std::string("*"),6,2,6,3),
//		c0::Token(c0::DIVISION_SIGN,std::string("/"),6,3,6,4),
//		c0::Token(c0::DIVISION_SIGN,std::string("/"),7,0,7,1),
//		c0::Token(c0::MULTIPLICATION_SIGN,std::string("*"),7,1,7,2),
//		c0::Token(c0::DIVISION_SIGN,std::string("/"),7,2,7,3),
//	};
//	auto result = tkz.AllTokens();
//
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	// for (auto i : result.first) {
//	// std::cout << i.GetType() << '\t' << "start " << i.GetStartPos().first << '\t' << i.GetStartPos().second \
//	// 		<< "\tend " << i.GetEndPos().first << '\t' << i.GetEndPos().second << '\t' << i.GetValueString() << std::endl;
//	// }
//
//	REQUIRE(result.first == output);
//}
//
//TEST_CASE("parentheses") {
//	std::string input =
//		"(\n"
//		")\n"
//		"(()))(";
//	std::stringstream ss;
//	ss.str(input);
//	c0::Tokenizer tkz(ss);
//	std::vector<c0::Token> output = {
//		c0::Token(c0::LEFT_BRACKET,std::string("("),0,0,0,1),
//		c0::Token(c0::RIGHT_BRACKET,std::string(")"),1,0,1,1),
//		c0::Token(c0::LEFT_BRACKET,std::string("("),2,0,2,1),
//		c0::Token(c0::LEFT_BRACKET,std::string("("),2,1,2,2),
//		c0::Token(c0::RIGHT_BRACKET,std::string(")"),2,2,2,3),
//		c0::Token(c0::RIGHT_BRACKET,std::string(")"),2,3,2,4),
//		c0::Token(c0::RIGHT_BRACKET,std::string(")"),2,4,2,5),
//		c0::Token(c0::LEFT_BRACKET,std::string("("),2,5,2,6),
//	};
//	auto result = tkz.AllTokens();
//
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	// for (auto i : result.first) {
//	// std::cout << i.GetType() << '\t' << "start " << i.GetStartPos().first << '\t' << i.GetStartPos().second \
//	// 		<< "\tend " << i.GetEndPos().first << '\t' << i.GetEndPos().second << '\t' << i.GetValueString() << std::endl;
//	// }
//
//	REQUIRE(result.first == output);
//}
//
//TEST_CASE("semicolons") {
//	std::string input =
//		";\n"
//		";;;";
//	std::stringstream ss;
//	ss.str(input);
//	c0::Tokenizer tkz(ss);
//	std::vector<c0::Token> output = {
//		c0::Token(c0::SEMICOLON,std::string(";"),0,0,0,1),
//		c0::Token(c0::SEMICOLON,std::string(";"),1,0,1,1),
//		c0::Token(c0::SEMICOLON,std::string(";"),1,1,1,2),
//		c0::Token(c0::SEMICOLON,std::string(";"),1,2,1,3),
//	};
//	auto result = tkz.AllTokens();
//
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	// for (auto i : result.first) {
//	// 	std::cout << i.GetType() << '\t' << "start " << i.GetStartPos().first << '\t' << i.GetStartPos().second \
//	// 		<< "\tend " << i.GetEndPos().first << '\t' << i.GetEndPos().second << '\t' << i.GetValueString() << std::endl;
//	// }
//
//	REQUIRE(result.first == output);
//}
//
//TEST_CASE("test") {
//	std::string input =
//		"var intMin = -2147483647-1;\n"
//		"const intMIN = - 2147483647 - 1;";
//	std::stringstream ss;
//	ss.str(input);
//	c0::Tokenizer tkz(ss);
//	std::vector<c0::Token> output = {
//		c0::Token(c0::VAR,std::string("var"),0,0,0,3),
//		c0::Token(c0::IDENTIFIER,std::string("intMin"),0,4,0,10),
//		c0::Token(c0::EQUAL_SIGN,std::string("="),0,11,0,12),
//		c0::Token(c0::MINUS_SIGN,std::string("-"),0,13,0,14),
//		c0::Token(c0::UNSIGNED_INTEGER,std::string("2147483647"),0,14,0,24),
//		c0::Token(c0::MINUS_SIGN,std::string("-"),0,24,0,25),
//		c0::Token(c0::UNSIGNED_INTEGER,std::string("1"),0,25,0,26),
//		c0::Token(c0::SEMICOLON,std::string(";"),0,26,0,27),
//		c0::Token(c0::CONST,std::string("const"),1,0,1,5),
//		c0::Token(c0::IDENTIFIER,std::string("intMIN"),1,6,1,12),
//		c0::Token(c0::EQUAL_SIGN,std::string("="),1,13,1,14),
//		c0::Token(c0::MINUS_SIGN,std::string("-"),1,15,1,16),
//		c0::Token(c0::UNSIGNED_INTEGER,std::string("2147483647"),1,17,1,27),
//		c0::Token(c0::MINUS_SIGN,std::string("-"),1,28,1,29),
//		c0::Token(c0::UNSIGNED_INTEGER,std::string("1"),1,30,1,31),
//		c0::Token(c0::SEMICOLON,std::string(";"),1,31,1,32),
//	};
//	auto result = tkz.AllTokens();
//
//	if (result.second.has_value()) {
//		FAIL();
//	}
//
//	// for (auto i : result.first) {
//	// 	std::cout << i.GetType() << '\t' << "start " << i.GetStartPos().first << '\t' << i.GetStartPos().second \
//	// 		<< "\tend " << i.GetEndPos().first << '\t' << i.GetEndPos().second << '\t' << i.GetValueString() << std::endl;
//	// }
//
//	REQUIRE(result.first == output);
//}
//
//
//
///* invalid */
//TEST_CASE("invalid_characters") {
//	std::string input =
//		"!\"#$%&',.:<>?@[\\]^_`{|}~";
//	std::stringstream ss;
//	ss.str(input);
//	c0::Tokenizer tkz(ss);
//	std::vector<c0::Token> output = {};
//	auto result = tkz.AllTokens();
//
//	if (!result.second.has_value()) {
//		FAIL();
//	}
//
//	REQUIRE(result.second.value().GetCode() == c0::ErrInvalidInput);
//}
//
//TEST_CASE("toooooo_big_integers") {
//	std::string input =
//		"2147483648\n"
//		"4000000000\n"
//		"18446744073709551616\n"
//		"1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
//	std::stringstream ss;
//	ss.str(input);
//	c0::Tokenizer tkz(ss);
//	std::vector<c0::Token> output = {};
//	auto result = tkz.AllTokens();
//
//	if (!result.second.has_value()) {
//		FAIL();
//	}
//
//	REQUIRE(result.second.value().GetCode() == c0::ErrIntegerOverflow);
//}