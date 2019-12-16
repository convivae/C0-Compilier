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
				"Line: {} Column: {} Type: {} Value: {}",
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
			std::string name;
			switch (p) {
			case cc0::ILL:
				name = "ILL";
				break;
			case cc0::ADD:
				name = "ADD";
				break;
			case cc0::SUB:
				name = "SUB";
				break;
			case cc0::MUL:
				name = "MUL";
				break;
			case cc0::DIV:
				name = "DIV";
				break;
			case cc0::WRT:
				name = "WRT";
				break;
			case cc0::LIT:
				name = "LIT";
				break;
			case cc0::LOD:
				name = "LOD";
				break;
			case cc0::STO:
				name = "STO";
				break;
			}
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
			case cc0::ILL:
			case cc0::ADD:
			case cc0::SUB:
			case cc0::MUL:
			case cc0::DIV:
			case cc0::WRT:
				return format_to(ctx.out(), "{}", p.GetOperation());
			case cc0::LIT:
			case cc0::LOD:
			case cc0::STO:
				return format_to(ctx.out(), "{} {}", p.GetOperation(), p.GetX());
			}
			return format_to(ctx.out(), "ILL");
		}
	};
}