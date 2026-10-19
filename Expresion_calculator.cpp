
#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <sstream>
#include <cmath>
#include <cctype>
#include <stdexcept>
#include <iomanip>
#include <limits>
using namespace std;

bool isOperator(const string& s) {
    return s == "+" || s == "-" || s == "*" ||
           s == "/" || s == "%" || s == "^" ||
           s == "u-" || s == "u+";
}

int precedence(const string& op) {
    if (op == "u-" || op == "u+") return 4;
    if (op == "^") return 3;
    if (op == "*" || op == "/" || op == "%") return 2;
    if (op == "+" || op == "-") return 1;
    return 0;
}

bool isRightAssociative(const string& op) {
    return op == "^" || op == "u-" || op == "u+";
}

// Convert infix expression into postfix expression
vector<string> infixToPostfix(const string& expression) {
    vector<string> output;
    stack<string> operators;
    bool expectOperand = true;
    bool sawToken = false;

    for (size_t i = 0; i < expression.size();) {
        char c = expression[i];

        if (isspace(static_cast<unsigned char>(c))) {
            i++;
            continue;
        }

        // Read numbers, including decimal numbers
        if (isdigit(static_cast<unsigned char>(c)) || c == '.') {
            if (!expectOperand)
                throw runtime_error("Missing operator between values.");

            size_t start = i;
            int dots = 0;

            while (i < expression.size() &&
                   (isdigit(static_cast<unsigned char>(expression[i])) ||
                    expression[i] == '.')) {
                if (expression[i] == '.') dots++;
                if (dots > 1)
                    throw runtime_error("Invalid number.");

                i++;
            }

            string number = expression.substr(start, i - start);

            if (number == ".")
                throw runtime_error("Invalid number.");

            output.push_back(number);
            expectOperand = false;
            sawToken = true;
            continue;
        }

        if (c == '(') {
            if (!expectOperand)
                throw runtime_error("Missing operator before '('.");

            operators.push("(");
            expectOperand = true;
            sawToken = true;
            i++;
            continue;
        }

        if (c == ')') {
            if (expectOperand)
                throw runtime_error("Empty or incomplete parentheses.");

            while (!operators.empty() && operators.top() != "(") {
                output.push_back(operators.top());
                operators.pop();
            }

            if (operators.empty())
                throw runtime_error("Mismatched parentheses.");

            operators.pop(); // Remove '('
            expectOperand = false;
            i++;
            continue;
        }

        if (string("+-*/%^").find(c) != string::npos) {
            string op(1, c);

            // Handle unary plus and minus, e.g. -5 or -(2+3)
            if (expectOperand) {
                if (c != '+' && c != '-')
                    throw runtime_error("Operator in invalid position.");

                op = (c == '-') ? "u-" : "u+";
                operators.push(op);
                i++;
                sawToken = true;
                continue;
            }

            while (!operators.empty() &&
                   operators.top() != "(" &&
                   (precedence(operators.top()) > precedence(op) ||
                   (precedence(operators.top()) == precedence(op) &&
                    !isRightAssociative(op)))) {
                output.push_back(operators.top());
                operators.pop();
            }

            operators.push(op);
            expectOperand = true;
            i++;
            sawToken = true;
            continue;
        }

        throw runtime_error("Invalid character in expression.");
    }

    if (!sawToken)
        throw runtime_error("Expression is empty.");

    if (expectOperand)
        throw runtime_error("Expression ends with an operator.");

    while (!operators.empty()) {
        if (operators.top() == "(")
            throw runtime_error("Mismatched parentheses.");

        output.push_back(operators.top());
        operators.pop();
    }

    return output;
}

// Evaluate postfix expression using a stack
double evaluatePostfix(const vector<string>& postfix) {
    stack<double> values;

    for (const string& token : postfix) {
        if (!isOperator(token)) {
            size_t used = 0;
            double value = stod(token, &used);

            if (used != token.size() || !isfinite(value))
                throw runtime_error("Invalid numeric value.");

            values.push(value);
            continue;
        }

        // Unary operators require one operand
        if (token == "u-" || token == "u+") {
            if (values.empty())
                throw runtime_error("Invalid unary expression.");

            double a = values.top();
            values.pop();

            values.push(token == "u-" ? -a : a);
            continue;
        }

        if (values.size() < 2)
            throw runtime_error("Invalid expression.");

        double b = values.top();
        values.pop();

        double a = values.top();
        values.pop();

        double result = 0;

        if (token == "+") result = a + b;
        else if (token == "-") result = a - b;
        else if (token == "*") result = a * b;
        else if (token == "/") {
            if (b == 0)
                throw runtime_error("Division by zero.");
            result = a / b;
        }
        else if (token == "%") {
            if (b == 0)
                throw runtime_error("Modulo by zero.");
            result = fmod(a, b);
        }
        else if (token == "^") {
            result = pow(a, b);
        }

        if (!isfinite(result))
            throw runtime_error("Result is outside the supported range.");

        values.push(result);
    }

    if (values.size() != 1)
        throw runtime_error("Invalid expression.");

    return values.top();
}

int main() {
    int choice;

    do {
        cout << "\n====================================\n";
        cout << "       EXPRESSION CALCULATOR\n";
        cout << "====================================\n";
        cout << "1. Evaluate Expression\n";
        cout << "2. Show Postfix Expression\n";
        cout << "0. Exit\n";
        cout << "Enter your choice: ";

        if (!(cin >> choice)) {
            cout << "Invalid menu input!\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 0) {
            cout << "Calculator closed.\n";
        }
        else if (choice == 1 || choice == 2) {
            string expression;

            cout << "Enter infix expression: ";
            getline(cin, expression);

            try {
                vector<string> postfix = infixToPostfix(expression);

                if (choice == 2) {
                    cout << "Postfix: ";
                    for (const string& token : postfix)
                        cout << token << " ";
                    cout << '\n';
                } else {
                    double answer = evaluatePostfix(postfix);
                    cout << fixed << setprecision(6);
                    cout << "Result: " << answer << '\n';
                }
            }
            catch (const exception& e) {
                cout << "Error: " << e.what() << '\n';
            }
        }
        else {
            cout << "Invalid choice! Try again.\n";
        }

    } while (choice != 0);

    return 0;
}
