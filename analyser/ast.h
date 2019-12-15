#pragma once
#include <instruction/instruction.h>
#include <vector>
#include <optional>

namespace c0 {
	//简而言之，generate()函数的作用就是执行该节点的语义分析以及代码生成，
	//你所需要做的就是在在对应节点的generate()中实现该节点的语义动作（比如字面量节点执行加载常量、变量声明节点操作符号表）。
	//只要调用语法树根部的generate()，就能够得到整棵语法树对应的代码了。
	class AST {
		std::optional<>generate();
	};

	// 表达式的语法树
	struct ExprAST :AST {
		//TODO 返回值为整数或double
		void generate();
	};
	
	//二值表达式
	struct BinaryExprAST :ExprAST {
		//运算符
		Operation op;
		//左右操作数
		ExprAST* lhs, * rhs;

		BinaryExprAST(Operation op, ExprAST* lhs, ExprAST* rhs)
			:op(op), lhs(lhs), rhs(rhs) {}

		void generate() {
			lhs->generate();
			rhs->generate();
			switch (op) {
			case Operation::ADD: output("iadd"); break;
			case Operation::SUB: output("isub"); break;
			default: /* error */
			}
			return /* */;
		}
	};

	struct IntExprAST : ExprAST {
		int value;

		IntExprAST(int value) : value(value){}

		void generate() {
			output("ipush %d", value);
			return /* */;
		}
	};

	struct DoubleExprAST : ExprAST {
		double value;

		DoubleExprAST(double value) : value(value) {}
	};


	//条件语句的语法树
	struct StmtAST:AST {};

	//if语句
	struct IfStmtAST : StmtAST {
		ExprAST* condtion;
		StmtAST* thenStmt;
		StmtAST* elseStmt;
	};

	struct CompoundStmtAST : StmtAST {
		std::vector<StmtAST*> stmts; // StmtAST* stmts[];
	};


}
