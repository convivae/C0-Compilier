#pragma once

#include <cstdint>
#include <utility>

namespace cc0 {
	class Functions final {
	private:
		using int32_t = std::int32_t;
	public:
		friend void swap(Functions& lhs, Functions& rhs);
	public:
		Functions(int32_t nameIndex, int32_t paramSize, int32_t level) : _nameIndex(nameIndex), _paramSize(paramSize), _level(level), _has_detected_ret(false) {}
		Functions(int32_t nameIndex, int32_t paramSize) : _nameIndex(nameIndex), _paramSize(paramSize), _level(1), _has_detected_ret(false) {}
		Functions(const Functions& i) { _nameIndex = i._nameIndex; _paramSize = i._paramSize; _level = i._level,_has_detected_ret = i._has_detected_ret; }
		Functions& operator=(Functions i) { swap(*this, i); return *this; }
		bool operator==(const Functions& i) const { return _nameIndex == i._nameIndex && _paramSize == i._paramSize && _level == i._level; }

		int32_t GetNameIndex() const { return _nameIndex; }		//��������.constants�е��±�
		int32_t GetParamSize() const { return _paramSize; }		//����ռ�õ�slot��
		int32_t GetLevel() const { return _level; }				//����Ƕ�׵Ĳ㼶

		//��������ʱ�ı仯���仯
		bool GetHasDetectedRetOrNot() { return this->_has_detected_ret; }
		void SetFindRetExpression()  { this->_has_detected_ret = true; }

	private:
		int32_t _nameIndex;	//��������.constants�е��±�
		int32_t _paramSize;	//����ռ�õ�slot��
		int32_t _level;		//����Ƕ�׵Ĳ㼶
		bool _has_detected_ret;	//�Ƿ��Ѿ���⵽�˷�����䣬��ֹvoid�����û���д�������������
	};

	inline void swap(Functions& lhs, Functions& rhs) {
		using std::swap;
		swap(lhs._nameIndex, rhs._nameIndex);
		swap(lhs._paramSize, rhs._paramSize);
		swap(lhs._level, rhs._level);
	}
}