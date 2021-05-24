//
// Created by Sandu Kiritsa on 21/10/2020.
//

#include <algorithm>
#include <iostream>
#include <locale>
#include <queue>
#include <stack>
#include <string>

typedef struct {
    const char* name;
    unsigned int pointer;
} symbol_t;

using std::cout;
using std::endl;

int OperatorPriority(std::string& ch)
{
    if (ch == "+" || ch == "-") {
        return 1; //Precedence of + or - is 1
    } else if (ch == "*" || ch == "/") {
        return 2; //Precedence of * or / is 2
    } else if (ch == "^") {
        return 3; //Precedence of ^ is 3
    } else {
        return 0;
    }
}

bool IsNumber(const std::string& s)
{
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

enum class TokenType {
    OPERATOR,
    FUNCTION_NAME,
    FUNCTION,
    PARENTHESES,
    CONSTANT,
    VARIABLE,
    COMMA,
    EMPTY,
};

struct Token {
    TokenType token_type;
    std::string token;
    std::vector<Token> function;
};

std::vector<Token> Infix2Postfix(std::vector<Token>& infix)
{
    std::vector<Token> postfix;
    std::stack<Token> stack;

    for (auto it : infix) {
        if (it.token_type == TokenType::VARIABLE ||
            it.token_type == TokenType::CONSTANT ||
            it.token_type == TokenType::FUNCTION) {
            postfix.emplace_back(it);
            continue;
        }

        if (it.token == "(") {
            stack.push(it);
            continue;
        }

        if (it.token == ")") {
            while (!stack.empty() && stack.top().token != "(") {
                postfix.emplace_back(stack.top());
                stack.pop();
            }

            stack.pop();
            continue;
        }

        if (it.token_type == TokenType::OPERATOR) {
            if (stack.empty() || OperatorPriority(it.token) >
                                     OperatorPriority(stack.top().token)) {
                stack.push(it);
            } else {
                while (!stack.empty() &&
                       OperatorPriority(it.token) <=
                           OperatorPriority(stack.top().token)) {
                    postfix.emplace_back(stack.top());
                    stack.pop();
                }
                stack.push(it);
            }
        }
    }

    while (!stack.empty()) {
        Token top = stack.top();
        if (top.token != "(")
            postfix.emplace_back(top);

        stack.pop();
    }

    return postfix;
}

template <typename T> void ConvertFunctions(T begin, T end);

template <typename T> void Display(T begin, T end)
{
    T it = begin;
    while (it != end) {
        cout << it->token << " ";
        ++it;
    }
    cout << endl;
}

template <typename T> void Display(T x)
{
    for (auto it : x) {
        cout << it.token << " ";
        if (it.token_type == TokenType::FUNCTION) {
            cout << " :";
            for (auto it1 : it.function) {
                cout << it1.token << " ";
            }
            cout << ": ";
        }
    }

    cout << endl;
}

template <typename T> void ConvertNestedFunctions(T begin, T end)
{
    int depth = 0;
    std::vector<Token> result;
    std::vector<Token> buff;
    for (auto it = begin; it != end; ++it) {
        if (it->token == "[")
            ++depth;
        if (it->token_type == TokenType::FUNCTION_NAME)
            --depth;

        if (depth >= 1) {
            buff.emplace_back(*it);
        }

        if (it->token_type == TokenType::FUNCTION_NAME && depth == 0) {
            buff.emplace_back(*it);
            result.emplace_back(Token{TokenType::FUNCTION, "", buff});
            buff = {};
        } else if (depth == 0) {
            result.emplace_back(*it);
        }
    }

    auto processed = Infix2Postfix(result);
    std::vector<Token> result2;

    for (auto it : processed) {
        if (it.token_type == TokenType::FUNCTION) {
            ConvertFunctions(it.function.begin(), it.function.end());
            for (const auto& it2 : it.function) {
                result2.emplace_back(it2);
            }
        } else {
            result2.emplace_back(it);
        }
    }

    size_t converted_size = result2.size();
    while (converted_size != end - begin) {
        result2.emplace_back(Token{TokenType::EMPTY, "", {}});
        ++converted_size;
    }

    std::copy(result2.begin(), result2.end(), begin);
}

template <typename T> void ConvertFunctions(T begin, T end)
{
    T it = begin;
    bool is_simple = true;
    bool is_expr = true;
    int depth = 0;

    int max_depth = 0;
    std::vector<Token> bits;
    std::vector<Token> f;

    for (auto it = begin; it != end; ++it) {
        if (it->token == "[") {
            is_expr = false;
            ++depth;
            max_depth = (depth > max_depth) ? depth : max_depth;
        }

        if (depth == 0 && it->token_type == TokenType::OPERATOR) {
            is_simple = false;
        }

        if (it->token_type == TokenType::FUNCTION_NAME) {
            --depth;
            if (depth == 0) {
                f.emplace_back(*it);
                Token new_el{TokenType::FUNCTION, "", f};
                bits.emplace_back(new_el);
                f = {};
                continue;
            }
        }

        if (depth > 0) {
            f.emplace_back(*it);
        }

        if (depth == 0) {
            bits.emplace_back(*it);
        }
    }

    if (!is_expr && !is_simple) {
        ConvertNestedFunctions(begin, end);
    }

    if (is_expr) {
        std::vector<Token> d(begin, end);

        auto converted = Infix2Postfix(d);
        size_t converted_size = converted.size();
        while (converted_size != end - begin) {
            converted.emplace_back(Token{TokenType::EMPTY, "", {}});
            ++converted_size;
        }

        std::copy(converted.begin(), converted.end(), begin);

        return;
    }

    if (is_simple) {
        int depth = 0;
        T arg_start = begin;
        T it = begin;

        while (it != end) {

            if (it->token == "[")
                ++depth;
            if (it->token_type == TokenType::FUNCTION_NAME)
                --depth;

            if (depth == 1 && it->token == "[")
                arg_start = it + 1;

            if (depth == 1 && it->token == ",") {
                ConvertFunctions(arg_start, it);
                arg_start = it + 1;
            }

            if (depth == 1 && it->token == "]") {
                ConvertFunctions(arg_start, it);
            }

            ++it;
        }
    }
}

std::vector<Token> PrepInfixString(std::string& input)
{
    std::string result;
    std::string input1;
    std::string buffer;
    std::stack<char> brackets;

    // spaces
    for (auto it : input) {
        if (it == ' ')
            continue;
        input1.push_back(it);
    }

    // unary minus
    std::string input2;
    auto it = input1.begin();
    while (it != input1.end()) {
        size_t i = it - input1.begin();
        if (i == 0 && *it == '-') {
            input2 += "0-";
            ++it;
            continue;
        }
        if (i >= 1 && *it == '-' && *(it - 1) == '(') {
            input2 += "0-";
            ++it;
            continue;
        }
        if (i >= 1 && *it == '-' && *(it - 1) == ',') {
            input2 += "0-";
            ++it;
            continue;
        }
        input2 += *it;
        ++it;
    }

    for (auto it : input2) {
        if (!IsNumber(std::string{it})) {
            buffer.push_back(it);

            if (buffer.size() > 2) {
                if (*(buffer.end() - 1) == '(' &&
                    isalpha(*(buffer.end() - 2))) {
                    result.push_back('[');
                    continue;
                }
            }
        }

        result.push_back(it);
    }

    for (char& it1 : result) {
        if (it1 == '[' || it1 == '(') {
            brackets.push(it1);
            continue;
        }

        if (it1 == ')') {
            if (brackets.top() == '[') {
                it1 = ']';
            }
            brackets.pop();
        }
    }

    std::vector<Token> tokens;
    std::stack<Token> stack;
    std::string buff;
    for (auto it : result) {

        if (IsNumber(std::string{it}) || isalpha(it)) {
            buff += it;
        }

        if (it == '[') {
            if (!buff.empty()) {
                tokens.emplace_back(Token{TokenType::FUNCTION_NAME, buff});
                buff = "";
            }
            tokens.emplace_back(Token{TokenType::PARENTHESES, std::string{it}});
        }

        if (it == '+' || it == '*' || it == '-' || it == ')' || it == '(' ||
            it == ']' || it == ',') {
            if (!buff.empty()) {
                TokenType tt =
                    IsNumber(buff) ? TokenType::CONSTANT : TokenType::VARIABLE;
                tokens.emplace_back(Token{tt, buff});
                buff = "";
            }
        }

        if (it == '+' || it == '-' || it == '*')
            tokens.emplace_back(Token{TokenType::OPERATOR, std::string{it}});

        if (it == ',')
            tokens.emplace_back(Token{TokenType::COMMA, std::string{it}});

        if (it == '(' || it == ']' || it == ')')
            tokens.emplace_back(Token{TokenType::PARENTHESES, std::string{it}});
    }

    if (!buff.empty()) {
        TokenType tt =
            IsNumber(buff) ? TokenType::CONSTANT : TokenType::VARIABLE;
        tokens.emplace_back(Token{tt, buff});
        buff = "";
    }

    std::vector<Token> postfix_funcs;
    std::stack<Token> funcs;

    for (const auto& token : tokens) {
        if (token.token_type == TokenType::FUNCTION_NAME) {
            funcs.push(token);
            continue;
        }

        if (token.token == "]") {
            postfix_funcs.emplace_back(token);
            postfix_funcs.emplace_back(funcs.top());
            funcs.pop();
            continue;
        }

        postfix_funcs.push_back(token);
    }

    ConvertFunctions(postfix_funcs.begin(), postfix_funcs.end());

    return postfix_funcs;
}

ssize_t FindArgCount(std::vector<Token>& input, size_t index)
{
    auto it = input.begin() + index;
    ssize_t depth = 0;

    if ((it - 1)->token == "]" && (it - 2)->token == "[")
        return 0;
    size_t count = 1;

    while (it != input.begin()) {
        if (it->token == "]")
            ++depth;
        if (it->token == "[")
            --depth;
        if (it->token == "," && depth == 1)
            ++count;
        if (it->token == "[" && depth == 0)
            break;
        --it;
    }
    return count;
}

class MachineInstructionsConverter
{
  public:
    enum {
        PUSH = 0xe52d0004,
        POP = 0xe8bd0000,
        MOV = 0xe3a00000,
        ADD = 0xe0800000,
        SUB = 0xe0400000,
        MUL = 0xe0000090,
        LDR = 0xe5900000,
        BXL = 0xe1200030,
        BXLR = 0xe12fff1e,
    };

    enum { r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, sp, lr, pc };

    const symbol_t* externs;
    std::vector<Token> inf;
    uint32_t* output_buffer;
    uint32_t* data_buffer;
    MachineInstructionsConverter(
        const symbol_t* externs,
        uint32_t* output_buffer,
        std::vector<Token>& inf)
        : externs(externs), output_buffer(output_buffer), inf(inf)
    {
        data_buffer = output_buffer + 4096 / sizeof(uint32_t) - 1;
    };

    void Execute()
    {
        push(lr);
        for (auto it = inf.begin(); it != inf.end(); ++it) {
            switch (it->token_type) {

            case (TokenType::CONSTANT):
                HandleConstant(*it);
                break;

            case (TokenType::VARIABLE):
                HandleVariable(*it);
                break;

            case (TokenType::OPERATOR):
                PerformBinaryOperation(*it);
                break;
            case (TokenType::FUNCTION_NAME):
                ExecuteFunction(*it, it - inf.begin());
                break;

            default:
                break;
            }
        }

        pop({r0});
        pop({lr});
        bxlr();
    }

    void ExecuteFunction(Token t, size_t index)
    {
        ssize_t arg_count = FindArgCount(inf, index);
        std::vector<int> regs = {r1, r0};
        size_t func_addr_index = FindExternAddressIndex(t.token);
        for (int i = arg_count - 1; i >= 0; --i)
            pop({i});

        push(r4);

        *data_buffer = (uint32_t)(externs[func_addr_index].pointer);
        uint32_t* addr = data_buffer;
        --data_buffer;
        int shift = sizeof(uint32_t) * (addr - output_buffer - 2);

        ldr(r4, pc, shift);
        bxl(r4);

        pop({r4});

        push(r0);
    }

    void PerformBinaryOperation(Token x)
    {
        pop({r0});
        pop({r1});
        if (x.token == "+")
            add(r0, r0, r1);
        if (x.token == "-")
            sub(r0, r1, r0);
        if (x.token == "*")
            mul(r0, r0, r1);

        push(r0);
    }

    void HandleConstant(Token& x)
    {
        *data_buffer = (uint32_t)(stoi(x.token));
        uint32_t* addr = data_buffer;

        --data_buffer;

        int shift = sizeof(uint32_t) * (addr - output_buffer - 2);

        ldr(r0, pc, shift);
        push(r0);
    }

    size_t FindExternAddressIndex(std::string& x) const
    {
        size_t counter = 0;
        while (externs[counter].name != x) {
            ++counter;
            if (externs[counter].pointer == NULL)
                break;
        }
        return counter;
    }

    void HandleVariable(Token x)
    {
        size_t i = FindExternAddressIndex(x.token);
        *data_buffer = (uint32_t)(externs[i].pointer);
        uint32_t* addr = data_buffer;

        --data_buffer;

        int shift = sizeof(uint32_t) * (addr - output_buffer - 2);

        ldr(r0, pc, shift);
        ldr(r0, r0, 0);
        push(r0);
    }

    void write_to_buff(uint32_t val)
    {
        *output_buffer = val;
        ++output_buffer;
    }

    void pop(std::vector<int> registers)
    {
        uint32_t value = POP;
        for (auto it : registers) {
            value += (1ULL << it);
        }
        write_to_buff(value);
    }

    void mov(int reg_index, int op)
    {
        uint32_t value = MOV;
        value += op;
        value += (reg_index << 12ULL);
        write_to_buff(value);
    }

    void push(uint32_t reg_index)
    {
        uint32_t value = PUSH;
        value += (reg_index << 12ULL);
        write_to_buff(value);
    }

    void add(int x, int y, int dest)
    {
        uint32_t value = ADD;
        value += (x << 16ULL) + (y << 12ULL) + dest;
        write_to_buff(value);
    }

    void sub(int x, int y, int dest)
    {
        uint32_t value = SUB;
        value += (y << 16ULL) + (x << 12ULL) + dest;
        write_to_buff(value);
    }

    void mul(int x, int y, int dest)
    {
        uint32_t value = MUL;
        value += (x << 16ULL) + (dest << 8ULL) + y;
        write_to_buff(value);
    }

    void ldr(int dest, int src, int shift)
    {
        uint32_t value = LDR;
        value += (src << 16ULL) + (dest << 12ULL) + shift;
        write_to_buff(value);
    }
    void bxl(int x)
    {
        uint32_t value = BXL;
        value += x;
        write_to_buff(value);
    }
    void bxlr()
    {
        uint32_t value = BXLR;
        write_to_buff(value);
    }
};

extern "C" void jit_compile_expression_to_arm(
    const char* expression,
    const symbol_t* externs,
    void* out_buffer)
{
    std::string expr = static_cast<std::string>(expression);
    auto* output_buffer = static_cast<uint32_t*>(out_buffer);
    uint32_t* vars = output_buffer + 4096 / sizeof(uint32_t) - 1;
    auto converted = PrepInfixString(expr);

    auto compiler =
        MachineInstructionsConverter(externs, output_buffer, converted);
    compiler.Execute();
}