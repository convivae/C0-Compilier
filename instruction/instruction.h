#pragma once

#include <cstdint>
#include <utility>
#include <unordered_map>

namespace cc0 {
	enum Operation {
		//内存操作指令
		nop = 0,
		bipush,		//将单字节值byte提升至int值value后压入栈，byte将按照8位无符号整数解释
		ipush,		//将int值value压入栈，value将按照32位有符号整数解释。
		pop,		//从栈顶弹出 1 个slot
		pop2,		//从栈顶弹出 2 个slot
		popn,		//从栈顶弹出 n 个slot,count按照32位无符号整数解释。
		dup,		//复制栈顶的1个slot并入栈
		dup2,		//复制栈顶的2个slot并入栈，value1(1), value2(1), value1(1), value2(1)
		loadc,		//加载常量池下标为index的常量值value，value占用的slot数取决于常量的类型,index以16位无符号整数解释
		loada,		//沿SL链向前移动level_diff次（移动到当前栈帧层次差为level_diff的栈帧中），加载该栈帧中栈偏移为offset的内存的栈地址值address。level_diff以16位无符号整数解释,offset以32位有符号整数解释
		_new,		//不能直接写new，改为了hnew，弹出栈顶的int值count，在堆上分配连续的大小为count个slot的内存，然后将这段内存的首地址address压入栈。内存的值保证被初始化为0。
		snew,		//在栈顶连续分配大小为 count个slot的内存,内存的值不保证被初始化为0,count以32位无符号整数解释
		iload,		//从内存地址address处加载一个指定类型的值,address可能是栈地址也可能是堆地址|..., address(1)| ..., value(T)
		dload,		//从内存地址address处加载一个指定类型的值,address可能是栈地址也可能是堆地址
		aload,		//从内存地址address处加载一个指定类型的值,address可能是栈地址也可能是堆地址
		iaload,		//将地址address视为数组首地址，加载数组下标为index处的指定类型的值value,address可能是栈地址也可能是堆地址
		daload,		//将地址address视为数组首地址，加载数组下标为index处的指定类型的值value,address可能是栈地址也可能是堆地址
		aaload,		//将地址address视为数组首地址，加载数组下标为index处的指定类型的值value,address可能是栈地址也可能是堆地址
		istore,		//将指定类型的值value存入内存地址address处。C语言中等价于 *address = value,address可能是栈地址也可能是堆地址
		dstore,		//将指定类型的值value存入内存地址address处。C语言中等价于 *address = value,address可能是栈地址也可能是堆地址
		astore,		//将指定类型的值value存入内存地址address处。C语言中等价于 *address = value,address可能是栈地址也可能是堆地址
		iastore,	//将地址address视为数组首地址，将指定类型的值value存入数组下标为index处。
		dastore,	//将地址address视为数组首地址，将指定类型的值value存入数组下标为index处。
		aastore,	//将地址address视为数组首地址，将指定类型的值value存入数组下标为index处。

		//算数运算指令
		iadd,		//弹出栈顶rhs和次栈顶lhs，将lhs + rhs的值result压栈。如果result不在int的值域内，那么截断高位（自然溢出）
		dadd,		//求值遵循IEEE浮点数运算
		isub,		//弹出栈顶rhs和次栈顶lhs，将lhs-rhs的值 result 压栈。
		dsub,		//弹出栈顶rhs和次栈顶lhs，将lhs-rhs的值 result 压栈。
		imul,		//弹出栈顶rhs和次栈顶lhs，将lhs*rhs的值 result 压栈。
		dmul,
		idiv,		//弹出栈顶rhs和次栈顶lhs，将lhs/rhs的值 result 压栈。如果 rhs 是0，会抛出异常
		ddiv,
		ineg,		//弹出栈顶value，将-value的值result压栈
		dneg,		//弹出栈顶value，将-value的值result压栈
		icmp,		//弹出栈顶rhs和次栈顶lhs，并将比较结果以int值 result 压栈,如果 lhs 较大，则result是1
		dcmp,		//弹出栈顶rhs和次栈顶lhs，并将比较结果以int值 result 压栈。正数大于负数，+0和-0也如此

		//类型转换指令
		i2d,		//弹出栈顶的int值value，转换为double值result并压栈。
		d2i,		//弹出栈顶的double值value，转换为int值result并压栈。
		i2c,		//弹出栈顶的int值value，截断到char的值域内，再进行零扩展得到result并压栈。这个操作可能存在精度损失，也可能改变符号。

		//控制转移指令
		jmp,		//栈不发生变化。直接进行跳转，之后的控制从当前函数代码区的地址offset处开始执行。offset以16位无符号整数解释。
		je,		//条件跳转指令弹出栈顶的int值value，如果value满足value是0，则进行跳转，之后的控制从当前函数代码区的地址offset处开始执行。
		jne,		//条件跳转指令弹出栈顶的int值value，如果value满足value不是0，则进行跳转，之后的控制从当前函数代码区的地址offset处开始执行。
		jl,		//条件跳转指令弹出栈顶的int值value，如果value满足value是负数，则进行跳转，之后的控制从当前函数代码区的地址offset处开始执行。
		jge,		//条件跳转指令弹出栈顶的int值value，如果value满足value不是负数，则进行跳转，之后的控制从当前函数代码区的地址offset处开始执行。
		jg,		//条件跳转指令弹出栈顶的int值value，如果value满足value是正数，则进行跳转，之后的控制从当前函数代码区的地址offset处开始执行。
		jle,		//条件跳转指令弹出栈顶的int值value，如果value满足value不是正数，则进行跳转，之后的控制从当前函数代码区的地址offset处开始执行。
		call,		//查找函数表中下标为index的函数，将其需要的参数全部弹栈，并在准备好新的内务信息之后将参数再次入栈，控制转移到该函数的开始。
		ret,		//被调用者栈被销毁，清理栈，恢复内务信息，将控制转移到原来函数的call指令的下一条指令。
		iret,		//将栈顶指定类型的值rtv弹栈作为返回值，清理栈，恢复内务信息，将返回值rtv压栈，将控制转移到原来函数的call指令的下一条指令。
		dret,		//将栈顶指定类型的值rtv弹栈作为返回值，清理栈，恢复内务信息，将返回值rtv压栈，将控制转移到原来函数的call指令的下一条指令。
		aret,		//将栈顶指定类型的值rtv弹栈作为返回值，清理栈，恢复内务信息，将返回值rtv压栈，将控制转移到原来函数的call指令的下一条指令。

		//辅助功能的指令
		iprint,		//弹出栈顶的value，并根据一定格式输出到标准输出：
		dprint,		//弹出栈顶的value，并根据一定格式输出到标准输出：
		cprint,		//弹出栈顶的value，并根据一定格式输出到标准输出：
		sprint,	//弹出栈顶的addr，将其视为一个字符串的首地址，对每个 slot 的值通过 cprint 输出，直到 slot 值是 0；类似 printf("%s",str)。
		printl,		//栈无变化。输出换行。
		iscan,		//从标准输入可有符号的十进制整数，并压栈解析得到的值value
		dscan,		//从标准输入可有符号的十进制浮点数，并压栈解析得到的值value
		cscan,		//从标准输入一个字节值，并压栈解析得到的值value
	};

#define NAME(op) { Operation::op, #op }
	const std::unordered_map<Operation, std::string> nameOfOpCode = {
		NAME(nop),

		NAME(bipush), NAME(ipush),
		NAME(pop),    NAME(pop2), NAME(popn),
		NAME(dup),    NAME(dup2),
		NAME(loadc),  NAME(loada),
		{Operation::_new, "new"},
		NAME(snew),

		NAME(iload),   NAME(dload),   NAME(aload),
		NAME(iaload),  NAME(daload),  NAME(aaload),
		NAME(istore),  NAME(dstore),  NAME(astore),
		NAME(iastore), NAME(dastore), NAME(aastore),

		NAME(iadd), NAME(dadd),
		NAME(isub), NAME(dsub),
		NAME(imul), NAME(dmul),
		NAME(idiv), NAME(ddiv),
		NAME(ineg), NAME(dneg),
		NAME(icmp), NAME(dcmp),

		NAME(i2d), NAME(d2i), NAME(i2c),

		NAME(jmp),
		NAME(je), NAME(jne), NAME(jl), NAME(jge), NAME(jg), NAME(jle),

		NAME(call),
		NAME(ret),
		NAME(iret), NAME(dret), NAME(aret),

		NAME(iprint), NAME(dprint), NAME(cprint), NAME(sprint),
		NAME(printl),
		NAME(iscan),  NAME(dscan),  NAME(cscan),
	};
#undef NAME
#define NAME(op) { #op, Operation::op }
	const std::unordered_map<std::string, Operation> opCodeOfName = {
		NAME(nop),

		NAME(bipush), NAME(ipush),
		NAME(pop),    NAME(pop2), NAME(popn),
		NAME(dup),    NAME(dup2),
		NAME(loadc),  NAME(loada),
		{"new", Operation::_new},
		NAME(snew),

		NAME(iload),   NAME(dload),   NAME(aload),
		NAME(iaload),  NAME(daload),  NAME(aaload),
		NAME(istore),  NAME(dstore),  NAME(astore),
		NAME(iastore), NAME(dastore), NAME(aastore),

		NAME(iadd), NAME(dadd),
		NAME(isub), NAME(dsub),
		NAME(imul), NAME(dmul),
		NAME(idiv), NAME(ddiv),
		NAME(ineg), NAME(dneg),
		NAME(icmp), NAME(dcmp),

		NAME(i2d), NAME(d2i), NAME(i2c),

		NAME(jmp),
		NAME(je), NAME(jne), NAME(jl), NAME(jge), NAME(jg), NAME(jle),

		NAME(call),
		NAME(ret),
		NAME(iret), NAME(dret), NAME(aret),

		NAME(iprint), NAME(dprint), NAME(cprint), NAME(sprint),
		NAME(printl),
		NAME(iscan),  NAME(dscan),  NAME(cscan),
	};
#undef NAME
	
	class Instruction final {
	private:
		using int32_t = std::int32_t;
	public:
		friend void swap(Instruction& lhs, Instruction& rhs);
	public:
		Instruction(Operation opr, int32_t x, int32_t y) : _opr(opr), _param1(x), _param2(y) {}
		
		//Instruction() : Instruction(Operation::ILL, 0){}
		Instruction(const Instruction& i) { _opr = i._opr; _param1 = i._param1; _param2 = i._param2; }
		//Instruction(Instruction&& i) :Instruction() { swap(*this, i); }
		Instruction& operator=(Instruction i) { swap(*this, i); return *this; }
		bool operator==(const Instruction& i) const { return _opr == i._opr && _param1 == i._param1 && _param2 == i._param2; }

		Operation GetOperation() const { return _opr; }
		int32_t GetX() const { return _param1; }
		int32_t GetY() const { return _param2; }
	private:
		Operation _opr;
		int32_t _param1, _param2;
	};

	inline void swap(Instruction& lhs, Instruction& rhs) {
		using std::swap;
		swap(lhs._opr, rhs._opr);
		swap(lhs._param1, rhs._param1);
		swap(lhs._param2, rhs._param2);
	}
}
