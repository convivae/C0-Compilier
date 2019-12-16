#pragma once
#include <instruction/instruction.h>
#include <vector>
#include <optional>

namespace cc0 {
	//�����֮��generate()���������þ���ִ�иýڵ����������Լ��������ɣ�
	//������Ҫ���ľ������ڶ�Ӧ�ڵ��generate()��ʵ�ָýڵ�����嶯���������������ڵ�ִ�м��س��������������ڵ�������ű�����
	//ֻҪ�����﷨��������generate()�����ܹ��õ������﷨����Ӧ�Ĵ����ˡ�
	class AST {
		std::optional<int>generate();
	};

	// ����ʽ���﷨��
	struct ExprAST :AST {
		//TODO ����ֵΪ������double
		void generate();
	};
	
	//��ֵ����ʽ
	struct BinaryExprAST :ExprAST {
		//�����
		Operation op;
		//���Ҳ�����
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


	//���������﷨��
	struct StmtAST:AST {};

	//if���
	struct IfStmtAST : StmtAST {
		ExprAST* condtion;
		StmtAST* thenStmt;
		StmtAST* elseStmt;
	};

	struct CompoundStmtAST : StmtAST {
		std::vector<StmtAST*> stmts; // StmtAST* stmts[];
	};


}
