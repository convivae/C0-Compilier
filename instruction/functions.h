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
		Functions(int32_t nameIndex, int32_t paramSize, int32_t level) : _nameIndex(nameIndex), _paramSize(paramSize), _level(level) {}
		Functions(int32_t nameIndex, int32_t paramSize) : _nameIndex(nameIndex), _paramSize(paramSize), _level(1) {}
		Functions(const Functions& i) { _nameIndex = i._nameIndex; _paramSize = i._paramSize; _level = i._level; }
		Functions& operator=(Functions i) { swap(*this, i); return *this; }
		bool operator==(const Functions& i) const { return _nameIndex == i._nameIndex && _paramSize == i._paramSize && _level == i._level; }

		int32_t GetNameIndex() const { return _nameIndex; }		//函数名在.constants中的下标
		int32_t GetParamSize() const { return _paramSize; }		//参数占用的slot数
		int32_t GetLevel() const { return _level; }				//函数嵌套的层级
	private:
		int32_t _nameIndex;	//函数名在.constants中的下标
		int32_t _paramSize;	//参数占用的slot数
		int32_t _level;		//函数嵌套的层级
	};

	inline void swap(Functions& lhs, Functions& rhs) {
		using std::swap;
		swap(lhs._nameIndex, rhs._nameIndex);
		swap(lhs._paramSize, rhs._paramSize);
		swap(lhs._level, rhs._level);
	}
}
