#include "analyser.h"

#include <climits>

namespace cc0 {
	std::pair<Output, std::optional<CompilationError>> Analyser::Analyse() {
		auto err = analyseProgram();
		if (err.has_value())
			return std::make_pair(_output, err);

		//最后一步，检测有无 main 函数
		if (_main_index == -1) {
			err = std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedMainFunction);
			return std::make_pair(_output, err);
		}

		//无 error
		return std::make_pair(_output, std::optional<CompilationError>());
	}

	// <C0-program> :: = { <variable-declaration> } {<function-definition>}
	// <variable-declaration> ::= [<const-qualifier>] <type-specifier> <identifier> [<initializer>]{','<init-declarator>}';'
	// <function-definition>  ::=					  <type-specifier> <identifier> '(' [<parameter-declaration-list>] ')'<compound-statement>
	std::optional<CompilationError> Analyser::analyseProgram() {
		std::optional<Token> next;
		std::optional<CompilationError> err;
		bool isInFirst = true;    //判断目前是不是在 { <variable-declaration> } 中
		TokenType type;
		while (isInFirst) {
			next = nextToken();
			if (!next.has_value())    //文件为空，直接返回
				return {};
			type = next.value().GetType();
			if (type == TokenType::CONST) { //确定是 <variable-declaration>
				unreadToken();
				err = analyseVariableDeclaration(TableType::START_TYPE);
				if (err.has_value())
					return err;
			}
			else if (type == TokenType::VOID || type == TokenType::INT) { // <type-specifier>
			 //<identifier>
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER) { // error
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
				}

				//judge <variable-declaration> or <function-definition>
				next = nextToken();
				if (!next.has_value()) {    // error
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
				}
				if (next.value().GetType() == TokenType::LEFT_PARENTHESIS) {    //确认是 <function-definition>
					isInFirst = false;
					unreadToken();
					unreadToken();
					unreadToken();
					err = analyseFunctionDeclaration();
					if (err.has_value())
						return err;
				}
				else {    //确认是 <variable-declaration>
					unreadToken();
					unreadToken();
					unreadToken();
					err = analyseVariableDeclaration(TableType::START_TYPE);
					if (err.has_value())
						return err;
				}
			}
			else {// error
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
			}
		}

		while (true) {    // <function-definition>
			next = nextToken();
			if (!next.has_value())    //文件为空，直接返回
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
	std::optional<CompilationError> Analyser::analyseVariableDeclaration(TableType type) {
		bool isConst = false;    //记录是否存在const

		// const
		// TODO const修饰的变量必须被显式初始化
		auto next = nextToken(); // [<const-qualifier>]
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		if (next.value().GetType() == TokenType::CONST) {
			next = nextToken();
			isConst = true;
		}

		// <type-specifier>
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedTypeSpecifier);
		if (next.value().GetType() == TokenType::VOID) {
			// void 不能参与变量的声明 (void 和 const void 都不行)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVoidDeclaration);
		}

		if (next.value().GetType() != TokenType::INT) { //如果不考虑double，这里没有int就是error
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedTypeSpecifier);
		}

		// <init-declarator-list>
		auto err = analyseInitDeclaratorList(isConst, type);
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
	std::optional<CompilationError> Analyser::analyseInitDeclaratorList(const bool isConst, TableType type) {
		// <init-declarator>
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);

		//是否重复声明，分两种情况讨论
		if (type == TableType::START_TYPE && isDeclared(next.value().GetValueString()))
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
		if (type == TableType::FUN_N_TYPE && isLocalDeclared(next.value().GetValueString()))
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

		auto tmpIdentifier = next;    //此时不知道后面有没有等号，还不知道要把标识符添加到哪里

		next = nextToken(); //[<initializer>]
		if (!next.has_value() || next.value().GetType() != TokenType::EQUAL_SIGN) {
			unreadToken();
			if (isConst) {//为const结果没有等号初始化，error
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);
			}
			else {//不是const，也没有等号初始化
				if (type == TableType::START_TYPE) {
					addUninitializedVariable(tmpIdentifier.value());

					_output._start.emplace_back(Operation::snew, 1, 0);
				}
				else {
					addLocalUninitializedVariable(tmpIdentifier.value());

					_output._funN[fun_num - 1].emplace_back(Operation::snew, 1, 0);
				}
			}
		}
		else {    //有等号，根据是否是const加入已经初始化的列表里面
			if (type == TableType::START_TYPE) {
				isConst ? addConstant(tmpIdentifier.value()) : addVariable(tmpIdentifier.value());
				//由于后面求出值之后自然会入栈，此处无需分配存储空间
			}
			else {
				isConst ? addLocalConstant(tmpIdentifier.value()) : addLocalVariable(tmpIdentifier.value());
			}

			//经过这一步就可以将上面的空间中填入具体的值
			auto err = analyseExpression(type);
			if (err.has_value())
				return err;
		}

		// {',' <init-declarator> }
		while (true) {
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

			tmpIdentifier = next;    //此时不知道后面有没有等号，还不知道要把标识符添加到哪里

			next = nextToken(); //[<initializer>]
			if (!next.has_value() || next.value().GetType() != TokenType::EQUAL_SIGN) {
				unreadToken();
				if (isConst) {//为const结果没有等号初始化，error
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);
				}
				else {//不是const，也没有等号初始化，类似 int a; 在栈上为其分配内存空间
					if (type == TableType::START_TYPE) {
						addUninitializedVariable(tmpIdentifier.value());

						_output._start.emplace_back(Operation::snew, 1, 0);
					}
					else {
						addLocalUninitializedVariable(tmpIdentifier.value());
						_output._funN[fun_num - 1].emplace_back(Operation::snew, 1, 0);
					}
				}
			}
			else {    //有等号，根据是否是const加入已经初始化的列表里面,并在栈上分配内存空间
				if (type == TableType::START_TYPE) {
					isConst ? addConstant(tmpIdentifier.value()) : addVariable(tmpIdentifier.value());
					_output._start.emplace_back(Operation::snew, 1, 0);
				}
				else {
					isConst ? addLocalConstant(tmpIdentifier.value()) : addLocalVariable(tmpIdentifier.value());
					_output._funN[fun_num - 1].emplace_back(Operation::snew, 1, 0);
				}

				//经过这一步就可以将上面的空间中填入具体的值
				auto err = analyseExpression(type);
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
		if (!next.has_value() ||
			(next.value().GetType() != TokenType::VOID && next.value().GetType() != TokenType::INT))
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedTypeSpecifier);

		auto type = next.value().GetType();    //记录下是void还是int

		// <identifier>
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
		if (isDeclared(next.value().GetValueString()))
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

		if (type == TokenType::VOID) {
			addVoidFunctions(next.value());
		}

		if (type == TokenType::INT) {
			addIntFunctions(next.value());
		}

		auto findIt = false;
		auto pos = 0;
		auto source = _output._constants;
		
		for (auto iter = source.begin(); iter != source.end(); ++iter) {
			if (std::get<0>(iter->GetValue()) == next.value().GetValueString()) {
				pos = iter - source.begin();
				findIt = true;
				break;
			}
		}
		if (!findIt) {
			// 添加到常量表
			auto tmp_constants = Constants(Type::STRING_TYPE, next.value().GetValueString());
			_output._constants.emplace_back(tmp_constants);
			pos = getPos(_output._constants, tmp_constants);
		}

		//新增一个函数体
		fun_num += 1;
		_output._funN.resize(fun_num);

		//确定main函数的位置
		if (next.value().GetValueString() == "main") {
			if (_main_index == -1)
				_main_index = pos;
			else {    //error 多个main函数
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
			}
		}

		// 每次函数的开始都要清空局部变量
		localClear();

		int32_t paramSize = 0;
		// <parameter-clause> 参数列表
		auto err = analyseParameterClause(paramSize);
		if (err.has_value())
			return err;

		// 添加到函数表(level是固定的)
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
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteBrackets);

		//[<parameter-declaration-list>]
		next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteBrackets);
		if (next.value().GetType() != TokenType::RIGHT_PARENTHESIS) {
			unreadToken();
			auto err = analyseParameterDeclarationList(paramSize);
			if (err.has_value())
				return err;
			// )
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_PARENTHESIS)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteBrackets);
		}

		//else )
		return {};
	}

	// <parameter-declaration-list> :: =<parameter-declaration>{ ',' <parameter-declaration> }
	std::optional<CompilationError> Analyser::analyseParameterDeclarationList(int32_t& paramSize) {
		//<parameter-declaration>
		auto err = analyseParameterDeclaration();
		if (err.has_value())
			return err;
		paramSize++;    //这里只可能是int，故只加一，没有考虑 double

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
		auto isConst = false;
		//const
		auto next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		if (next.value().GetType() == TokenType::CONST) {
			next = nextToken();
			isConst = true;
		}

		// <type-specifier><identifier>
		if (!next.has_value() ||
			(next.value().GetType() != TokenType::VOID && next.value().GetType() != TokenType::INT))
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedTypeSpecifier);

		if (next.value().GetType() == TokenType::VOID) { //error
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVoidParameterDeclaration);
		}
		else if (next.value().GetType() == TokenType::INT) {
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
			}
			//TODO 需要加入到相应的函数表（fun_n）中,参数的值从栈中获取
			// 参数列表中的参数，可以视为函数内部的参数
			isConst ? addLocalConstant(next.value()) : addLocalVariable(next.value());
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

			unreadToken();
			auto err = analyseVariableDeclaration(TableType::FUN_N_TYPE);
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

		//没有返回语句
		if (_output._functions[fun_num - 1].GetHasDetectedRetOrNot() == false) {
			if (isIntFun(fun_num - 1))
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedReturnExpression);
			_output._funN[fun_num - 1].emplace_back(Operation::ret, 0, 0);
		}

		return {};
	}

	//<statement-seq> ::= {<statement>}
	std::optional<CompilationError> Analyser::analyseStatementSeq() {
		while (true) {
			auto next = nextToken();
			if (!next.has_value())
				return {};
			if (next.value().GetType() == TokenType::RIGHT_BRACE) { // 一次也没有的情况
				unreadToken();
				return {};
			}
			unreadToken();
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
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedStatement);
		std::optional<CompilationError> err;
		std::string s;

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
			s = next.value().GetValueString();
			next = nextToken();
			if (!next.has_value())
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
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
				err = analyseFunctionCall(TableType::FUN_N_TYPE, true);
				if (err.has_value())
					return err;

				//这里的function-call只是陈述语句，返回值是用不到的（感觉可以被优化掉）
				//如果调用者不需要返回值，执行 pop 系列指令清除调用者栈帧得到的返回值
				if(isIntFun(s)) {
					_output._funN[fun_num - 1].emplace_back(Operation::pop, 0, 0);
				}

				// ;
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
			}
		case TokenType::SEMICOLON:
			break;
		default:
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedStatement);
		}
		return {};
	}

	//<condition> ::= <expression>[<relational-operator><expression>]
	//<relational-operator> ::= '<' | '<=' | '>' | '>=' | '!=' | '=='
	std::optional<CompilationError> Analyser::analyseCondition(int32_t& label1) {
		//TODO expression 需要在栈上压入某个值
		auto err = analyseExpression(TableType::FUN_N_TYPE);
		if (err.has_value()) {
			return err;
		}

		// [<relational-operator><expression>]
		auto next = nextToken();
		if (!next.has_value()) {    //说明没有可选部分
			_output._funN[fun_num - 1].emplace_back(Operation::ipush, 0, 0);    //与0做对比
			_output._funN[fun_num - 1].emplace_back(Operation::icmp, 0, 0);
			_output._funN[fun_num - 1].emplace_back(Operation::je, 0, 0);        //此处的位置是 label1，需要回填
			label1 = _output._funN[fun_num - 1].size() - 1;                        //记下label1的地址

			return {};
		}

		auto type = next.value().GetType();
		if (type != LESS_SIGN && type != LESS_EQUAL_SIGN && type != ABOVE_SIGN && type != ABOVE_EQUAL_SIGN
			&& type != NOT_EQUAL_SIGN && type != EQUAL_EQUAL_SIGN) {//说明没有可选部分

			_output._funN[fun_num - 1].emplace_back(Operation::ipush, 0, 0);    //与0做对比
			_output._funN[fun_num - 1].emplace_back(Operation::icmp, 0, 0);
			_output._funN[fun_num - 1].emplace_back(Operation::je, 0, 0);        //此处的位置是 label1，需要回填
			label1 = _output._funN[fun_num - 1].size() - 1;                        //记下label1的地址

			unreadToken();
			return {};
		}

		err = analyseExpression(TableType::FUN_N_TYPE);
		if (err.has_value())
			return err;

		switch (type) {
		case TokenType::LESS_SIGN:
			_output._funN[fun_num - 1].emplace_back(Operation::icmp, 0, 0);
			_output._funN[fun_num - 1].emplace_back(Operation::jge, 0, 0);    //先填入 0
			// 记录下此时的下标 1，等到能推断出来时进行回填
			label1 = _output._funN[fun_num - 1].size() - 1;

			break;
		case TokenType::LESS_EQUAL_SIGN:
			_output._funN[fun_num - 1].emplace_back(Operation::icmp, 0, 0);
			_output._funN[fun_num - 1].emplace_back(Operation::jg, 0, 0);    //先填入 0
			label1 = _output._funN[fun_num - 1].size() - 1;

			break;
		case TokenType::ABOVE_SIGN:
			_output._funN[fun_num - 1].emplace_back(Operation::icmp, 0, 0);
			_output._funN[fun_num - 1].emplace_back(Operation::jle, 0, 0);    //先填入 0
			label1 = _output._funN[fun_num - 1].size() - 1;

			break;
		case TokenType::ABOVE_EQUAL_SIGN:
			_output._funN[fun_num - 1].emplace_back(Operation::icmp, 0, 0);
			_output._funN[fun_num - 1].emplace_back(Operation::jl, 0, 0);    //先填入 0
			label1 = _output._funN[fun_num - 1].size() - 1;

			break;
		case TokenType::NOT_EQUAL_SIGN:
			_output._funN[fun_num - 1].emplace_back(Operation::icmp, 0, 0);
			_output._funN[fun_num - 1].emplace_back(Operation::je, 0, 0);    //先填入 0
			label1 = _output._funN[fun_num - 1].size() - 1;

			break;
		case TokenType::EQUAL_EQUAL_SIGN:
			_output._funN[fun_num - 1].emplace_back(Operation::icmp, 0, 0);
			_output._funN[fun_num - 1].emplace_back(Operation::jne, 0, 0);    //先填入 0
			label1 = _output._funN[fun_num - 1].size() - 1;

			break;
		default:
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrUnexpectedError);
		}
		return {};
	}

	// <condition-statement> :: =
	// 'if' '(' <condition> ')' <statement> ['else' <statement>]
	std::optional<CompilationError> Analyser::analyseConditionStatement() {
		//回填地址
		int32_t label1, label2;

		
		//if语句和while语句中的return可能会影响到整个函数对于是否有return的判断，这里的return不能作数
		const auto isRet = _output._functions[fun_num - 1].GetHasDetectedRetOrNot();
		
		// if
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::IF)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		// (
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_PARENTHESIS)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		// <condition>
		auto err = analyseCondition(label1);
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
		if (!next.has_value() || next.value().GetType() != TokenType::ELSE) {    //没有else就不需要有label2
			//没有else的话，label1 就在这，回填，指向下一条指令（下一条指令现在还没有）
			const int32_t next_pos1 = _output._funN[fun_num - 1].size();
			_output._funN[fun_num - 1][label1].SetX(next_pos1);
			unreadToken();
			return {};
		}

		_output._funN[fun_num - 1].emplace_back(Operation::jmp, 0, 0);    //无条件跳转，先填入 0
		label2 = _output._funN[fun_num - 1].size() - 1;    //记下label2的地址

		//有else的话，label1 就在这（位于第二个jmp之后），回填，指向下一条指令（下一条指令现在还没有）
		const int32_t next_pos1 = _output._funN[fun_num - 1].size();
		_output._funN[fun_num - 1][label1].SetX(next_pos1);

		// <statement>
		err = analyseStatement();
		if (err.has_value())
			return err;

		//label2 就在这，回填，指向下一条指令（下一条指令现在还没有）
		const int32_t next_pos2 = _output._funN[fun_num - 1].size();
		_output._funN[fun_num - 1][label2].SetX(next_pos2);

		//最后恢复关于是否有return的判断
		_output._functions[fun_num - 1].SetFindRetExpression(isRet);

		return {};
	}

	// <loop-statement> ::=
	// 'while' '(' <condition> ')' <statement>
	std::optional<CompilationError> Analyser::analyseLoopStatement() {
		// 回填地址
		int32_t label1;

		//if语句和while语句中的return可能会影响到整个函数对于是否有return的判断，这里的return不能作数
		const auto isRet = _output._functions[fun_num - 1].GetHasDetectedRetOrNot();
		
		// while
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::WHILE)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		// (
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_PARENTHESIS)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		
		//先获得进入while的第一条语句所在位置，while的最后一句要无条件跳转过来
		const int32_t next_pos2 = _output._funN[fun_num - 1].size();

		// <condition>
		auto err = analyseCondition(label1);    //获得label1的值
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

		//最后一条指令，无条件跳转回第一句话
		_output._funN[fun_num - 1].emplace_back(Operation::jmp, next_pos2, 0);

		//label1 就在最后一条指令之后，即while的外面。回填，指向下一条指令（下一条指令现在还没有）
		const int32_t next_pos1 = _output._funN[fun_num - 1].size();
		_output._funN[fun_num - 1][label1].SetX(next_pos1);

		//最后恢复关于是否有return的判断
		_output._functions[fun_num - 1].SetFindRetExpression(isRet);

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
		bool isInt = isIntFun(fun_num - 1);

		_output._functions[fun_num - 1].SetFindRetExpression(true);	//表明有返回函数

		// return
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RETURN) {	//不该运行到这一句
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrUnexpectedError);
		}

		// [<expression>] ';'
		next = nextToken();
		if (!next.has_value()) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
		}

		if (next.value().GetType() != TokenType::SEMICOLON) { //说明存在可选项，返回值应是int
			if (!isInt)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrVoidFunReturnByInt);

			unreadToken();
			auto err = analyseExpression(TableType::FUN_N_TYPE);
			if (err.has_value())
				return err;
			_output._funN[fun_num - 1].emplace_back(Operation::iret, 0, 0);

			// ;
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
		}
		else {    //返回值应该是void
			if (isInt)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIntFunReturnByVoid);
			_output._funN[fun_num - 1].emplace_back(Operation::ret, 0, 0);
		}

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
		if (next.value().GetType() != TokenType::RIGHT_PARENTHESIS) {//print为空
			unreadToken();
			auto err = analysePrintableList();
			if (err.has_value())
				return err;
			// )
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_PARENTHESIS)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteBrackets);
		}

		// ;
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

		// print最后会输出一个换行
		_output._funN[fun_num - 1].emplace_back(Operation::printl, 0, 0);

		return {};
	}

	// <printable-list>  ::= <printable> {',' <printable> }
	// <printable> ::= <expression> | <string - literal> | <char - literal>
	std::optional<CompilationError> Analyser::analysePrintableList() {
		auto source = _output._constants;
		auto pos = 0;
		bool findIt = false;
		
		//判断
		auto next = nextToken();
		if(!next.has_value()) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		}
		if(next.value().GetType() == TokenType::CHAR_SIGN) {
			auto num_tmp = std::stoll(next.value().GetValueString());
			
			_output._funN[fun_num - 1].emplace_back(Operation::bipush, num_tmp, 0);
			_output._funN[fun_num - 1].emplace_back(Operation::cprint, 0, 0);
		}
		else if (next.value().GetType() == TokenType::STRING_SIGN) {
			findIt = false;
			for (auto iter = source.begin(); iter != source.end(); ++iter) {
				if (std::get<0>(iter->GetValue()) == next.value().GetValueString()) {
					pos = iter - source.begin();
					findIt = true;
					break;
				}
			}
			if(!findIt) {
			 	// 添加到常量表
			 	auto tmp_constants = Constants(Type::STRING_TYPE, next.value().GetValueString());
			 	_output._constants.emplace_back(tmp_constants);
			 	pos = getPos(_output._constants, tmp_constants);
			}
			_output._funN[fun_num - 1].emplace_back(Operation::loadc, pos, 0);
			 _output._funN[fun_num - 1].emplace_back(Operation::sprint, 0, 0);
		}
		else {
			//<printable>
			unreadToken();
			auto err = analyseExpression(TableType::FUN_N_TYPE);
			if (err.has_value())
				return err;

			_output._funN[fun_num - 1].emplace_back(Operation::iprint, 0, 0);
		}
		

		// {',' <printable> }
		while (true) {
			auto next = nextToken();
			if (!next.has_value())
				return {};
			if (next.value().GetType() != TokenType::COMMA) {
				unreadToken();
				return {};
			}

			//一个print有多个<printable>时，<printable>之间输出一个空格(bipush 32 + cprint)
			_output._funN[fun_num - 1].emplace_back(Operation::bipush, 32, 0);
			_output._funN[fun_num - 1].emplace_back(Operation::cprint, 0, 0);
			
			
			next = nextToken();
			if (!next.has_value()) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
			}
			if (next.value().GetType() == TokenType::CHAR_SIGN) {
				auto num_tmp = std::stoll(next.value().GetValueString());

				_output._funN[fun_num - 1].emplace_back(Operation::bipush, num_tmp, 0);
				_output._funN[fun_num - 1].emplace_back(Operation::cprint, 0, 0);
			}
			else if (next.value().GetType() == TokenType::STRING_SIGN) {
				findIt = false;
				for (auto iter = source.begin(); iter != source.end(); ++iter) {
					if (std::get<0>(iter->GetValue()) == next.value().GetValueString()) {
						pos = iter - source.begin();
						findIt = true;
						break;
					}
				}
				if (!findIt) {
					// 添加到常量表
					auto tmp_constants = Constants(Type::STRING_TYPE, next.value().GetValueString());
					_output._constants.emplace_back(tmp_constants);
					pos = getPos(_output._constants, tmp_constants);
				}
				_output._funN[fun_num - 1].emplace_back(Operation::loadc, pos, 0);
				_output._funN[fun_num - 1].emplace_back(Operation::sprint, 0, 0);
			}
			else {
				//<printable>
				unreadToken();
				auto err = analyseExpression(TableType::FUN_N_TYPE);
				if (err.has_value())
					return err;

				_output._funN[fun_num - 1].emplace_back(Operation::iprint, 0, 0);
			}
		}

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

		//是否声明过
		auto s = next.value().GetValueString();
		if (isLocalDeclared(s)) {
			//是否是const
			if (isLocalConstant(s)) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);
			}
			else {    //不是const，此时需要获取他所在的index
				auto index_tmp = getLocalIndex(s);

				//局部能找到的，栈帧都是 0
				_output._funN[fun_num - 1].emplace_back(Operation::loada, 0, index_tmp);
				_output._funN[fun_num - 1].emplace_back(Operation::iscan, 0, 0);
				_output._funN[fun_num - 1].emplace_back(Operation::istore, 0, 0);

				transLocalUninitVarToLocalInitVar(next.value());
			}
		}
		else if (isIntFun(s) || isVoidFun(s)) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToFunction);
		}
		else if (isDeclared(s)) {    //局部没找到，找全局声明
			if (isConstant(s)) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);
			}
			else {
				auto index_tmp = getIndex(s);

				//全局能找到的，栈帧是 1
				_output._funN[fun_num - 1].emplace_back(Operation::loada, 1, index_tmp);
				_output._funN[fun_num - 1].emplace_back(Operation::iscan, 0, 0);
				_output._funN[fun_num - 1].emplace_back(Operation::istore, 0, 0);

				transUninitVarToInitVar(next.value());
			}
		}
		else { //赋值给没声明过的变量
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidAssignment);
		}

		// )
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_PARENTHESIS)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteBrackets);

		// ;
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
		return {};
	}

	// <assignment-expression> ::= <identifier><assignment-operator><expression>
	// <assignment-operator> ::= '='
	std::optional<CompilationError> Analyser::analyseAssignmentExpression() {
		// <identifier>
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);

		auto s = next.value().GetValueString();

		//是否声明过
		if (isLocalDeclared(s)) {    //局部声明
			//是否是const
			if (isLocalConstant(s)) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);
			}
			else {    //不是const，此时需要获取他所在的index
				auto index_tmp = getLocalIndex(s);

				//局部能找到的，栈帧都是 0
				_output._funN[fun_num - 1].emplace_back(Operation::loada, 0, index_tmp);

				transLocalUninitVarToLocalInitVar(next.value());
			}

		}
		else if (isDeclared(s)) {    //全局声明
			if (isConstant(s)) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);
			}
			else {
				auto index_tmp = getIndex(s);

				//全局能找到的，栈帧是 1
				_output._funN[fun_num - 1].emplace_back(Operation::loada, 1, index_tmp);

				transUninitVarToInitVar(next.value());
			}

		}
		else { //赋值给没声明过的变量
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidAssignment);
		}

		// <assignment-operator>
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::EQUAL_SIGN)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		// <expression>
		auto err = analyseExpression(TableType::FUN_N_TYPE);
		if (err.has_value())
			return err;

		//把expression得到的值存入加载的地址当中
		_output._funN[fun_num - 1].emplace_back(Operation::istore, 0, 0);

		return {};
	}

	// <expression> :: = <additive-expression>
	// 参数是为了判断将汇编代码加入到哪个符号表里
	// type表示符号表，index只在type=fun_n_type时有用，用来标识哪个函数
	// TODO 这个type需要传递下去
	std::optional<CompilationError> Analyser::analyseExpression(TableType type) {// TODO 此处可能会调用常量表中的内容
		auto err = analyseAdditiveExpression(type);
		if (err.has_value())
			return err;
		return {};
	}

	// <additive-expression> :: =
	// 	<multiplicative-expression>{ <additive-operator><multiplicative-expression> }
	// 	<additive-operator>       ::= '+' | '-'
	std::optional<CompilationError> Analyser::analyseAdditiveExpression(TableType type) {
		bool isPlus = false;    //判断是加号还是减号

		auto err = analyseMultiplicativeExpression(type);
		if (err.has_value())
			return err;

		// {<additive-operator><multiplicative-expression>}
		while (true) {
			auto next = nextToken();
			if (!next.has_value())
				return {};
			auto tk_type = next.value().GetType();
			if (tk_type == TokenType::PLUS_SIGN) {
				isPlus = true;
			}
			else if (tk_type == TokenType::MINUS_SIGN) {
				isPlus = false;
			}
			else {
				unreadToken();
				return {};
			}
			err = analyseMultiplicativeExpression(type);
			if (err.has_value())
				return err;

			//根据传进来的type值判断在那里添加汇编代码
			if (type == TableType::START_TYPE) {
				isPlus ? _output._start.emplace_back(Operation::iadd, 0, 0) :
					_output._start.emplace_back(Operation::isub, 0, 0);
			}
			else {
				isPlus ? _output._funN[fun_num - 1].emplace_back(Operation::iadd, 0, 0) :
					_output._funN[fun_num - 1].emplace_back(Operation::isub, 0, 0);
			}
		}
		return {};
	}

	// <multiplicative-expression> ::=
	// <unary-expression>{<multiplicative-operator><unary-expression>}
	std::optional<CompilationError> Analyser::analyseMultiplicativeExpression(TableType type) {
		bool isMul = false;    //判断是乘号还是除号

		auto err = analyseUnaryExpression(type);
		if (err.has_value())
			return err;

		// {<multiplicative-operator><unary-expression>}
		while (true) {
			auto next = nextToken();
			if (!next.has_value())
				return {};
			auto tk_type = next.value().GetType();

			//存在可选式
			if (tk_type == TokenType::MULTIPLICATION_SIGN) {
				isMul = true;
			}
			else if (tk_type == TokenType::DIVISION_SIGN) {
				isMul = false;
			}
			else {
				unreadToken();
				return {};
			}

			err = analyseUnaryExpression(type);
			if (err.has_value())
				return err;

			//根据传进来的type值判断在那里添加汇编代码
			if (type == TableType::START_TYPE) {
				isMul ? _output._start.emplace_back(Operation::imul, 0, 0) :
					_output._start.emplace_back(Operation::idiv, 0, 0);
			}
			else {
				isMul ? _output._funN[fun_num - 1].emplace_back(Operation::imul, 0, 0) :
					_output._funN[fun_num - 1].emplace_back(Operation::idiv, 0, 0);
			}

		}
		return {};
	}

	//<unary-expression> ::= [<unary-operator>]<primary-expression>
	//<unary-operator>   ::= '+' | '-'
	std::optional<CompilationError> Analyser::analyseUnaryExpression(TableType type) {
		bool isNeg = false;    //判断是不是负数

		//[<unary-operator>]
		auto next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		auto tk_type = next.value().GetType();
		if (tk_type == TokenType::PLUS_SIGN) {
			isNeg = false;
		}
		else if (tk_type == TokenType::MINUS_SIGN) {
			isNeg = true;
		}
		else {
			isNeg = false;
			unreadToken();
		}
		auto err = analysePrimaryExpression(type);
		if (err.has_value())
			return err;

		if (isNeg)    //是负数的话需要取负
			type == TableType::START_TYPE ? _output._start.emplace_back(Operation::ineg, 0, 0) :
			_output._funN[fun_num - 1].emplace_back(Operation::ineg, 0, 0);

		return {};
	}

	//<primary-expression> ::=
	// '(' <expression> ')'
	// 	| <identifier>
	// 	| <integer-literal>
	// 	| <function-call>
	std::optional<CompilationError> Analyser::analysePrimaryExpression(TableType type) {
		int32_t num_tmp;

		auto next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		std::optional<cc0::CompilationError> err;

		auto tk_type = next.value().GetType();
		auto s = next.value().GetValueString();


		switch (tk_type) {
		case TokenType::LEFT_PARENTHESIS:
			err = analyseExpression(type);
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
			if (!next.has_value() || next.value().GetType() != TokenType::LEFT_PARENTHESIS) {
				unreadToken();
				if (type == TableType::START_TYPE) {    //这是全局变量
					//是否声明过
					//是否有值
					if (!isDeclared(s))
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
					if (!isInitializedVariable(s) && !isConstant(s))
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);


					auto index_tmp = getIndex(s);
					_output._start.emplace_back(Operation::loada, 0, index_tmp);
					_output._start.emplace_back(Operation::iload, 0, 0);
				}
				else {  //局部变量  
					if (!isLocalDeclared(s)) { // 局部找不到就去全局找
						if (!isDeclared(s))
							return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
						//全局找到了
						if (!isInitializedVariable(s) && !isConstant(s))
							return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);

						auto index_tmp = getIndex(s);
						_output._funN[fun_num - 1].emplace_back(Operation::loada, 1, index_tmp);
						_output._funN[fun_num - 1].emplace_back(Operation::iload, 0, 0);
					}
					else {//局部找到了
						if (!isLocalInitializedVariable(s) && !isLocalConstant(s))
							return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);

						auto index_tmp = getLocalIndex(s);
						_output._funN[fun_num - 1].emplace_back(Operation::loada, 0, index_tmp);
						_output._funN[fun_num - 1].emplace_back(Operation::iload, 0, 0);
					}
				}

				return {};
			}

			//是 <function-call>
			unreadToken();
			unreadToken();
			err = analyseFunctionCall(type,false);
			if (err.has_value())
				return err;
			break;

			//<integer-literal>
		case TokenType::DECIMAL_INTEGER:
		case TokenType::HEXADECIMAL_INTEGER:
			num_tmp = std::stoll(next.value().GetValueString());
			if (type == TableType::START_TYPE) {
				_output._start.emplace_back(Operation::ipush, num_tmp, 0);
			}
			else {
				_output._funN[fun_num - 1].emplace_back(Operation::ipush, num_tmp, 0);
			}
			break;
		default:
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrUnexpectedError);
		}
		return {};
	}

	// <function-call> :: =
	// 	<identifier> '('[<expression-list>] ')'
	std::optional<CompilationError> Analyser::analyseFunctionCall(TableType type, bool can_be_void_or_not) {
		//<identifier>
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
		auto s = next.value().GetValueString();

		//TODO
		if (!isVoidFun(s) && !isIntFun(s)) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrFunctionNotExist);
		}

		if(isVoidFun(s) && can_be_void_or_not == false) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrFunctionCanNotBeVoid);
		}

		//因为参数只能是int型的，所以函数表中有他的参数个数信息
		auto _expect_param_num = getParamsNum(s);
		auto _fun_index = getFunctionIndexInConstants(s);

		// (
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_PARENTHESIS)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		// [<expression-list>]
		auto _actual_param_num = 0;
		next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		unreadToken();

		// 存在可选项
		if (next.value().GetType() != TokenType::RIGHT_PARENTHESIS) {
			auto err = analyseExpressionList(type, _actual_param_num);
			if (err.has_value())
				return err;
			//参数不匹配
			if (_actual_param_num != _expect_param_num) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrParameterMismatch);
			}
		}
		else {
			//参数不匹配
			if (_actual_param_num != _expect_param_num) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrParameterMismatch);
			}
		}

		// )
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_PARENTHESIS)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteBrackets);

		if (type == TableType::FUN_N_TYPE) {
			_output._funN[fun_num - 1].emplace_back(Operation::call, _fun_index, 0);
		}
		else {	//在全局变量里不能调用函数
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrCallFunctionInGlobalArea);
		}

		return {};
	}

	// <expression-list> ::= <expression>{',' < expression > }
	std::optional<CompilationError> Analyser::analyseExpressionList(TableType type, int32_t& paramSize) {
		auto err = analyseExpression(type);
		if (err.has_value())
			return err;
		paramSize++;

		// {',' < expression > }
		while (true) {
			auto next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::COMMA) {
				unreadToken();
				return {};
			}
			err = analyseExpression(type);
			if (err.has_value())
				return err;
			paramSize++;

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

	void Analyser::addVariable(const Token& tk) {	//已经用等号初始化的
		_add(tk, _vars);
	}

	void Analyser::addConstant(const Token& tk) {
		_add(tk, _consts);
	}

	void Analyser::addUninitializedVariable(const Token& tk) {	//未用等号初始化的
		_add(tk, _uninitialized_vars);
	}

	void Analyser::transUninitVarToInitVar(const Token& tk) {	//全局变量未初始化的后来初始化了
		if (tk.GetType() != TokenType::IDENTIFIER)
			DieAndPrint("only identifier can be added to the table.");

		const auto tmp = _uninitialized_vars.find(tk.GetValueString());

		if (tmp == _uninitialized_vars.end())	//没有找到说明已经被初始化了，就不用管
			return;

		_vars.insert(std::make_pair(tk.GetValueString(), tmp->second));	//先增后删，位置不能改变
		_uninitialized_vars.erase(tk.GetValueString());
	}

	void Analyser::transLocalUninitVarToLocalInitVar(const Token& tk) {	//局部变量未初始化的后来初始化了
		if (tk.GetType() != TokenType::IDENTIFIER)
			DieAndPrint("only identifier can be added to the table.");

		const auto tmp = _local_uninitialized_vars.find(tk.GetValueString());

		if (tmp == _local_uninitialized_vars.end())
			return;

		_local_vars.insert(std::make_pair(tk.GetValueString(), tmp->second));	//先增后删，位置不能改变
		_local_uninitialized_vars.erase(tk.GetValueString());
	}

	void Analyser::addVoidFunctions(const Token& tk) {
		_add(tk, _void_funs);
	}

	void Analyser::addIntFunctions(const Token& tk) {
		_add(tk, _int_funs);
	}

	void Analyser::_add_local(const Token& tk, std::map<std::string, int32_t>& mp) {
		if (tk.GetType() != TokenType::IDENTIFIER)
			DieAndPrint("only identifier can be added to the table.");
		mp[tk.GetValueString()] = _nextLocalTokenIndex;
		_nextLocalTokenIndex++;
	}

	void Analyser::addLocalVariable(const Token& tk) {	//已经初始化的
		_add_local(tk, _local_vars);
	}

	void Analyser::addLocalConstant(const Token& tk) {
		_add_local(tk, _local_consts);
	}

	void Analyser::addLocalUninitializedVariable(const Token& tk) {
		_add_local(tk, _local_uninitialized_vars);
	}

	int32_t Analyser::getIndex(const std::string& s) {  //返回包括全局变量和函数的位置
		if (_uninitialized_vars.find(s) != _uninitialized_vars.end())
			return _uninitialized_vars[s];
		else if (_vars.find(s) != _vars.end())
			return _vars[s];
		else if (_consts.find(s) != _consts.end())
			return _consts[s];
		// 不应该会查询函数的位置，函数名是不参与计算的
//        else if(_int_funs.find(s) != _int_funs.end())
//            return _int_funs[s];
//        else if(_void_funs.find(s) != _void_funs.end())
//            return _void_funs[s];
		else
			DieAndPrint("can not find global variable");    //不应该执行到这一句
	}

	//TODO 应该能根据返回值判断出是在函数内部还是外部
	int32_t Analyser::getLocalIndex(const std::string& s) {
		if (_local_uninitialized_vars.find(s) != _local_uninitialized_vars.end())
			return _local_uninitialized_vars[s];
		else if (_local_vars.find(s) != _local_vars.end())
			return _local_vars[s];
		else if (_local_consts.find(s) != _local_consts.end())
			return _local_consts[s];
		else
			DieAndPrint("can not find local variable");    //不应该执行到这一句
	}

	bool Analyser::isDeclared(const std::string& s) { //变量和函数重名也算重命名
		return isConstant(s) || isUninitializedVariable(s) || isInitializedVariable(s) || isVoidFun(s) || isIntFun(s);
	}

	bool Analyser::isUninitializedVariable(const std::string& s) {
		return _uninitialized_vars.find(s) != _uninitialized_vars.end();
	}

	bool Analyser::isInitializedVariable(const std::string& s) {
		return _vars.find(s) != _vars.end();
	}

	bool Analyser::isConstant(const std::string& s) {
		return _consts.find(s) != _consts.end();
	}

	bool Analyser::isVoidFun(const std::string& s) {
		return _void_funs.find(s) != _void_funs.end();
	}

	bool Analyser::isIntFun(const std::string& s) {
		return _int_funs.find(s) != _int_funs.end();
	}

	bool Analyser::isIntFun(const int32_t index_in_constants) {
		auto s = std::get<0>(_output._constants.at(index_in_constants).GetValue());
		return _int_funs.find(s) != _int_funs.end();
	}

	bool Analyser::isVoidFun(const int32_t index_in_constants) {
		auto s = std::get<0>(_output._constants.at(index_in_constants).GetValue());
		return _void_funs.find(s) != _void_funs.end();
	}

	bool Analyser::isLocalDeclared(const std::string& s) { //变量和函数重名也算重命名，但是没有嵌套函数
		return isLocalConstant(s) || isLocalInitializedVariable(s) || isLocalUninitializedVariable(s);
	}

	bool Analyser::isLocalUninitializedVariable(const std::string& s) {
		return _local_uninitialized_vars.find(s) != _local_uninitialized_vars.end();
	}

	bool Analyser::isLocalInitializedVariable(const std::string& s) {
		return _local_vars.find(s) != _local_vars.end();
	}

	bool Analyser::isLocalConstant(const std::string& s) {
		return _local_consts.find(s) != _local_consts.end();
	}

	int32_t Analyser::getFunctionIndexInConstants(const std::string& s) {
		for (auto i = _output._constants.begin(); i != _output._constants.end(); ++i) {
			auto ss = std::get<0>(i->GetValue());
			if (s == ss)
				return i - _output._constants.begin();
		}
		//unexpected error
		DieAndPrint("can not find this function in constants table");
	}

	int32_t Analyser::getParamsNum(const std::string& s) {
		auto _consts_index = getFunctionIndexInConstants(s);
		for (auto i : _output._functions) {
			if (i.GetNameIndex() == _consts_index)
				return i.GetParamSize();
		}
		//unexpected error
		DieAndPrint("can not find this function in functions table");
	}

	bool cc0::Analyser::isMainFun(int32_t index_in_constants) {
		return std::get<0>(_output._constants.at(index_in_constants).GetValue()) == "main";
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
				if (std::get<0>(iter->GetValue()) == value) {
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

	void Analyser::localClear() {
		_local_consts.clear();
		_local_vars.clear();
		_local_uninitialized_vars.clear();
		_nextLocalTokenIndex = 0;
	}
}