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

	// <C0-program> :: =
	// { <variable-declaration> } {<function-definition>}
	std::optional<CompilationError> Analyser::analyseProgram() {
		while(true) {
			
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
		if(next.has_value() && next.value().GetType() == TokenType::CONST) {
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
	// '{' < statement - seq > '}'
	// 	| <condition-statement>
	// 	| <loop-statement>
	// 	| <jump-statement>
	// 	| <print-statement>
	// 	| <scan-statement>
	// 	| <assignment-expression>';'
	// 	| <function-call>';'
	// 	| ';'
	std::optional<CompilationError> Analyser::analyseStatement() {

		return {};
	}

	std::optional<CompilationError> Analyser::analyseExpression(){
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