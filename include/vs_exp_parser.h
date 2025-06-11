#ifndef VS_EXP_PARSER
#define VS_EXP_PARSER

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_exp_parser.h
*   Date: 6/9/2025
*   Version: 1.1
*   Updated: 6/11/2025
*   Author: Ryandracus Chapman
*
********************************************/

#include <vs_utils.h>

#define VS_STACK_SIZE 256

void VS_InitExprParser();
void VS_PushItemToExprStack(char item);
char VS_PopItemFromExprStack();
void VS_PushItemToExprStack_long(long item);
long VS_PopItemFromExprStack_long();
int VS_OperatorPrecedence(char symbol);
int VS_IsValidOperator(char symbol);
int VS_IsValidMathOperator(char symbol);
int VS_IsValidOperatorPair(char prev_op, char cur_op, char next_op);
int VS_IsValidExpression(char* expr, VS_SYNTAX syntax);
int VS_LineContainsOperator(char* line);
void VS_ConvertExpr(char infix[], char postfix[]);
int VS_IsBalancedParenthesis(char* expr);
long VS_EvaluateExpr(char *input, VS_SYNTAX syntax);

#endif