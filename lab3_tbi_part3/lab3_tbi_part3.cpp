#include <iostream>
#include <windows.h>
#include <sstream>
#include <stdexcept>
#pragma execution_character_set("utf-8")


int currentSymbolIndex = 0;
char currentSymbol = ' ';
std::string inputString = "";
bool doesExpressionContainCondition = true;
bool isBracketAllowed = false;

struct SyntaxElementsTree {
	char symbol;
	SyntaxElementsTree* left;
	SyntaxElementsTree* right;
};

static void CheckIfCurrentSymbolIsAcceptable() {
	if (!strchr("+-/*=<>&|", currentSymbol) && currentSymbol != '_' && !isBracketAllowed) {
		throw std::invalid_argument(std::string("Ошибка при написании выражения: лишний символ ")
			+ currentSymbol + std::string(" на позиции ")
			+ std::to_string(currentSymbolIndex + 1)
			+ std::string("."));
	}
}

static void GetResultExpression(SyntaxElementsTree* currentNode);

static void GoToNextSymbol() {
	if (currentSymbolIndex < inputString.length() - 1) {
		currentSymbolIndex++;
		currentSymbol = inputString[currentSymbolIndex];
	}
	else {
		currentSymbolIndex = inputString.length();
		currentSymbol = '_';
	}
}

/// <summary>
/// определение, является ли символ латинской буквой
/// </summary>
/// <returns>истинность утверждения, что символ является латинской буквой</returns>
static bool TellIfLetter() {
	bool isCurrentSymbolLetter = int(currentSymbol) > 64 && int(currentSymbol) < 91 || int(currentSymbol) > 96 && int(currentSymbol) < 123;
	return isCurrentSymbolLetter;
}

/// <summary>
/// правило для считывания выражения String, представляющего собой переменную, состоящую из букв
/// </summary>
static void GetString() {
	do {
		GoToNextSymbol();
	} while (TellIfLetter());
}

/// <summary>
/// определение, является ли символ цифрой
/// </summary>
/// <returns>истинность утверждения, что символ является цифрой</returns>
static bool TellIfDigit() {
	bool isCurrentSymbolDigit = strchr("0123456789", currentSymbol);
	return isCurrentSymbolDigit;
}

/// <summary>
/// правило для считывания выражения Number, представляющего собой число, состоящее из цифр
/// </summary>
static void GetNumber() {
	do {
		GoToNextSymbol();
	} while (TellIfDigit());
	if (currentSymbol == ',') {
		int countDigitsAfterComma = 0;
		do {
			GoToNextSymbol();
			if (TellIfDigit()) {
				countDigitsAfterComma++;
			}
		} while (TellIfDigit());
		if (countDigitsAfterComma < 1) {
			throw std::invalid_argument(std::string("Ошибка при написании числа: в дробной части вещественного числа (части после запятой, на месте символа ")
				+ currentSymbol + std::string(" на позиции ")
				+ std::to_string(inputString.find(currentSymbol) + 1)
				+ std::string(") отсутствуют разряды."));
		}
	}
}

/// <summary>
/// правило для считывания выражения Factor, являющегося либо числом, либо строкой, либо выражением в скобках
/// </summary>
static void GetFactor(SyntaxElementsTree* currentNode) {
	if (TellIfLetter()) {
		GetString();
	}
	else if (currentSymbol == '-') {
		GoToNextSymbol();
		if (TellIfDigit()) {
			GetNumber();
		}
		else {
			throw std::invalid_argument(std::string("Ошибка при написании числа: после минуса в отрицательном на месте символа ")
				+ currentSymbol + std::string(" на позиции ")
				+ std::to_string(inputString.find(currentSymbol) + 1)
				+ std::string(") отсутствуют разряды."));
		}
	}
	else if (TellIfDigit()) {
		GetNumber();
	}
	else if (currentSymbol == '(') {
		isBracketAllowed = true;
		GoToNextSymbol();
		GetResultExpression();
		if (currentSymbol == ')') {
			GoToNextSymbol();
		}
		else {
			if (currentSymbol == '_') {
				throw std::invalid_argument(std::string("Ошибка при написании выражения: скобка была открыта, но не была закрыта на месте пустого символа после ")
					+ inputString[currentSymbolIndex - 1] + std::string(" на позиции ")
					+ std::to_string(inputString.length())
					+ std::string("."));
			}
			throw std::invalid_argument(std::string("Ошибка при написании выражения: скобка была открыта, но не была закрыта на месте символа ")
				+ currentSymbol + std::string(" на позиции ")
				+ std::to_string(currentSymbolIndex + 1)
				+ std::string("."));
		}
	}
	else {
		if (currentSymbol == '_') {
			throw std::invalid_argument(std::string("Ошибка при написании выражения: пустой символ после знака ")
				+ inputString[currentSymbolIndex - 1] + std::string(" на позиции ")
				+ std::to_string(inputString.length())
				+ std::string(" не может являться членом выражения."));
		}
		else {
			throw std::invalid_argument(std::string("Ошибка при написании выражения: символ ")
				+ currentSymbol + std::string(" на позиции ")
				+ std::to_string(currentSymbolIndex + 1)
				+ std::string(" не может являться членом выражения."));
		}
	}
}

/// <summary>
/// правило для считывания выражения AddedExpression, состоящего из выражений, произведение значений которых необходимо найти
/// </summary>
static void GetAddedExpression(SyntaxElementsTree* currentNode) {
	currentNode = null;
	SyntaxElementsTree* leftNode;
	SyntaxElementsTree* rightNode;

	GetFactor(leftNode);
	CheckIfCurrentSymbolIsAcceptable();
	while (currentSymbol == '*'
		|| currentSymbol == '/') {
		CreateNode(currentNode, currentSymbol);
		&currentNode.left = leftNode;
		GoToNextSymbol();
		GetFactor(rightNode);
		&currentNode.right = rightNode;
		leftNode = currentNode;
	}
}

/// <summary>
/// правило для считывания выражения ComparedCondition, состоящего из выражений, складываемых друг с другом
/// </summary>
static void GetComparedCondition(SyntaxElementsTree* currentNode) {
	currentNode = null;
	SyntaxElementsTree* leftNode;
	SyntaxElementsTree* rightNode;
	GetAddedExpression(leftNode);
	CheckIfCurrentSymbolIsAcceptable();
	while (currentSymbol == '+'
		|| currentSymbol == '-') {
		CreateNode(currentNode, currentSymbol);
		&currentNode.left = leftNode;
		GoToNextSymbol();
		GetAddedExpression(rightNode);
		leftNode = currentNode;
	}
}

/// <summary>
/// правило для считывания выражения-условия ContrCondition, состоящего из выражений, сравниваемых друг с другом
/// </summary>
static void GetContrCondition(SyntaxElementsTree* currentNode) {
	currentNode = null;
	SyntaxElementsTree* leftNode;
	SyntaxElementsTree* rightNode;
	GetComparedCondition(leftNode);
	CheckIfCurrentSymbolIsAcceptable();
	while (currentSymbol == '='
		|| currentSymbol == '<'
		|| currentSymbol == '>') {
		doesExpressionContainCondition = true;
		CreateNode(currentNode, currentSymbol);
		GoToNextSymbol();
		GetComparedCondition(rightNode);
		leftNode = currentNode;
	}
}

/// <summary>
/// правило для считывания выражения-условия AndCondition, состоящего из условия с союзом НЕ
/// </summary>
static void GetAndCondition(SyntaxElementsTree* currentNode) {
	if (currentSymbol == '!') {
		doesExpressionContainCondition = false;
		GoToNextSymbol();
		GetContrCondition();
		if (!doesExpressionContainCondition) {
			throw std::invalid_argument(std::string("Ошибка при написании выражения: логические операторы '&', '|' и '!' могут применяться только к выражениям, в которых есть сравнение"));
		}
		CheckIfCurrentSymbolIsAcceptable();
	}
	else {
		GetContrCondition();
		CheckIfCurrentSymbolIsAcceptable();
	}
}

/// <summary>
/// правило для считывания выражения-условия OrCondition, состоящего из условий с союзом И
/// </summary>
static void GetOrCondition(SyntaxElementsTree* currentNode) {
	currentNode = null;
	SyntaxElementsTree* leftNode;
	SyntaxElementsTree* rightNode;
	GetAndCondition(leftNode);
	CheckIfCurrentSymbolIsAcceptable();
	while (currentSymbol == '&') {
		doesExpressionContainCondition = false;
		CreateNode(currentNode, currentSymbol);
		GoToNextSymbol();
		GetAndCondition(rightNode);
		leftNode = currentNode;
		if (!doesExpressionContainCondition) {
			throw std::invalid_argument(std::string("Ошибка при написании выражения: логические операторы '&', '|' и '!' могут применяться только к выражениям, в которых есть сравнение"));
		}
	}
}

/// <summary>
/// правило для считывания целого выражения ResultExpression, состоящего из условий с союзом ИЛИ
/// </summary>
static void GetResultExpression(SyntaxElementsTree* currentNode) {
	currentNode = null;
	SyntaxElementsTree* leftNode;
	SyntaxElementsTree* rightNode;
	GetOrCondition(leftNode);
	CheckIfCurrentSymbolIsAcceptable();
	while (currentSymbol == '|') {
		doesExpressionContainCondition = false;
		CreateNode(currentNode, currentSymbol);
		GoToNextSymbol();
		GetOrCondition(rightNode);
		leftNode = currentNode;
		if (!doesExpressionContainCondition) {
			throw std::invalid_argument(std::string("Ошибка при написании выражения: логические операторы '&', '|' и '!' могут применяться только к выражениям, в которых есть сравнение"));
		}
	}

}

int main()
{
	SetConsoleOutputCP(65001);
	SetConsoleCP(65001);
	SyntaxElementsTree currentTree;
	try {
		std::cout << "Введите строку с арифметическим выражением. \nНе используйте пробелы и табуляцию, для дробной части вещественных чисел используйте символ запятой." << std::endl;
		std::cin >> inputString;
		currentSymbol = inputString[0];
		//std::cout << inputString << std::endl;
		GetResultExpression(currentTree);
	}
	catch (std::invalid_argument& exception) {
		std::cout << exception.what() << std::endl;
	}
	std::cout << "Синтаксический разбор завершен.\n";
}
