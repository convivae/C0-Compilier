- 需要识别的token：

  > - <reserved-word> ::= 
  >        'const'
  >       |'void'   |'int'    |'char'   |'double'
  >       |'struct'
  >       |'if'     |'else'   
  >       |'switch' |'case'   |'default'
  >       |'while'  |'for'    |'do'
  >       |'return' |'break'  |'continue' 
  >       |'print'  |'scan'
  >
  > - 标识符（C0的标识符只支持数字和英文字母，且不能以数字开头。标识符区分大小写。）
  >
  > - 整数字面量：十进制整数字面量和十六进制整数字面量。（整数字面量不包含符号，但是其类型是有符号的`int`，这一点在有其他数据类型的情况下是需要注意的一点，后文会提到。非0的十进制整数字面量不能有任何前导0。）
  >
  > - 浮点数的小数点
  >
  > - 运算符与表达式
  >
  >   ```
  >   <unary-operator>          ::= '+' | '-'
  >   <additive-operator>       ::= '+' | '-'
  >   <multiplicative-operator> ::= '*' | '/'
  >   <relational-operator>     ::= '<' | '<=' | '>' | '>=' | '!=' | '=='
  >   <assignment-operator>     ::= '='
  >   ```
  >
  > - 其他符号
  >
  >   ```
  >   ( ) ; , { }
  >   ```
  >
  > - 注释
  >
  >   ```
  >   <single-line-comment> ::=
  >       '//' {<any-char>} (<LF>|<CR>)
  >   <multi-line-comment> ::= 
  >       '/*' {<any-char>} '*/'
  >   ```
  >
  > 注释的分析**不遵循**最大吞噬规则：
  >
  > - 单行注释内容的分析遇到第一个 0x0A 或 0x0D 字节就立即结束
  > - 多行注释内容的分析遇到第一个`*/`序列就立即结束
  >
  > 
  >
  > - ##### 字符字面量与字符串字面量

- 语法分析处理的内容

  > - 基础类指示符和一个常类型修饰符：这些关键字主要被显式地使用于[变量声明](https://github.com/BUAA-SE-Compiling/c0-handbook/blob/master)、[函数定义](https://github.com/BUAA-SE-Compiling/c0-handbook/blob/master)和[类型转换](https://github.com/BUAA-SE-Compiling/c0-handbook/blob/master)。
  > - 

- 修改词法分析和语法分析的read

- C0 basic grammar

  ```pascal
  <digit> ::= 
      '0'|<nonzero-digit>
  <nonzero-digit> ::= 
      '1'|'2'|'3'|'4'|'5'|'6'|'7'|'8'|'9'
  <hexadecimal-digit> ::=
      <digit>|'a'|'b'|'c'|'d'|'e'|'f'|'A'|'B'|'C'|'D'|'E'|'F'
  
  <integer-literal> ::= 
      <decimal-literal>|<hexadecimal-literal>
  <decimal-literal> ::= 
      '0'|<nonzero-digit>{<digit>}
  <hexadecimal-literal> ::= 
      ('0x'|'0X')<hexadecimal-digit>{<hexadecimal-digit>}
  
  
  <nondigit> ::=    'a'|'b'|'c'|'d'|'e'|'f'|'g'|'h'|'i'|'j'|'k'|'l'|'m'|'n'|'o'|'p'|'q'|'r'|'s'|'t'|'u'|'v'|'w'|'x'|'y'|'z'|'A'|'B'|'C'|'D'|'E'|'F'|'G'|'H'|'I'|'J'|'K'|'L'|'M'|'N'|'O'|'P'|'Q'|'R'|'S'|'T'|'U'|'V'|'W'|'X'|'Y'|'Z'
  
  <identifier> ::= 
      <nondigit>{<nondigit>|<digit>}
  <reserved-word> ::= 
       'const'
      |'void'   |'int'    |'char'   |'double'
      |'struct'
      |'if'     |'else'   
      |'switch' |'case'   |'default'
      |'while'  |'for'    |'do'
      |'return' |'break'  |'continue' 
      |'print'  |'scan'
  
  <unary-operator>          ::= '+' | '-'
  <additive-operator>       ::= '+' | '-'
  <multiplicative-operator> ::= '*' | '/'
  <relational-operator>     ::= '<' | '<=' | '>' | '>=' | '!=' | '=='
  <assignment-operator>     ::= '='   
  
  
      
  <type-specifier>         ::= <simple-type-specifier>
  <simple-type-specifier>  ::= 'void'|'int'
  <const-qualifier>        ::= 'const'
      
      
  <C0-program> ::= 
      {<variable-declaration>}{<function-definition>}
  
  
  <variable-declaration> ::= 
      [<const-qualifier>]<type-specifier><init-declarator-list>';'
  <init-declarator-list> ::= 
      <init-declarator>{','<init-declarator>}
  <init-declarator> ::= 
      <identifier>[<initializer>]
  <initializer> ::= 
      '='<expression>    
  
      
  <function-definition> ::= 
      <type-specifier><identifier><parameter-clause><compound-statement>
  
  <parameter-clause> ::= 
      '(' [<parameter-declaration-list>] ')'
  <parameter-declaration-list> ::= 
      <parameter-declaration>{','<parameter-declaration>}
  <parameter-declaration> ::= 
      [<const-qualifier>]<type-specifier><identifier>
  
      
  <compound-statement> ::= 
      '{' {<variable-declaration>} <statement-seq> '}'
  <statement-seq> ::= 
  	{<statement>}
  <statement> ::= 
       '{' <statement-seq> '}'
      |<condition-statement>
      |<loop-statement>
      |<jump-statement>
      |<print-statement>
      |<scan-statement>
      |<assignment-expression>';'
      |<function-call>';'
      |';'   
      
      
  <condition> ::= 
       <expression>[<relational-operator><expression>] 
     
  <condition-statement> ::= 
       'if' '(' <condition> ')' <statement> ['else' <statement>]
      
  <loop-statement> ::= 
      'while' '(' <condition> ')' <statement>
  
  <jump-statement> ::= 
      <return-statement>
  <return-statement> ::= 
      'return' [<expression>] ';'
      
  <scan-statement> ::= 
      'scan' '(' <identifier> ')' ';'
  <print-statement> ::= 
      'print' '(' [<printable-list>] ')' ';'
  <printable-list>  ::= 
      <printable> {',' <printable>}
  <printable> ::= 
      <expression>
  
  <assignment-expression> ::= 
      <identifier><assignment-operator><expression>
      
  <expression> ::= 
      <additive-expression>
  <additive-expression> ::= 
       <multiplicative-expression>{<additive-operator><multiplicative-expression>}
  <multiplicative-expression> ::= 
       <unary-expression>{<multiplicative-operator><unary-expression>}
  <unary-expression> ::=
      [<unary-operator>]<primary-expression>
  <primary-expression> ::=  
       '('<expression>')' 
      |<identifier>
      |<integer-literal>
      |<function-call>
  
  <function-call> ::= 
      <identifier> '(' [<expression-list>] ')'
  <expression-list> ::= 
      <expression>{','<expression>}
  ```

- c0 grammar

  ```c++
  <digit> ::= 
      '0'|<nonzero-digit>
  <nonzero-digit> ::= 
      '1'|'2'|'3'|'4'|'5'|'6'|'7'|'8'|'9'
  <hexadecimal-digit> ::=
      <digit>|'a'|'b'|'c'|'d'|'e'|'f'|'A'|'B'|'C'|'D'|'E'|'F'
  
  <integer-literal> ::= 
      <decimal-literal>|<hexadecimal-literal>
  <decimal-literal> ::= 
      '0'|<nonzero-digit>{<digit>}
  <hexadecimal-literal> ::= 
      ('0x'|'0X')<hexadecimal-digit>{<hexadecimal-digit>}
  
  
  <nondigit> ::=    'a'|'b'|'c'|'d'|'e'|'f'|'g'|'h'|'i'|'j'|'k'|'l'|'m'|'n'|'o'|'p'|'q'|'r'|'s'|'t'|'u'|'v'|'w'|'x'|'y'|'z'|'A'|'B'|'C'|'D'|'E'|'F'|'G'|'H'|'I'|'J'|'K'|'L'|'M'|'N'|'O'|'P'|'Q'|'R'|'S'|'T'|'U'|'V'|'W'|'X'|'Y'|'Z'
  
  <identifier> ::= 
      <nondigit>{<nondigit>|<digit>}
  <reserved-word> ::= 
       'const'
      |'void'   |'int'    |'char'   |'double'
      |'struct'
      |'if'     |'else'   
      |'switch' |'case'   |'default'
      |'while'  |'for'    |'do'
      |'return' |'break'  |'continue' 
      |'print'  |'scan'
  
  
  <char-liter> ::= 
      "'" (<c-char>|<escape-seq>) "'" 
  <string-literal> ::= 
      '"' {<s-char>|<escape-seq>} '"'
  <escape-seq> ::=  
        '\\' | "\'" | '\"' | '\n' | '\r' | '\t'
      | '\x'<hexadecimal-digit><hexadecimal-digit>
  
      
  <sign> ::= 
      '+'|'-'
  <digit-seq> ::=
      <digit>{<digit>}
  <floating-literal> ::= 
       [<digit-seq>]'.'<digit-seq>[<exponent>]
      |<digit-seq>'.'[<exponent>]
      |<digit-seq><exponent>
  <exponent> ::= 
      ('e'|'E')[<sign>]<digit-seq>
     
      
  <unary-operator>          ::= '+' | '-'
  <additive-operator>       ::= '+' | '-'
  <multiplicative-operator> ::= '*' | '/'
  <relational-operator>     ::= '<' | '<=' | '>' | '>=' | '!=' | '=='
  <assignment-operator>     ::= '='   
  
      
  <single-line-comment> ::=
      '//'{<any-char>}<LF>
  <multi-line-comment> ::= 
      '/*'{<any-char>}'*/'  
      
      
  <type-specifier>         ::= <simple-type-specifier>
  <simple-type-specifier>  ::= 'void'|'int'|'char'|'double'
  <const-qualifier>        ::= 'const'
      
      
  <C0-program> ::= 
      {<variable-declaration>}{<function-definition>}
  
  
  <variable-declaration> ::= 
      [<const-qualifier>]<type-specifier><init-declarator-list>';'
  <init-declarator-list> ::= 
      <init-declarator>{','<init-declarator>}
  <init-declarator> ::= 
      <identifier>[<initializer>]
  <initializer> ::= 
      '='<expression>    
  
      
  <function-definition> ::= 
      <type-specifier><identifier><parameter-clause><compound-statement>
  
  <parameter-clause> ::= 
      '(' [<parameter-declaration-list>] ')'
  <parameter-declaration-list> ::= 
      <parameter-declaration>{','<parameter-declaration>}
  <parameter-declaration> ::= 
      [<const-qualifier>]<type-specifier><identifier>
  
      
  <compound-statement> ::= 
      '{' {<variable-declaration>} <statement-seq> '}'
  <statement-seq> ::= 
  	{<statement>}
  <statement> ::= 
       <compound-statement>
      |<condition-statement>
      |<loop-statement>
      |<jump-statement>
      |<print-statement>
      |<scan-statement>
      |<assignment-expression>';'
      |<function-call>';'
      |';'   
      
      
  <condition> ::= 
       <expression>[<relational-operator><expression>] 
     
  <condition-statement> ::= 
       'if' '(' <condition> ')' <statement> ['else' <statement>]
      |'switch' '(' <expression> ')' '{' {<labeled-statement>} '}'
  
  <labeled-statement> ::= 
       'case' (<integer-literal>|<char-literal>) ':' <statement>
      |'default' ':' <statement>
  
      
  <loop-statement> ::= 
      'while' '(' <condition> ')' <statement>
     |'do' <statement> 'while' '(' <condition> ')' ';'
     |'for' '('<for-init-statement> [<condition>]';' [<for-update-expression>]')' <statement>
  
  <for-init-statement> ::= 
      [<assignment-expression>{','<assignment-expression>}]';'
  <for-update-expression> ::=
      (<assignment-expression>|<function-call>){','(<assignment-expression>|<function-call>)}
  
  
  <jump-statement> ::= 
       'break' ';'
      |'continue' ';'
      |<return-statement>
  <return-statement> ::= 'return' [<expression>] ';'
      
      
  <scan-statement> ::= 
      'scan' '(' <identifier> ')' ';'
  <print-statement> ::= 
      'print' '(' [<printable-list>] ')' ';'
  <printable-list>  ::= 
      <printable> {',' <printable>}
  <printable> ::= 
      <expression> | <string-literal>
  
  <assignment-expression> ::= 
      <identifier><assignment-operator><expression>
      
     
    
  <expression> ::= 
      <additive-expression>
  <additive-expression> ::= 
       <multiplicative-expression>{<additive-operator><multiplicative-expression>}
  <multiplicative-expression> ::= 
       <cast-expression>{<multiplicative-operator><cast-expression>}
  <cast-expression> ::=
      {'('<type-specifier>')'}<unary-expression>
  <unary-expression> ::=
      [<unary-operator>]<primary-expression>
  <primary-expression> ::=  
       '('<expression>')' 
      |<identifier>
      |<integer-literal>
      |<char-literal>
      |<floating-literal>
      |<function-call>
  
  <function-call> ::= 
      <identifier> '(' [<expression-list>] ')'
  <expression-list> ::= 
      <expression>{','<expression>}
  ```
  


