#include "analyser.h"

#include <climits>

namespace cc0 {
	std::pair<Output, std::optional<CompilationError>> Analyser::Analyse() {
		auto err = analyseProgram();
		if (err.has_value())
			return std::make_pair(_output, err);
		else
			return std::make_pair(_output, std::optional<CompilationError>());
	}

	// <C0-program> :: = { <variable-declaration> } {<function-definition>}
	// <variable-declaration> ::= [<const-qualifier>] <type-specifier> <identifier> [<initializer>]{','<init-declarator>}';'
	// <function-definition>  ::=					  <type-specifier> <identifier> '(' [<parameter-declaration-list>] ')'<compound-statement>
	std::optional<CompilationError> Analyser::analyseProgram() {
		std::optional<Token> next;
		std::optional<CompilationError> err;
		bool isInFirst = true;	//判断目前是不是在 { <variable-declaration> } 中
		TokenType type;
		while(isInFirst) {
			next = nextToken();
			if (!next.has_value())	//文件为空，直接返回
				return {};
			type = next.value().GetType();
			if(type == TokenType::CONST) { //确定是 <variable-declaration>
				unreadToken();
				err = analyseVariableDeclaration();
				if (err.has_value())
					return err;
			}
			else if(type == TokenType::VOID || type == TokenType::INT) { // <type-specifier>
				//<identifier>
				next = nextToken();
				if(!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER) { // error
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
				}

				//judge <variable-declaration> or <function-definition>
				next = nextToken();
				if(!next.has_value()) {	// error
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
				}
				if(next.value().GetType() == TokenType::LEFT_PARENTHESIS) {	//确认是 <function-definition>
					isInFirst = false;
					unreadToken();
					unreadToken();
					unreadToken();
					err = analyseFunctionDeclaration();
					if (err.has_value())
						return err;
				}else {	//确认是 <variable-declaration>
					unreadToken();
					unreadToken();
					unreadToken();
					err = analyseVariableDeclaration();
					if (err.has_value())
						return err;
				}
			}else {// error
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
			}
		}

		while(true) {	// <function-definition>
			next = nextToken();
			if (!next.has_value())	//文件为空，直接返回
				return {};
			unreadToken();
			err = analyseFunctionDeclaration();
			if (err.has_value())
				return err;
		}
		return {};
	}

	// <variable-declaration> ::= 
	// [<const-qualifier>]<type-specifier> < init-declarator-list>';'
	// <type-specifier>         ::= <simple-type-specifier>
	// <simple-type-specifier>  :: = 'void' | 'int'
	// <const-qualifier>        ::= 'const'
	// 变量声明，包括const，放入.constants中，另外，需要分配内存
	// TODO 考虑类型转化，函数return等问题
	std::optional<CompilationError> Analyser::analyseVariableDeclaration() {
		bool isConst = false;	//记录是否存在const
		
		// const
		// TODO const修饰的变量必须被显式初始化
		auto next = nextToken(); // [<const-qualifier>]
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		if(next.value().GetType() == TokenType::CONST) {
			next = nextToken();
			isConst = true;
		}

		// <type-specifier>
		if (!next.has_value() || (next.value().GetType() != TokenType::VOID && next.value().GetType() != TokenType::INT))
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedTypeSpecifier);
		if(next.value().GetType() == TokenType::VOID) {
			// void 不能参与变量的声明 (void 和 const void 都不行)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVoidDeclaration);
		}

		if(next.value().GetType() != TokenType::INT) { //如果不考虑double，这里没有int就是error
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedTypeSpecifier);
		}

		// <init-declarator-list>
		auto err = analyseInitDeclaratorList(isConst);
		if (err.has_value())
			return err;

		// ';'
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
		
		return {};
	}

	// <init-declarator-list> ::= <init-declarator>{',' <init-declarator> }
	// <init-declarator> ::= <identifier>[<initializer>]
	// <initializer> ::= '=' < expression >
	// TODO 生成汇编
	std::optional<CompilationError> Analyser::analyseInitDeclaratorList(const bool isConst) {
		// <init-declarator>
		auto next = nextToken();
		if(!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
		if (isDeclared(next.value().GetValueString()))
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

		auto tmpIdentifier = next;	//此时不知道后面有没有等号，还不知道要把标识符添加到哪里
		
		next = nextToken(); //[<initializer>]
		if(!next.has_value() || next.value().GetType() != TokenType::EQUAL_SIGN) {
			unreadToken();
			if(isConst) {//为const结果没有等号初始化，error
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);
			}
			else {//不是const，也没有等号初始化
				addUninitializedVariable(tmpIdentifier.value());
				_output._start.emplace_back(Operation::snew, 1, 0);
			}
		}
			
		else {	//有等号，根据是否是const加入已经初始化的列表里面
			isConst ? addConstant(tmpIdentifier.value()) : addVariable(tmpIdentifier.value());
			_output._start.emplace_back(Operation::snew, 1, 0);

			//经过这一步就可以将上面的空间中填入具体的值
			auto err = analyseExpression(TableType::START_TYPE);
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
			if (isDeclared(next.value().GetValueString()))
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

			tmpIdentifier = next;	//此时不知道后面有没有等号，还不知道要把标识符添加到哪里

			next = nextToken(); //[<initializer>]
			if (!next.has_value() || next.value().GetType() != TokenType::EQUAL_SIGN) {
				unreadToken();
				if (isConst) {//为const结果没有等号初始化，error
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);
				}
				else {//不是const，也没有等号初始化，类似 int a; 在栈上为其分配内存空间
					addUninitializedVariable(tmpIdentifier.value());
					_output._start.emplace_back(Operation::snew, 1, 0);
				}
			}
			else {	//有等号，根据是否是const加入已经初始化的列表里面,并在栈上分配内存空间
				isConst ? addConstant(tmpIdentifier.value()) : addVariable(tmpIdentifier.value());
				_output._start.emplace_back(Operation::snew, 1, 0);
				
				//经过这一步就可以将上面的空间中填入具体的值
				auto err = analyseExpression(TableType::START_TYPE);
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

		auto type = next.value().GetType();	//记录下是void还是int

		// <identifier>
		next = nextToken();
		if(!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
		if (isDeclared(next.value().GetValueString()))
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

		if (type == TokenType::VOID) { 
			addVoidFunctions(next.value());
		}

		if (type == TokenType::INT) {
			addIntFunctions(next.value());
		}

		// 添加到常量表
		auto tmp_constants = Constants(Type::STRING_TYPE, next.value().GetValueString());
		_output._constants.emplace_back(tmp_constants);
		auto pos = getPos(_output._constants, tmp_constants);
		
		int32_t paramSize = 0;
		// <parameter-clause> 参数列表
		auto err = analyseParameterClause(paramSize);
		if (err.has_value())
			return err;

		// 添加到函数表
		_output._functions.emplace_back(pos, paramSize);

		// <compound-statement>
		err = analyseCompoundStatement();
		if (err.has_value())
			return err;
		
		return {};
	}

	// <parameter-clause> ::= '('[<parameter-declaration-list>] ')'
	//TODO 参数不可能是 void
	std::optional<CompilationError> Analyser::analyseParameterClause(int32_t& paramSize) {
		// (
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_PARENTHESIS)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrEOF);

		//[<parameter-declaration-list>]
		next = nextToken();
		if(!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteBrackets);
		if(next.value().GetType() != TokenType::RIGHT_PARENTHESIS) {
			unreadToken();
			auto err = analyseParameterDeclarationList(paramSize);
			if (err.has_value())
				return err;
		}
		
		// )
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_PARENTHESIS)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteBrackets);
		return {};
	}

	// <parameter-declaration-list> :: =<parameter-declaration>{ ',' <parameter-declaration> }
	std::optional<CompilationError> Analyser::analyseParameterDeclarationList(int32_t& paramSize) {
		//参数不能重复声明，每次调用先清空
		_parameterDeclarationList.clear();
		
		//<parameter-declaration>
		auto err = analyseParameterDeclaration();
		if (err.has_value())
			return err;
		paramSize++;	//这里只可能是int，故只加一，没有考虑 double

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
			paramSize++;
		}
		return {};
	}

	// <parameter-declaration> :: =[<const-qualifier>]<type-specifier><identifier>
	std::optional<CompilationError> Analyser::analyseParameterDeclaration() {
		// const的参数需要存到函数自己对应的地方，不能被更改
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

		if (next.value().GetType() == TokenType::VOID) { //error
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVoidParameterDeclaration);
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
	// 参数是为了判断将汇编代码加入到哪个符号表里
	// type表示符号表，index只在type=fun_n_type时有用，用来标识哪个函数
	std::optional<CompilationError> Analyser::analyseExpression(TableType type, int32_t index = -1){// TODO 此处可能会调用常量表中的内容
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

	void Analyser::addVoidFunctions(const Token& tk) {
		_add(tk, _void_funs);
	}

	void Analyser::addIntFunctions(const Token& tk) {
		_add(tk, _int_funs);
	}

	int32_t Analyser::getIndex(const std::string& s) {
		if (_uninitialized_vars.find(s) != _uninitialized_vars.end())
			return _uninitialized_vars[s];
		else if (_vars.find(s) != _vars.end())
			return _vars[s];
		else
			return _consts[s];
	}

	bool Analyser::isDeclared(const std::string& s) { //变量和函数重名也算重命名
		return isConstant(s) || isUninitializedVariable(s) || isInitializedVariable(s) || isVoidFun(s) || isIntFun(s);
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

	bool Analyser::isVoidFun(const std::string& s) {
		return _void_funs.find(s) != _void_funs.end();
	}

	bool Analyser::isIntFun(const std::string& s) {
		return _int_funs.find(s) != _int_funs.end();
	}

	int32_t Analyser::getPos(std::vector<Constants> source, const Constants& value) const {
		const auto iter = std::find(source.begin(), source.end(), value);
		if (iter != source.end())
			return iter - source.begin(); 
		DieAndPrint("can not find value in table");
	}

	//根据变量名找位置
	int32_t Analyser::getPos(std::vector<Constants> source, const std::string& value) const {
		try {
			for (auto iter = source.begin(); iter != source.end(); ++iter) {
				if(std::get<0>(iter->GetValue()) == value) {
					return iter - source.begin();
				}
			}
		}
		catch (const std::bad_variant_access&) {
			DieAndPrint("bad variant access");
		}	
		DieAndPrint("can not find value in table");
	}
	
	int32_t Analyser::getPos(std::vector<Functions> source, const Functions& value) const {
		const auto iter = std::find(source.begin(), source.end(), value);
		if (iter != source.end())
			return iter - source.begin();
		DieAndPrint("can not find value in table");
	}

	//根据变量名找位置
	int32_t Analyser::getPos(std::vector<Functions> source, const std::string& value) const {
		try {
			for (auto iter = source.begin(); iter != source.end(); ++iter) {
				if (std::get<0>(_output._constants[iter->GetNameIndex()].GetValue()) == value) {
					return iter - source.begin();
				}
			}
		}
		catch (const std::bad_variant_access&) {
			DieAndPrint("bad variant access");
		}
		DieAndPrint("can not find value in table");
	}
}