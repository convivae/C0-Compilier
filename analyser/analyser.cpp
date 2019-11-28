#include "analyser.h"

#include <climits>

namespace c0 {
	std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyser::Analyse() {
		auto err = analyseProgram();
		if (err.has_value())
			return std::make_pair(std::vector<Instruction>(), err);
		else
			return std::make_pair(_instructions, std::optional<CompilationError>());
	}

	// <程序> ::= 'begin'<主过程>'end'
	std::optional<CompilationError> Analyser::analyseProgram() {
		// 示例函数，示例如何调用子程序

		// 'begin'
		auto bg = nextToken();
		if (!bg.has_value() || bg.value().GetType() != TokenType::BEGIN)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoBegin);

		// <主过程>
		auto err = analyseMain();
		if (err.has_value())
			return err;

		// 'end'
		auto ed = nextToken();
		if (!ed.has_value() || ed.value().GetType() != TokenType::END)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoEnd);
		return {};
	}

	// <主过程> ::= <常量声明><变量声明><语句序列>
	// 需要补全
	std::optional<CompilationError> Analyser::analyseMain() {
		// 完全可以参照 <程序> 编写

		// <常量声明>
		auto err = analyseConstantDeclaration();
		if (err.has_value())
			return err;

		// <变量声明>
		err = analyseVariableDeclaration();
		if (err.has_value())
			return err;

		// <语句序列>
		err = analyseStatementSequence();
		if (err.has_value())
			return err;
		return {};
	}

	// <常量声明> ::= {<常量声明语句>}
	// <常量声明语句> ::= 'const'<标识符>'='<常表达式>';'
	std::optional<CompilationError> Analyser::analyseConstantDeclaration() {
		// 示例函数，示例如何分析常量声明

		// 常量声明语句可能有 0 或无数个
		while (true) {
			// 预读一个 token，不然不知道是否应该用 <常量声明> 推导
			auto next = nextToken();
			if (!next.has_value())
				return {};
			// 如果是 const 那么说明应该推导 <常量声明> 否则直接返回
			if (next.value().GetType() != TokenType::CONST) {
				unreadToken();
				return {};
			}

			// <常量声明语句>
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
			if (isDeclared(next.value().GetValueString()))
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
			addConstant(next.value());

			// '='
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::EQUAL_SIGN)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantNeedValue);

			// <常表达式>
			int32_t val;
			auto err = analyseConstantExpression(val);
			if (err.has_value())
				return err;

			// ';'
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
			// 生成一次 LIT 指令加载常量
			_instructions.emplace_back(Operation::LIT, val);
		}
		return {};
	}

	// <变量声明> ::= {<变量声明语句>}
	// <变量声明语句> ::= 'var'<标识符>['='<表达式>]';'
	// 需要补全
	std::optional<CompilationError> Analyser::analyseVariableDeclaration() {
		// 变量声明语句可能有一个或者多个
		while (true) {
			// 预读？
			auto next = nextToken();
			if (!next.has_value())
				return {};
			// 如果是 ‘var’ 那么说明应该推导 <变量声明语句> 否则直接返回
			if (next.value().GetType() != TokenType::VAR) {
				unreadToken();
				return {};
			}
			// <标识符>
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
			if (isDeclared(next.value().GetValueString()))
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

			// 变量可能没有初始化，仍然需要一次预读
			auto next_tmp = nextToken();
			if (!next_tmp.has_value())	//类似 var a 没了，缺少分号，报错
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

			// 生成一次 LIT 指令加载变量，先用0来占位，表示未初始化
			_instructions.emplace_back(Operation::LIT, 0);

			if (next_tmp.value().GetType() == TokenType::SEMICOLON) {// var a; 未初始化
				addUninitializedVariable(next.value());
			}
			else {
				// '='
				if (next_tmp.value().GetType() != TokenType::EQUAL_SIGN) {//var a ? 赋值语句不正确
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidAssignment);
				}
				addVariable(next.value());	//有等号，已初始化

				auto id_name = next.value().GetValueString();	//得到该标识符的名字，便于找到在栈中的偏移量

				// '<表达式>'
				auto err = analyseExpression();
				if (err.has_value())
					return err;

				// ';'
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

				// 生成一次 STO 指令加载变量
				auto id_pos = getIndex(id_name);
				_instructions.emplace_back(Operation::STO, id_pos);
			}

		}
		return {};
	}

	// <语句序列> ::= {<语句>}
	// <语句> :: = <赋值语句> | <输出语句> | <空语句>
	// <赋值语句> :: = <标识符>'='<表达式>';'
	// <输出语句> :: = 'print' '(' <表达式> ')' ';'
	// <空语句> :: = ';'
	// 需要补全
	std::optional<CompilationError> Analyser::analyseStatementSequence() {
		while (true) {
			// 预读
			auto next = nextToken();
			if (!next.has_value())
				return {};
			unreadToken();
			if (next.value().GetType() != TokenType::IDENTIFIER &&
				next.value().GetType() != TokenType::PRINT &&
				next.value().GetType() != TokenType::SEMICOLON) {
				return {};
			}
			std::optional<CompilationError> err;
			switch (next.value().GetType()) {
				// 这里需要你针对不同的预读结果来调用不同的子程序
			case TokenType::IDENTIFIER:
				err = analyseAssignmentStatement();
				if (err.has_value())
					return err;
				break;
			case TokenType::PRINT:
				err = analyseOutputStatement();
				if (err.has_value())
					return err;
				break;
				// 注意我们没有针对空语句单独声明一个函数，因此可以直接在这里返回
			default:
				//print
				next = nextToken();
				// ';'
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
				break;
			}
		}
		return {};
	}

	// <常表达式> ::= [<符号>]<无符号整数>
	// 需要补全
	std::optional<CompilationError> Analyser::analyseConstantExpression(int32_t& out) {
		// out 是常表达式的结果
		// 这里你要分析常表达式并且计算结果
		// 注意以下均为常表达式
		// +1 -1 1
		// 同时要注意是否溢出
		auto prefix = 1;
		auto next = nextToken();
		if (next.value().GetType() != TokenType::PLUS_SIGN &&
			next.value().GetType() != TokenType::MINUS_SIGN &&
			next.value().GetType() != TokenType::UNSIGNED_INTEGER) {//const a = ? 常表达式需要赋值
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantNeedValue);
		}
		if (next.value().GetType() == TokenType::MINUS_SIGN) {
			prefix = -1;
		}

		if (next.value().GetType() == TokenType::PLUS_SIGN || next.value().GetType() == TokenType::MINUS_SIGN) {
			next = nextToken();
			if (next.value().GetType() != TokenType::UNSIGNED_INTEGER) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantNeedValue);
			}
		}
		try {
			out = std::stoll(next.value().GetValueString());
			out *= prefix;
		}
		catch (const std::bad_any_cast&) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIntegerOverflow);
		}
		return {};
	}

	// <表达式> ::= <项>{<加法型运算符><项>}
	std::optional<CompilationError> Analyser::analyseExpression() {
		// <项>
		auto err = analyseItem();
		if (err.has_value())
			return err;

		// {<加法型运算符><项>}
		while (true) {
			// 预读
			auto next = nextToken();
			if (!next.has_value())
				return {};
			auto type = next.value().GetType();
			if (type != TokenType::PLUS_SIGN && type != TokenType::MINUS_SIGN) {
				unreadToken();
				return {};
			}

			// <项>
			err = analyseItem();
			if (err.has_value())
				return err;

			// 根据结果生成指令
			if (type == TokenType::PLUS_SIGN)
				_instructions.emplace_back(Operation::ADD, 0);
			else if (type == TokenType::MINUS_SIGN)
				_instructions.emplace_back(Operation::SUB, 0);
		}
		return {};
	}

	// <赋值语句> ::= <标识符>'='<表达式>';'
	// 需要补全
	std::optional<CompilationError> Analyser::analyseAssignmentStatement() {
		// 这里除了语法分析以外还要留意
		// 标识符声明过吗？
		// 标识符是常量吗？
		// 需要生成指令吗？
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER) {//此处不应该有错，unexpected ERR
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
		}
		if (!isDeclared(next.value().GetValueString()))//使用了未声明的变量
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
		if (isConstant(next.value().GetValueString()))//不能给常量赋值
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);

		_instructions.emplace_back(Operation::LIT, 0);

		auto id_name = next.value().GetValueString();	//得到该标识符的名字，便于找到在栈中的偏移量
		auto next_tmp = next;

		// '='
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::EQUAL_SIGN)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		// <表达式>
		auto err = analyseExpression();
		if (err.has_value())
			return err;

		// ';'
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

		//赋值以后该变量就已经初始化了，将其放在普通的变量map中
		if (isUninitializedVariable(next_tmp.value().GetValueString())) {
			_uninitialized_vars.erase(next_tmp.value().GetValueString());
			this->addVariable(next_tmp.value());
		}

		//把栈中的该值重新赋值为栈顶的值
		auto id_pos = getIndex(id_name);
		_instructions.emplace_back(Operation::STO, id_pos);
		return {};
	}

	// <输出语句> ::= 'print' '(' <表达式> ')' ';'
	std::optional<CompilationError> Analyser::analyseOutputStatement() {
		// 如果之前 <语句序列> 的实现正确，这里第一个 next 一定是 TokenType::PRINT
		auto next = nextToken();

		// '('
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);

		// <表达式>
		auto err = analyseExpression();
		if (err.has_value())
			return err;

		// ')'
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);

		// ';'
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

		// 生成相应的指令 WRT
		_instructions.emplace_back(Operation::WRT, 0);
		return {};
	}

	// <项> :: = <因子>{ <乘法型运算符><因子> }
	// 需要补全
	std::optional<CompilationError> Analyser::analyseItem() {
		// 可以参考 <表达式> 实现
		//  <因子>
		auto err = analyseFactor();
		if (err.has_value())
			return err;

		// { <乘法型运算符><因子> }
		while (true) {
			// 预读
			auto next = nextToken();
			if (!next.has_value())
				return {};
			auto type = next.value().GetType();
			if (type != TokenType::MULTIPLICATION_SIGN && type != TokenType::DIVISION_SIGN) {
				unreadToken();
				return {};
			}

			// <因子>
			err = analyseFactor();
			if (err.has_value())
				return err;

			// 根据结果生成指令
			if (type == TokenType::MULTIPLICATION_SIGN)
				_instructions.emplace_back(Operation::MUL, 0);
			else if (type == TokenType::DIVISION_SIGN)
				_instructions.emplace_back(Operation::DIV, 0);
		}
		return {};
	}

	// <因子> ::= [<符号>]( <标识符> | <无符号整数> | '('<表达式>')' )
	// 需要补全
	std::optional<CompilationError> Analyser::analyseFactor() {
		// [<符号>]
		auto next = nextToken();
		auto prefix = 1;
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		if (next.value().GetType() == TokenType::PLUS_SIGN)
			prefix = 1;
		else if (next.value().GetType() == TokenType::MINUS_SIGN) {
			prefix = -1;
			_instructions.emplace_back(Operation::LIT, 0);
		}
		else
			unreadToken();

		// 预读
		next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		std::optional<CompilationError> err;
		auto id_pos = -1;
		switch (next.value().GetType()) {
			// 这里和 <语句序列> 类似，需要根据预读结果调用不同的子程序

		case TokenType::IDENTIFIER:
			if (!isDeclared(next.value().GetValueString()))//使用了未声明的变量
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
			if (isUninitializedVariable(next.value().GetValueString()))//如果没有初始化
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);

			id_pos = getIndex(next.value().GetValueString());	//得到该标识符的名字，便于找到在栈中的偏移量
			//把他的值再次入栈
			_instructions.emplace_back(Operation::LOD, id_pos);
			break;

		case TokenType::UNSIGNED_INTEGER:
			//TODO
			// 这里借用 <常表达式> 来计算val的值，因为此时不会有符号，故不会生成多余的指令
			unreadToken();
			int32_t val;
			err = analyseConstantExpression(val);
			if (err.has_value())
				return err;
			// 生成一次 LIT 指令加载常量
			_instructions.emplace_back(Operation::LIT, val);
			break;
		case TokenType::LEFT_BRACKET:
			err = analyseExpression();
			if (err.has_value())
				return err;
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
			break;
			// 但是要注意 default 返回的是一个编译错误
		default:
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		}

		// 取负
		if (prefix == -1)
			_instructions.emplace_back(Operation::SUB, 0);
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