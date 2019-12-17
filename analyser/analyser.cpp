#include "analyser.h"

#include <climits>

namespace cc0 {
	std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyser::Analyse() {
		auto err = analyseProgram();
		if (err.has_value())
			return std::make_pair(std::vector<Instruction>(), err);
		else
			return std::make_pair(_instructions, std::optional<CompilationError>());
	}

	// <C0-program> :: = { <variable-declaration> } {<function-definition>}
	// <variable-declaration> ::= [<const-qualifier>] <type-specifier> <identifier> [<initializer>]{','<init-declarator>}';'
	// <function-definition>  ::=					  <type-specifier> <identifier> '(' [<parameter-declaration-list>] ')'<compound-statement>
	std::optional<CompilationError> Analyser::analyseProgram() {
		std::optional<Token> next;
		TokenType type;
		while(true) {
			next = nextToken();
			if (!next.has_value())	//文件为空，直接返回
				return {};
			type = next.value().GetType();
			if(type == TokenType::CONST) { //确定是 <variable-declaration>
				
			}
			else if(type == TokenType::VOID || type == TokenType::INT) { // <type-specifier>
				//<identifier>
				next = nextToken();
				if(!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER) { // error
					
				}

				//judge <variable-declaration> or <function-definition>
				next = nextToken();
				if(!next.has_value()) {	// error
					
				}
				if(next.value().GetType() == TokenType::LEFT_PARENTHESIS) {	//确认是 
					
				}

				
			}else {// error
				
			}
		}

		while(true) {
			
		}
		return {};
	}

	// <variable-declaration> ::= 
	// [<const-qualifier>]<type-specifier> < init-declarator-list>';'
	// <type-specifier>         ::= <simple-type-specifier>
	// <simple - type - specifier>  :: = 'void' | 'int'
	// <const-qualifier>        ::= 'const'
	std::optional<CompilationError> Analyser::analyseVariableDeclaration() {
		//const
		auto next = nextToken(); // [<const-qualifier>]
		if(!next.has_value() && next.value().GetType() == TokenType::CONST) {
			next = nextToken();
		}

		// <type-specifier>
		if (!next.has_value() || (next.value().GetType() != TokenType::VOID && next.value().GetType() != TokenType::INT))
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedTypeSpecifier);
		if(next.value().GetType() != TokenType::VOID) {
			//TODO
			//生成编译语句
		}
		else if(next.value().GetType() != TokenType::INT) {
			//TODO
			//生成编译语句
		}
		auto err = analyseInitDeclaratorList();
		if (err.has_value())
			return err;

		// ';'
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
		
		return {};
	}

	// <init-declarator-list> ::= 
	// <init-declarator>{',' <init-declarator> }
	// <init-declarator> ::= <identifier>[<initializer>]
	// <initializer> ::= '=' < expression >
	// TODO 生成汇编
	std::optional<CompilationError> Analyser::analyseInitDeclaratorList() {
		// <init-declarator>
		auto next = nextToken();
		if(!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
		
		next = nextToken(); //[<initializer>]
		if(!next.has_value() || next.value().GetType() != TokenType::EQUAL_SIGN)
			unreadToken();
		else {
			auto err = analyseExpression();
			if (err.has_value())
				return err;
		}

		// {',' <init-declarator> }
		while(true) {
			next = nextToken();
			if (!next.has_value())
				return {};
			if (next.value().GetType() != TokenType::COMMA) {
				unreadToken();
				return {};
			}

			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);

			next = nextToken(); //[<initializer>]
			if (!next.has_value() || next.value().GetType() != TokenType::EQUAL_SIGN)
				unreadToken();
			else {
				auto err = analyseExpression();
				if (err.has_value())
					return err;
			}
		}
		return {};
	}

	// <function-definition> ::= 
	// <type-specifier><identifier><parameter-clause><compound-statement>

	std::optional<CompilationError> Analyser::analyseFunctionDeclaration() {
		// <type-specifier>
		auto next = nextToken();
		if (!next.has_value() || (next.value().GetType() != TokenType::VOID && next.value().GetType() != TokenType::INT))
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedTypeSpecifier);

		// <identifier>
		next = nextToken();
		if(!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);

		// <parameter-clause>
		auto err = analyseParameterClause();
		if (err.has_value())
			return err;

		// <compound-statement>
		
		
		return {};
	}

	// <parameter-clause> ::= '('[<parameter-declaration-list>] ')'
	std::optional<CompilationError> Analyser::analyseParameterClause() {
		// (
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_PARENTHESIS)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteBrackets);

		//[<parameter-declaration-list>]
		next = nextToken();
		if(!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrEOF);
		if(next.value().GetType() != TokenType::RIGHT_PARENTHESIS) {
			unreadToken();
			auto err = analyseParameterDeclarationList();
			if (err.has_value())
				return err;
		}
		
		// )
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_PARENTHESIS)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteBrackets);
		return {};
	}

	// <parameter-declaration-list > :: =<parameter-declaration>{ ',' <parameter-declaration> }
	std::optional<CompilationError> Analyser::analyseParameterDeclarationList() {
		//<parameter-declaration>
		auto err = analyseParameterDeclaration();
		if (err.has_value())
			return err;

		//{ ',' <parameter-declaration> }
		while (true) {
			// ,
			auto next = nextToken();
			if (!next.has_value())
				return {};
			if (next.value().GetType() != TokenType::COMMA) {
				unreadToken();
				return {};
			}
			err = analyseParameterDeclaration();
			if (err.has_value())
				return err;
		}
		return {};
	}

	// <parameter-declaration> :: =[<const-qualifier>]<type-specifier><identifier>
	std::optional<CompilationError> Analyser::analyseParameterDeclaration() {
		//const
		auto  next = nextToken();
		if (next.has_value() && next.value().GetType() == TokenType::CONST) {
			next = nextToken();
		}
		else {
			unreadToken();
			return {};
		}

		// <type-specifier><identifier>
		if (!next.has_value() || (next.value().GetType() != TokenType::VOID && next.value().GetType() != TokenType::INT))
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedTypeSpecifier);

		if (next.value().GetType() == TokenType::VOID) {
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
			}
			//TODO
			//生成编译语句
		}
		else if (next.value().GetType() == TokenType::INT) {
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
			}
			//TODO
			//生成编译语句
		}
		
		return {};
	}

	// <compound-statement> ::= 
	// '{' {<variable-declaration>} <statement-seq> '}'
	std::optional<CompilationError> Analyser::analyseCompoundStatement() {
		// {
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACE)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteBrackets);

		// {<variable-declaration>}
		while (true) {
			next = nextToken();
			if (!next.has_value())
				return {};
			auto type = next.value().GetType();
			if (type != TokenType::CONST && type != TokenType::VOID && type != TokenType::INT) {
				unreadToken();
				break;
			}

			auto err = analyseVariableDeclaration();
			if (err.has_value())
				return err;
		}

		// <statement-seq>
		auto err = analyseStatementSeq();
		if (err.has_value())
			return err;

		// }
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACE)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteBrackets);
		
		return {};
	}

	//<statement-seq> ::= {<statement>}
	std::optional<CompilationError> Analyser::analyseStatementSeq() {
		while (true) {
			auto next = nextToken();
			if (!next.has_value())
				return {};

			//TODO 这里写的不对，没考虑一次也没有的情况
			auto err = analyseStatement();
			if (err.has_value())
				return err;
		}
		return {};
	}

	//<statement> ::= 
	// '{' < statement-seq > '}'
	// 	| <condition-statement>
	// 	| <loop-statement>
	// 	| <jump-statement>
	// 	| <print-statement>
	// 	| <scan-statement>
	// 	| <assignment-expression>';'
	// 	| <function-call>';'
	// 	| ';'
	std::optional<CompilationError> Analyser::analyseStatement() {
		auto next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidAssignment);
		std::optional<CompilationError> err;
		switch (next.value().GetType()) {
		case TokenType::LEFT_BRACE:
			err = analyseStatementSeq();
			if (err.has_value())
				return err;
			// }
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACE)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteBrackets);
			break;
		case TokenType::IF:
			unreadToken();
			err = analyseConditionStatement();
			if (err.has_value())
				return err;
			break;

		case TokenType::WHILE:
			unreadToken();
			err = analyseLoopStatement();
			if (err.has_value())
				return err;
			break;
		case TokenType::RETURN:
			unreadToken();
			err = analyseJumpStatement();
			if (err.has_value())
				return err;
			break;
		case TokenType::PRINT:
			unreadToken();
			err = analysePrintStatement();
			if (err.has_value())
				return err;
			break;
		case TokenType::SCAN:
			unreadToken();
			err = analyseScanStatement();
			if (err.has_value())
				return err;
			break;
		case TokenType::IDENTIFIER:
			next = nextToken();
			if (!next.has_value())
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidAssignment);
			if (next.value().GetType() == TokenType::EQUAL_SIGN) {
				unreadToken();
				unreadToken();
				err = analyseAssignmentExpression();
				if (err.has_value())
					return err;
				// ;
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
			}
			else if (next.value().GetType() == TokenType::LEFT_PARENTHESIS) {
				unreadToken();
				unreadToken();
				err = analyseFunctionCall();
				if (err.has_value())
					return err;
				// ;
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
			}
		case TokenType::SEMICOLON:
			//TODO 汇编语句
			break;
		default:
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidAssignment);
		}
		return {};
	}

	//<condition> ::= <expression>[<relational-operator><expression>]
	//<relational-operator> ::= '<' | '<=' | '>' | '>=' | '!=' | '=='
	std::optional<CompilationError> Analyser::analyseCondition() {
		auto err = analyseExpression();
		if (err.has_value())
			return err;

		// [<relational-operator><expression>]
		auto next = nextToken();
		if (!next.has_value())
			return {};
		auto type = next.value().GetType();
		switch (type) {
		case TokenType::LESS_SIGN:
			// TODO 分析出是 <，生成汇编
		
			break;
		case TokenType::LESS_EQUAL_SIGN:
			// TODO 分析出是 <=，生成汇编

			break;
		case TokenType::ABOVE_SIGN:
			// TODO 分析出是 >，生成汇编

			break;
		case TokenType::ABOVE_EQUAL_SIGN:
			// TODO 分析出是 >=，生成汇编

			break;
		case TokenType::NOT_EQUAL_SIGN:
			// TODO 分析出是 !=，生成汇编

			break;
		case TokenType::EQUAL_EQUAL_SIGN:
			// TODO 分析出是 ==，生成汇编
			break;
		default:	//说明没有可选部分
			unreadToken();
			return {};
		}

		err = analyseExpression();
		if (err.has_value())
			return err;
		
		return {};
	}

	// <assignment-expression> ::= <identifier><assignment-operator><expression>
	// <assignment-operator> ::= '='
	std::optional<CompilationError> Analyser::analyseAssignmentExpression() {
		// <identifier>
		auto next = nextToken();
		if(!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);

		// <assignment-operator>
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::EQUAL_SIGN)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		// <expression>
		auto err = analyseExpression();
		if (err.has_value())
			return err;
		return {};
	}

	// <scan-statement> ::= 'scan' '(' <identifier> ')' ';'
	std::optional<CompilationError> Analyser::analyseScanStatement() {
		// scan
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SCAN)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		// (
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_PARENTHESIS)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		// <identifier>
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);

		// )
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_PARENTHESIS)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteBrackets);

		// ;
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
		//TODO 生成汇编
		return {};
	}

	//<print-statement> ::= 'print' '('[<printable-list>] ')' ';'
	std::optional<CompilationError> Analyser::analysePrintStatement() {
		// print
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::PRINT)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		// (
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_PARENTHESIS)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		// [<printable-list>]
		next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		else if(next.value().GetType() == TokenType::RIGHT_PARENTHESIS) {
			unreadToken();
		}
		else {
			auto err = analysePrintableList();
			if (err.has_value())
				return err;
		}
		
		// )
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_PARENTHESIS)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteBrackets);

		// ;
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
		//TODO 生成汇编
		return {};
	}

	// <printable-list>  ::= <printable> {',' < printable > }
	std::optional<CompilationError> Analyser::analysePrintableList(){
		//<printable>
		auto err = analysePrintable();
		if (err.has_value())
			return err;

		// {',' <printable> }
		while (true) {
			auto next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::COMMA)
				return {};
			err = analysePrintable();
			if (err.has_value())
				return err;
		}
		
		return {};
	}

	// <printable> :: = <expression>
	std::optional<CompilationError> Analyser::analysePrintable() {
		auto err = analyseExpression();
		if (err.has_value())
			return err;
		//TODO 汇编在哪生成
		return {};
	}

	//<jump-statement> ::= <return-statement>
	std::optional<CompilationError> Analyser::analyseJumpStatement() {
		auto err = analyseReturnStatement();
		if (err.has_value())
			return err;
		return {};
	}

	//<return-statement> ::= 'return'[<expression>] ';'
	std::optional<CompilationError> Analyser::analyseReturnStatement() {
		// return
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RETURN)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		// [<expression>] ';'
		next = nextToken();
		if(!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		if(next.value().GetType() != TokenType::SEMICOLON) { //说明存在可选项
			auto err = analyseExpression();
			if (err.has_value())
				return err;
			// ;
			next = nextToken();
			if(!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
		}
		// TODO 此句分析成功 生成汇编
		
		
		return {};
	}

	// <loop-statement> ::= 
	// 'while' '(' <condition> ')' <statement>
	std::optional<CompilationError> Analyser::analyseLoopStatement() {
		// while
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::WHILE)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		
		// (
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_PARENTHESIS)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		// <condition>
		auto err = analyseCondition();
		if (err.has_value())
			return err;

		// )
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_PARENTHESIS)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteBrackets);

		// <statement>
		err = analyseStatement();
		if (err.has_value())
			return err;
		return {};
	}

	// <condition-statement> :: =
	// 'if' '(' < condition > ')' <statement> ['else' <statement>]
	std::optional<CompilationError> Analyser::analyseConditionStatement() {
		// if
		auto next = nextToken();
		if(!next.has_value() || next.value().GetType() != TokenType::IF)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		// (
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_PARENTHESIS)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		// <condition>
		auto err = analyseCondition();
		if (err.has_value())
			return err;

		// )
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_PARENTHESIS)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteBrackets);

		// <statement>
		err = analyseStatement();
		if (err.has_value())
			return err;

		// ['else' <statement> ]
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::ELSE)
			return {};

		// <statement>
		err = analyseStatement();
		if (err.has_value())
			return err;
		return  {};
	}

	// <expression> :: = <additive-expression>
	std::optional<CompilationError> Analyser::analyseExpression(){
		auto err = analyseAdditiveExpression();
		if (err.has_value())
			return err;
		return {};
	}

	// <additive-expression> :: =
	// 	<multiplicative-expression>{ <additive-operator><multiplicative-expression> }
	// 	<additive-operator>       ::= '+' | '-'
	std::optional<CompilationError> Analyser::analyseAdditiveExpression() {
		auto err = analyseMultiplicativeExpression();
		if (err.has_value())
			return err;

		// {<additive-operator><multiplicative-expression>}
		while (true) {
			auto next = nextToken();
			if (!next.has_value() || (next.value().GetType() != TokenType::PLUS_SIGN && next.value().GetType() != TokenType::MINUS_SIGN))
				return {};
			err = analyseMultiplicativeExpression();
			if (err.has_value())
				return err;
		}
		return {};
	}

	// <multiplicative-expression> ::= 
	// <unary-expression>{<multiplicative-operator><unary-expression>}
	std::optional<CompilationError> Analyser::analyseMultiplicativeExpression() {
		auto err = analyseUnaryExpression();
		if (err.has_value())
			return err;

		// {<multiplicative-operator><unary-expression>}
		while (true) {
			auto next = nextToken();
			if (!next.has_value())
				return {};
			auto type = next.value().GetType();
			
			//存在可选式
			if(type == TokenType::LEFT_PARENTHESIS || type == TokenType::PLUS_SIGN || type == TokenType::MINUS_SIGN ||
				type == TokenType::IDENTIFIER || type == TokenType::DECIMAL_INTEGER || type == TokenType::HEXADECIMAL_INTEGER) {
				unreadToken();
				auto err = analyseMultiplicativeExpression();
				if (err.has_value())
					return err;

				err = analyseUnaryExpression();
				if (err.has_value())
					return err;
			}else {
				return {};
			}
		}
		return {};
	}

	//<unary-expression> ::= [<unary-operator>]<primary-expression>
	std::optional<CompilationError> Analyser::analyseUnaryExpression() {
		//[<unary-operator>]
		auto next = nextToken();
		if(!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		auto type = next.value().GetType();
		if(type != TokenType::PLUS_SIGN && type != TokenType::MINUS_SIGN) {//无可选式
			unreadToken();
		}
		auto err = analysePrimaryExpression();
		if (err.has_value())
			return err;
		return {};
	}

	//<primary-expression> ::=  
	// '(' <expression> ')'
	// 	| <identifier>
	// 	| <integer - literal>
	// 	| <function - call>
	std::optional<CompilationError> Analyser::analysePrimaryExpression() {
		auto next = nextToken();
		if(!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		std::optional<cc0::CompilationError> err;
		
		auto type = next.value().GetType();
		switch (type) {
		case TokenType::LEFT_PARENTHESIS:
			err = analyseExpression();
			if (err.has_value())
				return err;
			// )
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_PARENTHESIS)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteBrackets);
			break;

		case TokenType::IDENTIFIER://两种情况，<identifier> 和 <function-call>
			next = nextToken();

			//是 <identifier>
			if(!next.has_value() || next.value().GetType() != TokenType::LEFT_PARENTHESIS) {
				unreadToken();
				//TODO
				return {};
			}

			//是 <function-call>
			unreadToken();
			unreadToken();
			err = analyseFunctionCall();
			if (err.has_value())
				return err;
			
			//TODO
			break;
			
		//<integer-literal>
		case TokenType::DECIMAL_INTEGER:
		case TokenType::HEXADECIMAL_INTEGER:
			//TODO
			break;



		}
		return {};
	}

	// <function-call> :: =
	// 	<identifier> '('[<expression-list>] ')'
	std::optional<CompilationError> Analyser::analyseFunctionCall() {
		//<identifier>
		auto next = nextToken();
		if(!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		// (
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_PARENTHESIS)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		// [<expression-list>]
		next = nextToken();
		if(!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		// 存在可选项
		if (next.value().GetType() != TokenType::RIGHT_PARENTHESIS) {
			auto err = analyseExpressionList();
			if (err.has_value())
				return err;
		}
		else {
			unreadToken();
		}

		// )
		next = nextToken();
		if(!next.has_value() || next.value().GetType() != TokenType::RIGHT_PARENTHESIS)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteBrackets);

		//TODO
		return {};
	}

	// <expression-list> ::= <expression>{',' < expression > }
	std::optional<CompilationError> Analyser::analyseExpressionList() {
		auto err = analyseExpression();
		if (err.has_value())
			return err;

		// {',' < expression > }
		while (true) {
			auto next = nextToken();
			if(!next.has_value() || next.value().GetType() != TokenType::COMMA) {
				unreadToken();
				return {};
			}
			err = analyseExpression();
			if (err.has_value())
				return err;
			//TODO
		}
		return {};
	}

	std::optional<Token> Analyser::nextToken() {
		if (_offset == _tokens.size())
			return {};
		// 考虑到 _tokens[0..._offset-1] 已经被分析过了
		// 所以我们选择 _tokens[0..._offset-1] 的 EndPos 作为当前位置
		_current_pos = _tokens[_offset].GetEndPos();
		return _tokens[_offset++];
	}

	void Analyser::unreadToken() {
		if (_offset == 0)
			DieAndPrint("analyser unreads token from the begining.");
		_current_pos = _tokens[_offset - 1].GetEndPos();
		_offset--;
	}

	void Analyser::_add(const Token& tk, std::map<std::string, int32_t>& mp) {
		if (tk.GetType() != TokenType::IDENTIFIER)
			DieAndPrint("only identifier can be added to the table.");
		mp[tk.GetValueString()] = _nextTokenIndex;
		_nextTokenIndex++;
	}

	void Analyser::addVariable(const Token& tk) {
		_add(tk, _vars);
	}

	void Analyser::addConstant(const Token& tk) {
		_add(tk, _consts);
	}

	void Analyser::addUninitializedVariable(const Token& tk) {
		_add(tk, _uninitialized_vars);
	}

	int32_t Analyser::getIndex(const std::string& s) {
		if (_uninitialized_vars.find(s) != _uninitialized_vars.end())
			return _uninitialized_vars[s];
		else if (_vars.find(s) != _vars.end())
			return _vars[s];
		else
			return _consts[s];
	}

	bool Analyser::isDeclared(const std::string& s) {
		return isConstant(s) || isUninitializedVariable(s) || isInitializedVariable(s);
	}

	bool Analyser::isUninitializedVariable(const std::string& s) {
		return _uninitialized_vars.find(s) != _uninitialized_vars.end();
	}
	
	bool Analyser::isInitializedVariable(const std::string&s) {
		return _vars.find(s) != _vars.end();
	}

	bool Analyser::isConstant(const std::string&s) {
		return _consts.find(s) != _consts.end();
	}
}