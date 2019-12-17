#pragma once
#include <instruction/instruction.h>
#include <vector>
#include <optional>

namespace cc0 {
	class AST {
		std::optional<int> generate();
	};

	struct ExprAST :AST {
		//TODO 
		void generate();
	};

	struct BinaryExprAST :ExprAST {
		Operation op;
		ExprAST* lhs, * rhs;

		BinaryExprAST(Operation op, ExprAST* lhs, ExprAST* rhs)
			:op(op), lhs(lhs), rhs(rhs) {}

		void generate() {
			lhs->generate();
			rhs->generate();
			switch (op) {
			// case Operation::ADD: output("iadd"); break;
			// case Operation::SUB: output("isub"); break;
			default:break;
				/* error */
			}
			return /* */;
		}
	};

	struct IntExprAST : ExprAST {
		int value;

		IntExprAST(int value) : value(value){}

		void generate() {
			// output("ipush %d", value);
			return /* */;
		}
	};

	struct DoubleExprAST : ExprAST {
		double value;

		DoubleExprAST(double value) : value(value) {}
	};


	struct StmtAST:AST {};

	struct IfStmtAST : StmtAST {
		ExprAST* condtion;
		StmtAST* thenStmt;
		StmtAST* elseStmt;
	};

	struct CompoundStmtAST : StmtAST {
		std::vector<StmtAST*> stmts; // StmtAST* stmts[];
	};


}
