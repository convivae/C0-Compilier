#pragma once

#include <cstdint>
#include <utility>
#include <string>
#include <variant>

namespace cc0 {
	enum Type {
		STRING_TYPE = 0,	// S
		INT_TYPE = 1,		// I
		DOUBLE_TYPE = 2		// D
	};

	class Constants final {
	private:
		using int32_t = std::int32_t;
	public:
		friend void swap(Constants& lhs, Constants& rhs);
	public:
		Constants(const Type type, std::variant<std::string, std::int32_t, double> value) : _type(type), _value(std::move(value)) {}
		Constants(const Constants& i) { _value = i._value; _type = i._type; }
		Constants& operator=(Constants i) { swap(*this, i); return *this; }
		bool operator==(const Constants& i) const { return _value == i._value && _type == i._type; }

		std::string GetType() { nameOfType(_type); }
		auto GetValue() const { return _value; }
	private:
		Type _type;
		std::variant<std::string, std::int32_t, double> _value;

		std::string nameOfType(Type& t) {
			switch (t) {
			case Type::STRING_TYPE:
				return "S";
			case Type::DOUBLE_TYPE:
				return "D";
			case Type::INT_TYPE:
				return "I";
			}
		}
	};

	inline void swap(Constants& lhs, Constants& rhs) {
		using std::swap;
		swap(lhs._type, rhs._type);
		swap(lhs._value, rhs._value);
	}

}
