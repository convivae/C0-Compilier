#include "fmt/core.h"
#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"

namespace fmt {
	template<>
	struct formatter<cc0::ErrorCode> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const cc0::ErrorCode &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
			case cc0::ErrNoError:
				name = "No error.";
				break;
			case cc0::ErrStreamError:
				name = "Stream error.";
				break;
			case cc0::ErrEOF:
				name = "EOF";
				break;
			case cc0::ErrInvalidInput:
				name = "The input is invalid.";
				break;
			case cc0::ErrInvalidIdentifier:
				name = "Identifier is invalid";
				break;
			case cc0::ErrIntegerOverflow:
				name = "The integer is too big(int64_t).";
				break;
			case cc0::ErrNeedTypeSpecifier:
				name = "Need a type specifier here";
				break;
			case cc0::ErrNeedIdentifier:
				name = "Need an identifier here.";
				break;
			case cc0::ErrConstantNeedValue:
				name = "The constant need a value to initialize.";
				break;
			case cc0::ErrNoSemicolon:
				name = "Zai? Wei shen me bu xie fen hao.";
				break;
			case cc0::ErrInvalidVariableDeclaration:
				name = "The declaration is invalid.";
				break;
			case cc0::ErrIncompleteExpression:
				name = "The expression is incomplete.";
				break;
			case cc0::ErrNotDeclared:
				name = "The variable or constant must be declared before being used.";
				break;
			case cc0::ErrAssignToConstant:
				name = "Trying to assign value to a constant.";
				break;
			case cc0::ErrDuplicateDeclaration:
				name = "The variable or constant has been declared.";
				break;
			case cc0::ErrNotInitialized:
				name = "The variable has not been initialized.";
				break;
			case cc0::ErrInvalidAssignment:
				name = "The assignment statement is invalid.";
				break;
			case cc0::ErrInvalidPrint:
				name = "The output statement is invalid.";
				break;
			case cc0::ErrIncompleteBrackets:
				name = "The brackets is incomplete.";
				break;
			case cc0::ErrIntFunReturnByVoid:
				name = "The Function must have a return value.";
				break;
			case cc0::ErrVoidFunReturnByInt:
				name = "Trying to return value in void function.";
				break;
			case cc0::ErrAssignToFunction:
				name = "Function name can't be assigned.";
				break;
			case cc0::ErrFunctionNotExist:
				name = "The function doesn't exist.";
				break;
			case cc0::ErrInvalidVoidDeclaration:
				name = "The declaration of variable can't be void.";
				break;
			case cc0::ErrInvalidVoidParameterDeclaration:
				name = "The parameter of a function can't be void.";
				break;
			case cc0::ErrParameterMismatch:
				name = "The parameter number doesn't match.";
				break;
			case cc0::ErrUnexpectedError:
				name = "Unexpected Error";
				break;
			case cc0::ErrNeedReturnExpression:
				name = "Need a return expression here.";
				break;
			case cc0::ErrCallFunctionInGlobalArea:
				name = "can't call a function in global area.";
				break;
			case cc0::ErrNeedMainFunction:
				name = "Zai? Wei shen me bu xie main han shu.";
				break;
			case cc0::ErrDectedFloatingPointNumber:
				name = "Not support for floating number now";
				break;
			case cc0::ErrNeedStatement:
				name = "Need a statement here";
				break;
			case cc0::ErrFunctionCanNotBeVoid:
				name = "Void Function can't be computed.";
				break;
			case cc0::ErrIncompleteChar:
				name = "The char value is incorrect.";
				break;
			case cc0::ErrIncompleteString:
				name = "The string value is incorrect.";
				break;
			case cc0::ErrNotSupportNow:
				name = "The string value is not support now.";
				break;
			case cc0::ErrIncompleteNotes:
				name = "The note is incomplete";
				break;
			case cc0::ErrIncompletePrimaryExpression:
				name = "The Primary Expression is incomplete";
				break;
			case cc0::ErrDuplicateMainFunction:
				name = "The main function is duplicate";
				break;
			}
			return format_to(ctx.out(), name);
		}
	};

	template<>
	struct formatter<cc0::CompilationError> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const cc0::CompilationError &p, FormatContext &ctx) {
			return format_to(ctx.out(), "Line: {} Column: {} Error: {}", p.GetPos().first, p.GetPos().second, p.GetCode());
		}
	};
}

namespace fmt {
	template<>
	struct formatter<cc0::Token> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const cc0::Token &p, FormatContext &ctx) {
			return format_to(ctx.out(),
				"Line: {}\tColumn: {}\tType: {}\t \tValue:\t{}",
				p.GetStartPos().first, p.GetStartPos().second, p.GetType(), p.GetValueString());
		}
	};

	template<>
	struct formatter<cc0::TokenType> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const cc0::TokenType &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
			case cc0::NULL_TOKEN:
				name = "NullToken";
				break;
			case cc0::DECIMAL_INTEGER:
				name = "DecimalInteger";
				break;
			case cc0::HEXADECIMAL_INTEGER:
				name = "HexDecimalInteger";
				break;
			case cc0::FLOATING_POINT:
				name = "FloatingPoint";
				break;
			case cc0::IDENTIFIER:
				name = "Identifier";
				break;
			case cc0::PLUS_SIGN:
				name = "PlusSign";
				break;
			case cc0::MINUS_SIGN:
				name = "MinusSign";
				break;
			case cc0::MULTIPLICATION_SIGN:
				name = "MultiplicationSign";
				break;
			case cc0::DIVISION_SIGN:
				name = "DivisionSign";
				break;
			case cc0::SEMICOLON:
				name = "Semicolon";
				break;
			case cc0::COMMA:
				name = "Comma";
				break;
			case cc0::LEFT_PARENTHESIS:
				name = "LeftParenthesis";
				break;
			case cc0::RIGHT_PARENTHESIS:
				name = "RightParenthesis";
				break;
			case cc0::LEFT_BRACE:
				name = "LeftBrace";
				break;
			case cc0::RIGHT_BRACE:
				name = "RightBrace";
				break;
			case cc0::EQUAL_SIGN:
				name = "EqualSign";
				break;
			case cc0::LESS_SIGN:
				name = "LessSign";
				break;
			case cc0::ABOVE_SIGN:
				name = "AboveSign";
				break;
			case cc0::EXCLAMATION_SIGN:
				name = "ExclamationSign";
				break;
			case cc0::LESS_EQUAL_SIGN:
				name = "LessEqualSign";
				break;
			case cc0::ABOVE_EQUAL_SIGN:
				name = "AboveEqualSign";
				break;
			case cc0::EQUAL_EQUAL_SIGN:
				name = "EqualEqualSign";
				break;
			case cc0::NOT_EQUAL_SIGN:
				name = "NotEqualSign";
				break;
			case cc0::CONST:
				name = "Const";
				break;
			case cc0::VOID:
				name = "Void";
				break;
			case cc0::INT:
				name = "Int";
				break;
			case cc0::CHAR:
				name = "Char";
				break;
			case cc0::DOUBLE:
				name = "Double";
				break;
			case cc0::STRUCT:
				name = "Struct";
				break;
			case cc0::IF:
				name = "If";
				break;
			case cc0::ELSE:
				name = "Else";
				break;
			case cc0::SWITCH:
				name = "Switch";
				break;
			case cc0::CASE:
				name = "Case";
				break;
			case cc0::DEFAULT:
				name = "Default";
				break;
			case cc0::WHILE:
				name = "While";
				break;
			case cc0::FOR:
				name = "For";
				break;
			case cc0::DO:
				name = "Do";
				break;
			case cc0::RETURN:
				name = "Return";
				break;
			case cc0::BREAK:
				name = "Break";
				break;
			case cc0::CONTINUE:
				name = "Continue";
				break;
			case cc0::PRINT:
				name = "Print";
				break;
			case cc0::SCAN:
				name = "Scan";
				break;
			case cc0::CHAR_SIGN:
				name = "Char";
				break;
			case cc0::STRING_SIGN:
				name = "String";
				break;
			}
			return format_to(ctx.out(), name);
		}
	};
}

namespace fmt {
	template<>
	struct formatter<cc0::Operation> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const cc0::Operation &p, FormatContext &ctx) {
			const auto& name = cc0::nameOfOpCode.at(p);
			return format_to(ctx.out(), name);
		}
	};
	template<>
	struct formatter<cc0::Instruction> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const cc0::Instruction &p, FormatContext &ctx) {
			std::string name;
			switch (p.GetOperation())
			{
			case cc0::nop:
			case cc0::pop:
			case cc0::pop2:
			case cc0::dup:
			case cc0::dup2:
			case cc0::_new:
			case cc0::iload:
			case cc0::dload:
			case cc0::aload:
			case cc0::iaload:
			case cc0::daload:
			case cc0::aaload:
			case cc0::istore:
			case cc0::dstore:
			case cc0::astore:
			case cc0::iastore:
			case cc0::dastore:
			case cc0::aastore:
			case cc0::iadd:
			case cc0::dadd:
			case cc0::isub:
			case cc0::dsub:
			case cc0::imul:
			case cc0::dmul:
			case cc0::idiv:
			case cc0::ddiv:
			case cc0::ineg:
			case cc0::dneg:
			case cc0::icmp:
			case cc0::dcmp:
			case cc0::i2d:
			case cc0::d2i:
			case cc0::i2c:
			case cc0::ret:
			case cc0::iret:
			case cc0::dret:
			case cc0::aret:
			case cc0::iprint:
			case cc0::dprint:
			case cc0::cprint:
			case cc0::sprint:
			case cc0::printl:
			case cc0::iscan:
			case cc0::dscan:
			case cc0::cscan:
				return format_to(ctx.out(), "{}", p.GetOperation());
			case cc0::bipush:
			case cc0::ipush:
			case cc0::popn:
			case cc0::loadc:
			case cc0::snew:
			case cc0::jmp:
			case cc0::je:
			case cc0::jne:
			case cc0::jl:
			case cc0::jge:
			case cc0::jg:
			case cc0::jle:
			case cc0::call:
				return format_to(ctx.out(), "{} {}", p.GetOperation(), p.GetX());
			case cc0::loada:
				return format_to(ctx.out(), "{} {}, {}", p.GetOperation(), p.GetX(),p.GetY());
			}
			return format_to(ctx.out(), "nop");
		}
	};
}