#pragma once

#include <cstdint>
#include <utility>
#include <unordered_map>

namespace cc0 {
	enum Operation {
		//�ڴ����ָ��
		nop = 0,
		bipush,		//�����ֽ�ֵbyte������intֵvalue��ѹ��ջ��byte������8λ�޷�����������
		ipush,		//��intֵvalueѹ��ջ��value������32λ�з����������͡�
		pop,		//��ջ������ 1 ��slot
		pop2,		//��ջ������ 2 ��slot
		popn,		//��ջ������ n ��slot,count����32λ�޷����������͡�
		dup,		//����ջ����1��slot����ջ
		dup2,		//����ջ����2��slot����ջ��value1(1), value2(1), value1(1), value2(1)
		loadc,		//���س������±�Ϊindex�ĳ���ֵvalue��valueռ�õ�slot��ȡ���ڳ���������,index��16λ�޷�����������
		loada,		//��SL����ǰ�ƶ�level_diff�Σ��ƶ�����ǰջ֡��β�Ϊlevel_diff��ջ֡�У������ظ�ջ֡��ջƫ��Ϊoffset���ڴ��ջ��ֵַaddress��level_diff��16λ�޷�����������,offset��32λ�з�����������
		_new,		//����ֱ��дnew����Ϊ��hnew������ջ����intֵcount���ڶ��Ϸ��������Ĵ�СΪcount��slot���ڴ棬Ȼ������ڴ���׵�ַaddressѹ��ջ���ڴ��ֵ��֤����ʼ��Ϊ0��
		snew,		//��ջ�����������СΪ count��slot���ڴ�,�ڴ��ֵ����֤����ʼ��Ϊ0,count��32λ�޷�����������
		iload,		//���ڴ��ַaddress������һ��ָ�����͵�ֵ,address������ջ��ַҲ�����Ƕѵ�ַ|..., address(1)| ..., value(T)
		dload,		//���ڴ��ַaddress������һ��ָ�����͵�ֵ,address������ջ��ַҲ�����Ƕѵ�ַ
		aload,		//���ڴ��ַaddress������һ��ָ�����͵�ֵ,address������ջ��ַҲ�����Ƕѵ�ַ
		iaload,		//����ַaddress��Ϊ�����׵�ַ�����������±�Ϊindex����ָ�����͵�ֵvalue,address������ջ��ַҲ�����Ƕѵ�ַ
		daload,		//����ַaddress��Ϊ�����׵�ַ�����������±�Ϊindex����ָ�����͵�ֵvalue,address������ջ��ַҲ�����Ƕѵ�ַ
		aaload,		//����ַaddress��Ϊ�����׵�ַ�����������±�Ϊindex����ָ�����͵�ֵvalue,address������ջ��ַҲ�����Ƕѵ�ַ
		istore,		//��ָ�����͵�ֵvalue�����ڴ��ַaddress����C�����еȼ��� *address = value,address������ջ��ַҲ�����Ƕѵ�ַ
		dstore,		//��ָ�����͵�ֵvalue�����ڴ��ַaddress����C�����еȼ��� *address = value,address������ջ��ַҲ�����Ƕѵ�ַ
		astore,		//��ָ�����͵�ֵvalue�����ڴ��ַaddress����C�����еȼ��� *address = value,address������ջ��ַҲ�����Ƕѵ�ַ
		iastore,	//����ַaddress��Ϊ�����׵�ַ����ָ�����͵�ֵvalue���������±�Ϊindex����
		dastore,	//����ַaddress��Ϊ�����׵�ַ����ָ�����͵�ֵvalue���������±�Ϊindex����
		aastore,	//����ַaddress��Ϊ�����׵�ַ����ָ�����͵�ֵvalue���������±�Ϊindex����

		//��������ָ��
		iadd,		//����ջ��rhs�ʹ�ջ��lhs����lhs + rhs��ֵresultѹջ�����result����int��ֵ���ڣ���ô�ضϸ�λ����Ȼ�����
		dadd,		//��ֵ��ѭIEEE����������
		isub,		//����ջ��rhs�ʹ�ջ��lhs����lhs-rhs��ֵ result ѹջ��
		dsub,		//����ջ��rhs�ʹ�ջ��lhs����lhs-rhs��ֵ result ѹջ��
		imul,		//����ջ��rhs�ʹ�ջ��lhs����lhs*rhs��ֵ result ѹջ��
		dmul,
		idiv,		//����ջ��rhs�ʹ�ջ��lhs����lhs/rhs��ֵ result ѹջ����� rhs ��0�����׳��쳣
		ddiv,
		ineg,		//����ջ��value����-value��ֵresultѹջ
		dneg,		//����ջ��value����-value��ֵresultѹջ
		icmp,		//����ջ��rhs�ʹ�ջ��lhs�������ȽϽ����intֵ result ѹջ,��� lhs �ϴ���result��1
		dcmp,		//����ջ��rhs�ʹ�ջ��lhs�������ȽϽ����intֵ result ѹջ���������ڸ�����+0��-0Ҳ���

		//����ת��ָ��
		i2d,		//����ջ����intֵvalue��ת��Ϊdoubleֵresult��ѹջ��
		d2i,		//����ջ����doubleֵvalue��ת��Ϊintֵresult��ѹջ��
		i2c,		//����ջ����intֵvalue���ضϵ�char��ֵ���ڣ��ٽ�������չ�õ�result��ѹջ������������ܴ��ھ�����ʧ��Ҳ���ܸı���š�

		//����ת��ָ��
		jmp,		//ջ�������仯��ֱ�ӽ�����ת��֮��Ŀ��ƴӵ�ǰ�����������ĵ�ַoffset����ʼִ�С�offset��16λ�޷����������͡�
		je,		//������תָ���ջ����intֵvalue�����value����value��0���������ת��֮��Ŀ��ƴӵ�ǰ�����������ĵ�ַoffset����ʼִ�С�
		jne,		//������תָ���ջ����intֵvalue�����value����value����0���������ת��֮��Ŀ��ƴӵ�ǰ�����������ĵ�ַoffset����ʼִ�С�
		jl,		//������תָ���ջ����intֵvalue�����value����value�Ǹ������������ת��֮��Ŀ��ƴӵ�ǰ�����������ĵ�ַoffset����ʼִ�С�
		jge,		//������תָ���ջ����intֵvalue�����value����value���Ǹ������������ת��֮��Ŀ��ƴӵ�ǰ�����������ĵ�ַoffset����ʼִ�С�
		jg,		//������תָ���ջ����intֵvalue�����value����value���������������ת��֮��Ŀ��ƴӵ�ǰ�����������ĵ�ַoffset����ʼִ�С�
		jle,		//������תָ���ջ����intֵvalue�����value����value�����������������ת��֮��Ŀ��ƴӵ�ǰ�����������ĵ�ַoffset����ʼִ�С�
		call,		//���Һ��������±�Ϊindex�ĺ�����������Ҫ�Ĳ���ȫ����ջ������׼�����µ�������Ϣ֮�󽫲����ٴ���ջ������ת�Ƶ��ú����Ŀ�ʼ��
		ret,		//��������ջ�����٣�����ջ���ָ�������Ϣ��������ת�Ƶ�ԭ��������callָ�����һ��ָ�
		iret,		//��ջ��ָ�����͵�ֵrtv��ջ��Ϊ����ֵ������ջ���ָ�������Ϣ��������ֵrtvѹջ��������ת�Ƶ�ԭ��������callָ�����һ��ָ�
		dret,		//��ջ��ָ�����͵�ֵrtv��ջ��Ϊ����ֵ������ջ���ָ�������Ϣ��������ֵrtvѹջ��������ת�Ƶ�ԭ��������callָ�����һ��ָ�
		aret,		//��ջ��ָ�����͵�ֵrtv��ջ��Ϊ����ֵ������ջ���ָ�������Ϣ��������ֵrtvѹջ��������ת�Ƶ�ԭ��������callָ�����һ��ָ�

		//�������ܵ�ָ��
		iprint,		//����ջ����value��������һ����ʽ�������׼�����
		dprint,		//����ջ����value��������һ����ʽ�������׼�����
		cprint,		//����ջ����value��������һ����ʽ�������׼�����
		sprint,	//����ջ����addr��������Ϊһ���ַ������׵�ַ����ÿ�� slot ��ֵͨ�� cprint �����ֱ�� slot ֵ�� 0������ printf("%s",str)��
		printl,		//ջ�ޱ仯��������С�
		iscan,		//�ӱ�׼������з��ŵ�ʮ������������ѹջ�����õ���ֵvalue
		dscan,		//�ӱ�׼������з��ŵ�ʮ���Ƹ���������ѹջ�����õ���ֵvalue
		cscan,		//�ӱ�׼����һ���ֽ�ֵ����ѹջ�����õ���ֵvalue
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
		Instruction(Operation opr, int32_t param1, int32_t param2) : _opr(opr), _param1(param1), _param2(param2) {}
		
		//Instruction() : Instruction(Operation::ILL, 0){}
		Instruction(const Instruction& i) { _opr = i._opr; _param1 = i._param1; _param2 = i._param2; }
		//Instruction(Instruction&& i) :Instruction() { swap(*this, i); }
		Instruction& operator=(Instruction i) { swap(*this, i); return *this; }
		bool operator==(const Instruction& i) const { return _opr == i._opr && _param1 == i._param1 && _param2 == i._param2; }

		Operation GetOperation() const { return _opr; }
		int32_t GetX() const { return _param1; }
		int32_t GetY() const { return _param2; }
		void SetX(const int32_t label1) { this->_param1 = label1; }
		void SetY(const int32_t label2) { this->_param1 = label2; }
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
