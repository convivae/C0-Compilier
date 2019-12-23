#pragma once

#include "error/error.h"
#include "instruction/instruction.h"
#include "instruction/constants.h"
#include "instruction/functions.h"
#include "tokenizer/token.h"

#include <vector>
#include <optional>
#include <utility>
#include <map>
#include <cstdint>
#include <cstddef> // for std::size_t
#include <algorithm>
#include <set>

namespace cc0 {

	enum TableType {
		START_TYPE,		//汇编写入outpust._start中
		FUN_N_TYPE,		//汇编写入output._funN中
	};

	struct Output {
		std::vector<Constants> _constants;					//常量表，记录函数名称的相关信息，其他一概不记录
		std::vector<Instruction> _start;					//启动代码，负责执行全局变量的初始化
		std::vector<Functions> _functions;					//函数表，记录函数的基本信息
		std::vector<std::vector<Instruction>> _funN;		//函数体，是二维的
	};

	class Analyser final {
	private:
		using uint64_t = std::uint64_t;
		using int64_t = std::int64_t;
		using uint32_t = std::uint32_t;
		using int32_t = std::int32_t;
	public:
		Analyser(std::vector<Token> v)
			: _tokens(std::move(v)), _offset(0), _output({}), _current_pos(0, 0),_main_index(-1),
			_uninitialized_vars({}), _vars({}), _consts({}), _nextTokenIndex(0), _nextLocalTokenIndex(0) {}
		Analyser(Analyser&&) = delete;
		Analyser(const Analyser&) = delete;
		Analyser& operator=(Analyser) = delete;

		// 接口
		std::pair<Output, std::optional<CompilationError>> Analyse();
		
	private:
		// 所有的递归子程序

		// <程序>
		std::optional<CompilationError> analyseProgram();
		// <变量声明>
		std::optional<CompilationError> analyseVariableDeclaration(TableType type);
		// <函数声明>
		std::optional<CompilationError> analyseFunctionDeclaration();
		// <参数从句>
		std::optional<CompilationError> analyseParameterClause(int32_t& paramSize);
		// <变量声明列表>
		std::optional<CompilationError> analyseParameterDeclarationList(int32_t& paramSize);
		// <变量声明>
		std::optional<CompilationError> analyseParameterDeclaration();
		std::optional<CompilationError> analyseCompoundStatement();
		std::optional<CompilationError> analyseStatementSeq();
		std::optional<CompilationError> analyseStatement();
		std::optional<CompilationError> analyseCondition(int32_t& label1);
		std::optional<CompilationError> analyseFunctionCall(TableType type);
		std::optional<CompilationError> analyseExpressionList(TableType type, int32_t& paramSize);
		std::optional<CompilationError> analyseAssignmentExpression();
		std::optional<CompilationError> analyseScanStatement();
		std::optional<CompilationError> analysePrintStatement();
		std::optional<CompilationError> analysePrintableList();
		std::optional<CompilationError> analyseJumpStatement();
		std::optional<CompilationError> analyseReturnStatement();
		std::optional<CompilationError> analyseLoopStatement();
		std::optional<CompilationError> analyseConditionStatement();
		// <初始化声明列表>
		std::optional<CompilationError> analyseInitDeclaratorList(const bool isConst, TableType type);
		// <表达式>
		std::optional<CompilationError> analyseExpression(TableType type);	//index是当前consts表中的最后一个元素
		std::optional<CompilationError> analyseAdditiveExpression(TableType type);
		std::optional<CompilationError> analyseMultiplicativeExpression(TableType type);
		std::optional<CompilationError> analyseUnaryExpression(TableType type);
		std::optional<CompilationError> analysePrimaryExpression(TableType type);


		// Token 缓冲区相关操作

		// 返回下一个 token
		std::optional<Token> nextToken();
		// 回退一个 token
		void unreadToken();

		// 下面是符号表相关操作

		// helper function
		void _add(const Token&, std::map<std::string, int32_t>&);
		void _add_local(const Token& tk, std::map<std::string, int32_t>& mp);
		void addLocalVariable(const Token& tk);
		void addLocalConstant(const Token& tk);
		void addLocalUninitializedVariable(const Token& tk);
		// 添加变量、常量、未初始化的变量
		void addVariable(const Token&);
		void addConstant(const Token&);
		void addUninitializedVariable(const Token&);
		//添加void、int函数
		void addVoidFunctions(const Token& tk);	//用来判断是否声明过以及return什么
		void addIntFunctions(const Token& tk);	//用来判断是否声明过以及return什么
		// 是否被声明过
		bool isDeclared(const std::string&);
		// 是否是未初始化的变量
		bool isUninitializedVariable(const std::string&);
		// 是否是已初始化的变量
		bool isInitializedVariable(const std::string&);
		// 是否是常量
		bool isConstant(const std::string&);
		//是否是相应类型的函数
		bool isVoidFun(const std::string& s);
		bool isIntFun(const std::string& s);
		bool isIntFun(int32_t index_in_constants);
		bool isVoidFun(int32_t index_in_constants);
		bool isLocalDeclared(const std::string& s);
		bool isLocalUninitializedVariable(const std::string& s);
		bool isLocalInitializedVariable(const std::string& s);
		bool isLocalConstant(const std::string& s);
		bool isMainFun(int32_t index_in_constants);
		//获得在各个表上的偏移量
		int32_t getPos(std::vector<Constants> source, const Constants& value) const;
		int32_t getPos(std::vector<Constants> source, const std::string& value) const;
		int32_t getPos(std::vector<Functions> source, const Functions& value) const;
		int32_t getPos(std::vector<Functions> source, const std::string& value) const;
		// 获得 {变量，常量} 在栈上的偏移
		int32_t getIndex(const std::string&);
		int32_t getLocalIndex(const std::string& s);

		int32_t getFunctionIndexInConstants(const std::string& s);

		int32_t getParamsNum(const std::string& s); //得到函数的参数个数

		//每次函数调用前清空局部变量
		void localClear();

		//判断是不是main函数
	private:
		Output _output;

		std::vector<Token> _tokens;
		std::size_t _offset;
		std::pair<uint64_t, uint64_t> _current_pos;
		int32_t _main_index;		//main函数在 _consts中的位置，初始化为-1，如果不是-1，说明存在main函数

		// 为了简单处理，我们直接把符号表耦合在语法分析里
		// 变量                   示例
		// _uninitialized_vars    var a;
		// _vars                  var a=1;
		// _consts                const a=1;
		// 全局符号表
		std::map<std::string, int32_t> _uninitialized_vars;
		std::map<std::string, int32_t> _vars;
		std::map<std::string, int32_t> _consts;

		std::map<std::string, int32_t> _void_funs;
		std::map<std::string, int32_t> _int_funs;

		//函数内部的参数
		std::map<std::string, int32_t> _local_consts;	//用于判断函数内的变量（包括参数）是否有重复，每次调用一个函数遇到参数时都要清空
		std::map<std::string, int32_t> _local_vars;
		std::map<std::string, int32_t> _local_uninitialized_vars;
		
		//下一个 token 在栈的偏移，此处需要对不同的栈有所区分
		int32_t _nextTokenIndex;
		int32_t _nextLocalTokenIndex;
	};
}
