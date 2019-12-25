#include "tokenizer/tokenizer.h"

#include <cctype>
#include <sstream>

namespace cc0 {

	std::pair<std::optional<Token>, std::optional<CompilationError>> Tokenizer::NextToken() {
		if (!_initialized)
			readAll();
		if (_rdr.bad())
			return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrStreamError));
		if (isEOF())
			return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrEOF));
		auto p = nextToken();
		if (p.second.has_value())
			return std::make_pair(p.first, p.second);
		auto err = checkToken(p.first.value());
		if (err.has_value())
			return std::make_pair(p.first, err.value());
		return std::make_pair(p.first, std::optional<CompilationError>());
	}

	std::pair<std::vector<Token>, std::optional<CompilationError>> Tokenizer::AllTokens() {
		std::vector<Token> result;
		while (true) {
			auto p = NextToken();
			if (p.second.has_value()) {
				if (p.second.value().GetCode() == ErrorCode::ErrEOF)
					return std::make_pair(result, std::optional<CompilationError>());
				else
					return std::make_pair(std::vector<Token>(), p.second);
			}
			result.emplace_back(p.first.value());
		}
	}

	// 注意：这里的返回值中 Token 和 CompilationError 只能返回一个，不能同时返回。
	std::pair<std::optional<Token>, std::optional<CompilationError>> Tokenizer::nextToken() {
		// 用于存储已经读到的组成当前token字符
		std::stringstream ss;
		// 分析token的结果，作为此函数的返回值
		std::pair<std::optional<Token>, std::optional<CompilationError>> result;
		// <行号，列号>，表示当前token的第一个字符在源代码中的位置
		std::pair<int64_t, int64_t> pos;
		// 记录当前自动机的状态，进入此函数时是初始状态
		DFAState current_state = DFAState::INITIAL_STATE;
		// 这是一个死循环，除非主动跳出
		// 每一次执行while内的代码，都可能导致状态的变更
		while (true) {
			// 读一个字符，请注意auto推导得出的类型是std::optional<char>
			// 这里其实有两种写法
			// 1. 每次循环前立即读入一个 char
			// 2. 只有在可能会转移的状态读入一个 char
			// 因为我们实现了 unread，为了省事我们选择第一种
			auto current_char = nextChar();
			// 针对当前的状态进行不同的操作
			switch (current_state) {

				// 初始状态
				// 这个 case 我们给出了核心逻辑，但是后面的 case 不用照搬。
			case INITIAL_STATE: {
				// 已经读到了文件尾
				if (!current_char.has_value())
					// 返回一个空的token，和编译错误ErrEOF：遇到了文件尾
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrEOF));

				// 获取读到的字符的值，注意auto推导出的类型是char
				auto ch = current_char.value();
				// 标记是否读到了不合法的字符，初始化为否
				auto invalid = false;

				// 使用了自己封装的判断字符类型的函数，定义于 tokenizer/utils.hpp
				// see https://en.cppreference.com/w/cpp/string/byte/isblank
				if (cc0::isspace(ch)) // 读到的字符是空白字符（空格、换行、制表符等）
					current_state = DFAState::INITIAL_STATE; // 保留当前状态为初始状态，此处直接break也是可以的
				else if (!cc0::isprint(ch)) // control codes and backspace
					invalid = true;
				else if (cc0::isdigit(ch)) // 读到的字符是数字
					current_state = DFAState::NUMBER_STATE; // 切换到数字状态
				else if (cc0::isalpha(ch)) // 读到的字符是英文字母
					current_state = DFAState::IDENTIFIER_STATE; // 切换到标识符的状态
				else {
					switch (ch) {
					case '=': // 如果读到的字符是`=`，则切换到等于号的状态
						current_state = DFAState::EQUAL_SIGN_STATE;
						break;
					case '-':
						// 请填空：切换到减号的状态
						current_state = DFAState::MINUS_SIGN_STATE;
						break;
					case '+':
						// 请填空：切换到加号的状态
						current_state = DFAState::PLUS_SIGN_STATE;
						break;
					case '*':
						current_state = DFAState::MULTIPLICATION_SIGN_STATE;
						break;
					case '/':
						current_state = DFAState::DIVISION_SIGN_AND_NOTE_STATE;
						break;
					case '<':
						current_state = DFAState::LESS_SIGN_STATE;
						break;
					case '>':
						current_state = DFAState::ABOVE_SIGN_STATE;
						break;
					case '!':
						current_state = DFAState::EXCLAMATION_SIGN_STATE;
						break;
					case ';':
						current_state = DFAState::SEMICOLON_STATE;
						break;
					case ',':
						current_state = DFAState::COMMA_STATE;
						break;
					case '(':
						current_state = DFAState::LEFT_PARENTHESIS_STATE;
						break;
					case ')':
						current_state = DFAState::RIGHT_PARENTHESIS_STATE;
						break;
					case '{':
						current_state = DFAState::LEFT_BRACE_STATE;
						break;
					case '}':
						current_state = DFAState::RIGHT_BRACE_STATE;
						break;
						// 不接受的字符导致的不合法的状态
					default:
						invalid = true;
						break;
					}
				}
				// 如果读到的字符导致了状态的转移，说明它是一个token的第一个字符
				if (current_state != DFAState::INITIAL_STATE)
					pos = previousPos(); // 记录该字符的的位置为token的开始位置
				// 读到了不合法的字符
				if (invalid) {
					// 回退这个字符
					unreadLast();
					// 返回编译错误：非法的输入
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidInput));
				}
				// 如果读到的字符导致了状态的转移，说明它是一个token的第一个字符
				if (current_state != DFAState::INITIAL_STATE && current_state != DFAState::DIVISION_SIGN_AND_NOTE_STATE) // ignore white spaces and notes
					ss << ch; // 存储读到的字符
				break;
			}

							  // 当前状态是整数时
			case NUMBER_STATE: {
				if (ss.str().at(0) == '0') {
					if (!current_char.has_value()) {//文件末尾
						return std::make_pair(std::make_optional<Token>(TokenType::DECIMAL_INTEGER, 0, pos, currentPos()), std::optional<CompilationError>());
					}
					else if (charInString(current_char.value(), "xX")) {
						ss << current_char.value();
						current_state = DFAState::HEXADECIMAL_STATE;
					}
					else if (charInString(current_char.value(), "0123456789.eE")) {
						ss << current_char.value();
						current_state = DFAState::FLOATING_POINT_STATE;
					}
					else {//十进制 0
						unreadLast();
						return std::make_pair(std::make_optional<Token>(TokenType::DECIMAL_INTEGER, 0, pos, currentPos()), std::optional<CompilationError>());
					}
				}
				else {
					//十进制数
					unreadLast();
					current_state = DFAState::DECIMAL_STATE;
				}
				break;
			}
			case DECIMAL_STATE: {
				// 如果当前已经读到了文件尾，则解析已经读到的字符串为整数
				if (!current_char.has_value()) {
					int32_t int_value;
					ss >> int_value;
					if (ss.fail() || ss.bad())
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrIntegerOverflow));

					return std::make_pair(std::make_optional<Token>(TokenType::DECIMAL_INTEGER, std::to_string(int_value), pos, currentPos()), std::optional<CompilationError>());
				}
				auto ch = current_char.value();

				// 如果读到的字符是数字，则存储读到的字符
				if (cc0::isdigit(ch)) {
					ss << ch;
				}
				// 如果读到的是字母，则存储读到的字符，并切换状态到标识符
				else if (cc0::isalpha(ch)) {
					ss << ch;
					current_state = DFAState::IDENTIFIER_STATE;
				}
				// 如果读到的字符不是上述情况之一，则回退读到的字符，并解析已经读到的字符串为整数
				else {
					unreadLast();
					int32_t int_value;
					ss >> int_value;
					if (ss.fail() || ss.bad())
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrIntegerOverflow));

					return std::make_pair(std::make_optional<Token>(TokenType::DECIMAL_INTEGER, std::to_string(int_value), pos, currentPos()), std::optional<CompilationError>());
				}
				//     解析成功则返回无符号整数类型的token，否则返回编译错误
				break;
			}
			case FLOATING_POINT_STATE: {
				//TODO 留待扩展C0处理
				return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrDectedFloatingPointNumber));
			}

			case HEXADECIMAL_STATE: {
				if (!current_char.has_value()) {//十六进制转成十进制返回
					std::string int_str;
					ss >> int_str;
					auto int_value = strtoll(int_str.c_str(), NULL, 16);
					if (int_value > INT32_MAX || int_value < INT32_MIN) {
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrIntegerOverflow));
					}
					return std::make_pair(std::make_optional<Token>(TokenType::HEXADECIMAL_INTEGER, std::to_string(int_value), pos, currentPos()), std::optional<CompilationError>());
				}
				auto ch = current_char.value();

				// 如果读到的字符是符合规范的十六进制字符，则存储读到的字符
				if (isHexCharacter(ch)) {
					ss << ch;
				}
				// 如果读到的是字母，则存储读到的字符，并切换状态到标识符
				else if (cc0::isalpha(ch)) {
					ss << ch;
					current_state = DFAState::IDENTIFIER_STATE;
				}
				// 如果读到的字符不是上述情况之一，则回退读到的字符，并解析已经读到的字符串为整数
				else {
					unreadLast();
					std::string int_str;
					ss >> int_str;
					auto int_value = strtoll(int_str.c_str(), NULL, 16);
					if (int_value > INT32_MAX || int_value < INT32_MIN) {
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrIntegerOverflow));
					}
					return std::make_pair(std::make_optional<Token>(TokenType::HEXADECIMAL_INTEGER, std::to_string(int_value), pos, currentPos()), std::optional<CompilationError>());
				}
				break;
			}

			case IDENTIFIER_STATE: {
				// 请填空：
				// 如果当前已经读到了文件尾，则解析已经读到的字符串
				if (!current_char.has_value()) {
					//     如果解析结果是关键字，那么返回对应关键字的token，否则返回标识符的token
					std::string str_value;
					ss >> str_value;

					auto tmp_token = dealReservedWord(str_value);
					if (tmp_token.has_value()) {
						return std::make_pair(std::make_optional<Token>(tmp_token.value(), str_value, pos, currentPos()), std::optional<CompilationError>());
					}

					return std::make_pair(std::make_optional<Token>(TokenType::IDENTIFIER, str_value, pos, currentPos()), std::optional<CompilationError>());
				}

				auto ch = current_char.value();

				// 如果读到的字符是数字，则存储读到的字符
				if (cc0::isalpha(ch) || cc0::isdigit(ch)) {
					ss << ch;
				}
				// 如果读到的字符不是上述情况之一，则回退读到的字符，并解析已经读到的字符串
				else {
					unreadLast();
					//     如果解析结果是关键字，那么返回对应关键字的token，否则返回标识符的token
					std::string str_value;
					ss >> str_value;

					auto tmp_token = dealReservedWord(str_value);
					if (tmp_token.has_value()) {
						return std::make_pair(std::make_optional<Token>(tmp_token.value(), str_value, pos, currentPos()), std::optional<CompilationError>());
					}

					return std::make_pair(std::make_optional<Token>(TokenType::IDENTIFIER, str_value, pos, currentPos()), std::optional<CompilationError>());
				}
				break;
			}

			case DIVISION_SIGN_AND_NOTE_STATE: {
				auto ch = current_char.value();
				if(ch == '/') {
					while(ch != '\n') {
						current_char = nextChar();

						//读到了文件尾
						if (!current_char.has_value())
							return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrEOF));
						ch = current_char.value();
					}
					// 这里 \n 被舍弃
				}
				else if(ch == '*') {
					bool detectedStar = false;
					while(true) {
						current_char = nextChar();

						//读到了文件尾
						if (!current_char.has_value())
							return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrEOF));
						ch = current_char.value();

						if(detectedStar == true) {
							if(ch == '/') {
								break;	//注释结束
							}
							else {
								detectedStar = false;	//仍在注释里面
							}
						}

						if (ch == '*') {
							detectedStar = true;
						}
					}
				}
				else {	//确定是除号
					unreadLast();
					return std::make_pair(std::make_optional<Token>(TokenType::DIVISION_SIGN, '/', pos, currentPos()), std::optional<CompilationError>());
				}

				//当前状态重新恢复为初始状态
				current_state = DFAState::INITIAL_STATE;
				
				break;
			}
			case EQUAL_SIGN_STATE: {
				auto ch = current_char.value();
				
				if(ch != '=') {
					unreadLast();
					return std::make_pair(std::make_optional<Token>(TokenType::EQUAL_SIGN, '=', pos, currentPos()), std::optional<CompilationError>());
				}
				else {
					ss << ch;
					std::string str_value;
					ss >> str_value;
					return std::make_pair(std::make_optional<Token>(TokenType::EQUAL_EQUAL_SIGN, str_value, pos, currentPos()), std::optional<CompilationError>());
				}
				break;
			}
			case LESS_SIGN_STATE: {
				auto ch = current_char.value();

				if (ch != '=') {
					unreadLast();
					return std::make_pair(std::make_optional<Token>(TokenType::LESS_SIGN, '<', pos, currentPos()), std::optional<CompilationError>());
				}
				else {
					ss << ch;
					std::string str_value;
					ss >> str_value;
					return std::make_pair(std::make_optional<Token>(TokenType::LESS_EQUAL_SIGN, str_value, pos, currentPos()), std::optional<CompilationError>());
				}
				break;
			}

			case ABOVE_SIGN_STATE: {
				auto ch = current_char.value();

				if (ch != '=') {
					unreadLast();
					return std::make_pair(std::make_optional<Token>(TokenType::ABOVE_SIGN, '>', pos, currentPos()), std::optional<CompilationError>());
				}
				else {
					ss << ch;
					std::string str_value;
					ss >> str_value;
					return std::make_pair(std::make_optional<Token>(TokenType::ABOVE_EQUAL_SIGN, str_value, pos, currentPos()), std::optional<CompilationError>());
				}
				break;
			}

			case EXCLAMATION_SIGN_STATE: {
				auto ch = current_char.value();

				if (ch != '=') {	//如果感叹号后面没有跟等于号，直接报错
					unreadLast();
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrInvalidInput));
				}
				else {
					ss << ch;
					std::string str_value;
					ss >> str_value;
					return std::make_pair(std::make_optional<Token>(TokenType::NOT_EQUAL_SIGN, str_value, pos, currentPos()), std::optional<CompilationError>());
				}
				break;
			}


								 // 如果当前状态是加号
			case PLUS_SIGN_STATE: {
				// 请思考这里为什么要回退，在其他地方会不会需要
				unreadLast(); // Yes, we unread last char even if it's an EOF.
				return std::make_pair(std::make_optional<Token>(TokenType::PLUS_SIGN, '+', pos, currentPos()), std::optional<CompilationError>());
			}
								// 当前状态为减号的状态
			case MINUS_SIGN_STATE: {
				// 请填空：回退，并返回减号token
				unreadLast(); // Yes, we unread last char even if it's an EOF.
				return std::make_pair(std::make_optional<Token>(TokenType::MINUS_SIGN, '-', pos, currentPos()), std::optional<CompilationError>());
			}
			case MULTIPLICATION_SIGN_STATE: {
				unreadLast();
				return std::make_pair(std::make_optional<Token>(TokenType::MULTIPLICATION_SIGN, '*', pos, currentPos()), std::optional<CompilationError>());
			}
			case SEMICOLON_STATE: {
				unreadLast();
				return std::make_pair(std::make_optional<Token>(TokenType::SEMICOLON, ';', pos, currentPos()), std::optional<CompilationError>());
			}
			case LEFT_PARENTHESIS_STATE: {
				unreadLast();
				return std::make_pair(std::make_optional<Token>(TokenType::LEFT_PARENTHESIS, '(', pos, currentPos()), std::optional<CompilationError>());
			}
			case RIGHT_PARENTHESIS_STATE: {
				unreadLast();
				return std::make_pair(std::make_optional<Token>(TokenType::RIGHT_PARENTHESIS, ')', pos, currentPos()), std::optional<CompilationError>());
			}
			case LEFT_BRACE_STATE: {
				unreadLast();
				return std::make_pair(std::make_optional<Token>(TokenType::LEFT_BRACE, '{', pos, currentPos()), std::optional<CompilationError>());
			}
			case RIGHT_BRACE_STATE: {
				unreadLast();
				return std::make_pair(std::make_optional<Token>(TokenType::RIGHT_BRACE, '}', pos, currentPos()), std::optional<CompilationError>());
			}
			case COMMA_STATE: {
				unreadLast();
				return std::make_pair(std::make_optional<Token>(TokenType::COMMA, ',', pos, currentPos()), std::optional<CompilationError>());
			}
							// 预料之外的状态，如果执行到了这里，说明程序异常
			default:
				DieAndPrint("unhandled state.");
				break;
			}
		}
		// 预料之外的状态，如果执行到了这里，说明程序异常
		return std::make_pair(std::optional<Token>(), std::optional<CompilationError>());
	}

	std::optional<CompilationError> Tokenizer::checkToken(const Token& t) {
		switch (t.GetType()) {
		case IDENTIFIER: {
			auto val = t.GetValueString();
			if (cc0::isdigit(val[0]))
				return std::make_optional<CompilationError>(t.GetStartPos().first, t.GetStartPos().second, ErrorCode::ErrInvalidIdentifier);
			break;
		}
		default:
			break;
		}
		return {};
	}

	void Tokenizer::readAll() {
		if (_initialized)
			return;
		for (std::string tp; std::getline(_rdr, tp);)
			_lines_buffer.emplace_back(std::move(tp + "\n"));
		_initialized = true;
		_ptr = std::make_pair<int64_t, int64_t>(0, 0);
		return;
	}

	// Note: We allow this function to return a postion which is out of bound according to the design like std::vector::end().
	std::pair<uint64_t, uint64_t> Tokenizer::nextPos() {
		if (_ptr.first >= _lines_buffer.size())
			DieAndPrint("advance after EOF");
		if (_ptr.second == _lines_buffer[_ptr.first].size() - 1)
			return std::make_pair(_ptr.first + 1, 0);
		else
			return std::make_pair(_ptr.first, _ptr.second + 1);
	}

	std::pair<uint64_t, uint64_t> Tokenizer::currentPos() {
		return _ptr;
	}

	std::pair<uint64_t, uint64_t> Tokenizer::previousPos() {
		if (_ptr.first == 0 && _ptr.second == 0)
			DieAndPrint("previous position from beginning");
		if (_ptr.second == 0)
			return std::make_pair(_ptr.first - 1, _lines_buffer[_ptr.first - 1].size() - 1);
		else
			return std::make_pair(_ptr.first, _ptr.second - 1);
	}

	std::optional<char> Tokenizer::nextChar() {
		if (isEOF())
			return {}; // EOF
		auto result = _lines_buffer[_ptr.first][_ptr.second];
		_ptr = nextPos();
		return result;
	}

	bool Tokenizer::isEOF() {
		return _ptr.first >= _lines_buffer.size();
	}

	// Note: Is it evil to unread a buffer?
	void Tokenizer::unreadLast() {
		_ptr = previousPos();
	}

	std::optional<TokenType> Tokenizer::dealReservedWord(const std::string& s) {
		if (s == "const")
			return TokenType::CONST;
		if (s == "void")
			return TokenType::VOID;
		if (s == "int")
			return TokenType::INT;
		if (s == "char")
			return TokenType::CHAR;
		if (s == "double")
			return TokenType::DOUBLE;
		if (s == "struct")
			return TokenType::STRUCT;
		if (s == "if")
			return TokenType::IF;
		if (s == "else")
			return TokenType::ELSE;
		if (s == "switch")
			return TokenType::SWITCH;
		if (s == "case")
			return TokenType::CASE;
		if (s == "default")
			return TokenType::DEFAULT;
		if (s == "while")
			return TokenType::WHILE;
		if (s == "for")
			return TokenType::FOR;
		if (s == "do")
			return TokenType::DO;
		if (s == "return")
			return TokenType::RETURN;
		if (s == "break")
			return TokenType::BREAK;
		if (s == "continue")
			return TokenType::CONTINUE;
		if (s == "print")
			return TokenType::PRINT;
		if (s == "scan")
			return TokenType::SCAN;
		return {};
	}

	bool Tokenizer::charInString(const char c, std::string s)
	{
		return s.find(c) != std::string::npos;
	}

	bool Tokenizer::isHexCharacter(char c)
	{
		std::string hex = "0123456789abcdefABCDEF";
		return charInString(c, hex);
	}
}