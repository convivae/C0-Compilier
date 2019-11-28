#include "fmt/core.h"
#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"

namespace fmt {
	template<>
	struct formatter<c0::ErrorCode> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const c0::ErrorCode &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
			case c0::ErrNoError:
				name = "No error.";
				break;
			case c0::ErrStreamError:
				name = "Stream error.";
				break;
			case c0::ErrEOF:
				name = "EOF";
				break;
			case c0::ErrInvalidInput:
				name = "The input is invalid.";
				break;
			case c0::ErrInvalidIdentifier:
				name = "Identifier is invalid";
				break;
			case c0::ErrIntegerOverflow:
				name = "The integer is too big(int64_t).";
				break;
			case c0::ErrNoBegin:
				name = "The program should start with 'begin'.";
				break;
			case c0::ErrNoEnd:
				name = "The program should end with 'end'.";
				break;
			case c0::ErrNeedIdentifier:
				name = "Need an identifier here.";
				break;
			case c0::ErrConstantNeedValue:
				name = "The constant need a value to initialize.";
				break;
			case c0::ErrNoSemicolon:
				name = "Zai? Wei shen me bu xie fen hao.";
				break;
			case c0::ErrInvalidVariableDeclaration:
				name = "The declaration is invalid.";
				break;
			case c0::ErrIncompleteExpression:
				name = "The expression is incomplete.";
				break;
			case c0::ErrNotDeclared:
				name = "The variable or constant must be declared before being used.";
				break;
			case c0::ErrAssignToConstant:
				name = "Trying to assign value to a constant.";
				break;
			case c0::ErrDuplicateDeclaration:
				name = "The variable or constant has been declared.";
				break;
			case c0::ErrNotInitialized:
				name = "The variable has not been initialized.";
				break;
			case c0::ErrInvalidAssignment:
				name = "The assignment statement is invalid.";
				break;
			case c0::ErrInvalidPrint:
				name = "The output statement is invalid.";
				break;
			}
			return format_to(ctx.out(), name);
		}
	};

	template<>
	struct formatter<c0::CompilationError> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const c0::CompilationError &p, FormatContext &ctx) {
			return format_to(ctx.out(), "Line: {} Column: {} Error: {}", p.GetPos().first, p.GetPos().second, p.GetCode());
		}
	};
}

namespace fmt {
	template<>
	struct formatter<c0::Token> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const c0::Token &p, FormatContext &ctx) {
			return format_to(ctx.out(),
				"Line: {} Column: {} Type: {} Value: {}",
				p.GetStartPos().first, p.GetStartPos().second, p.GetType(), p.GetValueString());
		}
	};

	template<>
	struct formatter<c0::TokenType> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const c0::TokenType &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
			case c0::NULL_TOKEN:
				name = "NullToken";
				break;
			case c0::UNSIGNED_INTEGER:
				name = "UnsignedInteger";
				break;
			case c0::IDENTIFIER:
				name = "Identifier";
				break;
			case c0::BEGIN:
				name = "Begin";
				break;
			case c0::END:
				name = "End";
				break;
			case c0::VAR:
				name = "Var";
				break;
			case c0::CONST:
				name = "Const";
				break;
			case c0::PRINT:
				name = "Print";
				break;
			case c0::PLUS_SIGN:
				name = "PlusSign";
				break;
			case c0::MINUS_SIGN:
				name = "MinusSign";
				break;
			case c0::MULTIPLICATION_SIGN:
				name = "MultiplicationSign";
				break;
			case c0::DIVISION_SIGN:
				name = "DivisionSign";
				break;
			case c0::EQUAL_SIGN:
				name = "EqualSign";
				break;
			case c0::SEMICOLON:
				name = "Semicolon";
				break;
			case c0::LEFT_BRACKET:
				name = "LeftBracket";
				break;
			case c0::RIGHT_BRACKET:
				name = "RightBracket";
				break;
			}
			return format_to(ctx.out(), name);
		}
	};
}

namespace fmt {
	template<>
	struct formatter<c0::Operation> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const c0::Operation &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
			case c0::ILL:
				name = "ILL";
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
				name = "LIT";
				break;
			case c0::LOD:
				name = "LOD";
				break;
			case c0::STO:
				name = "STO";
				break;
			}
			return format_to(ctx.out(), name);
		}
	};
	template<>
	struct formatter<c0::Instruction> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const c0::Instruction &p, FormatContext &ctx) {
			std::string name;
			switch (p.GetOperation())
			{
			case c0::ILL:
			case c0::ADD:
			case c0::SUB:
			case c0::MUL:
			case c0::DIV:
			case c0::WRT:
				return format_to(ctx.out(), "{}", p.GetOperation());
			case c0::LIT:
			case c0::LOD:
			case c0::STO:
				return format_to(ctx.out(), "{} {}", p.GetOperation(), p.GetX());
			}
			return format_to(ctx.out(), "ILL");
		}
	};
}