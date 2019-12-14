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
	std::optional<CompilationError> Analyser::analyseVariableDeclaration() {
		//const
		auto next = nextToken();
		if(next.has_value() && next.value().GetType() == TokenType::CONST) {
			next = nextToken();
		}
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
	}

	// <init-declarator-list> ::= 
	// <init-declarator>{',' <init-declarator> }
	std::optional<CompilationError> Analyser::analyseInitDeclaratorList() {
		return {};
	}


	
	// <function-definition> ::= 
	// <type-specifier><identifier><parameter-clause><compound-statement>
	std::optional<CompilationError> Analyser::analyseFunctionDeclaration() {
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