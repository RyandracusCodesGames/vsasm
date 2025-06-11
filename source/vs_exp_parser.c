/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_exp_parser.c
*   Date: 6/9/2025
*   Version: 1.1
*   Updated: 6/10/2025
*   Author: Ryandracus Chapman
*
********************************************/
#include <vs_exp_parser.h>
#include <stdio.h> 
#include <string.h> 
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

char stack[VS_STACK_SIZE]; 
int top = -1; 

long stack_int[VS_STACK_SIZE]; 
int top_int = -1; 

void VS_InitExprParser(){
	top = -1;
	top_int = -1;
}

void VS_PushItemToExprStack(char item){ 
   stack[++top] = item; 
}
char VS_PopItemFromExprStack(){ 
   return stack[top--]; 
}

void VS_PushItemToExprStack_long(long item){ 
   stack_int[++top_int] = item; 
} 

long VS_PopItemFromExprStack_long(){ 
   return stack_int[top_int--]; 
}

int VS_OperatorPrecedence(char symbol){ 
   switch(symbol){
	  case '|':
		return 2;
	  case '&':
		return 3;
      case '<': /* << */
      case '>': /* >> */
         return 4;
      case '+': 
      case '-':
         return 5; 
      case '*': 
      case '/':
         return 6; 
      case '^': 
         return 7; 
      case '(': 
      case ')': 
      case '#':
         return 1; 
      default:
         return 0;
   }
}

int VS_IsValidOperator(char symbol){ 
   switch(symbol){ 
      case '+': case '-': case '*': case '/': 
      case '^': case '(': case ')': 
      case '<': case '>': case '&': case '|':
         return 1; 
      default:
         return 0; 
   } 
}

int VS_IsValidMathOperator(char symbol){ 
   switch(symbol){ 
      case '+': case '-': case '*': case '/': 
      case '^': case '&': case '|':
         return 1; 
      default:
         return 0; 
   } 
}

int VS_IsBalancedParenthesis(char* expr){
	int i, balance = 0, len = strlen(expr);
    for(i = 0; i < len; i++){
        if(expr[i] == '('){
			balance++;
		}
		
        if(expr[i] == ')'){
            if(--balance < 0) return 0;
        }
    }
	
    return balance == 0;
}

int VS_IsValidOperatorPair(char prev_op, char cur_op, char next_op){
	if(prev_op == '(' && cur_op == ')'){
		return 0;
	}
	else if(prev_op == ')' && cur_op == '('){
		return 0;
	}
	else if(prev_op == '<' && cur_op != '<'){
		return 0;
	}
	else if(prev_op == '>' && cur_op != '>'){
		return 0;
	}
	else if(VS_IsValidMathOperator(prev_op) && VS_IsValidMathOperator(cur_op)){
		return 0;
	}
	else if(VS_IsValidMathOperator(prev_op) && cur_op == ')'){
		return 0;
	}
	else if(prev_op == '(' && VS_IsValidMathOperator(cur_op)){
		return 0;
	}
	else if(VS_IsValidMathOperator(prev_op) && cur_op == '(' && VS_IsValidMathOperator(next_op)){
		return 0;
	}
	else return 1;
}

int VS_IsValidExpression(char* expr,  VS_SYNTAX syntax){
	if(!VS_IsBalancedParenthesis(expr)){
		return 0;
	}
	
	int i, len = strlen(expr), prev_op = 0, cur_op, num_is_hex = 0;
    for(i = 0; i < len-1; i++) {
		cur_op = expr[i];

		if(expr[i] == '0' && expr[i+1] == 'x' && !num_is_hex){
			num_is_hex = 1;
			i++;
			continue;
		}
	
		if(syntax == VS_ASMPSX_SYNTAX){
			if(expr[i] == '$' && !num_is_hex){
				num_is_hex = 1;
				continue;
			}
		}
		
		if(num_is_hex){
			if(!VS_IsValidOperator(expr[i]) && !isxdigit(expr[i]) && expr[i] != '\n'){
				return 0;
			}
		}
		else{
			if(!VS_IsValidOperator(expr[i]) && !isdigit(expr[i]) && expr[i] != '\n'){
				return 0;
			}
		}
		
		if(VS_IsValidOperator(expr[i])){
			num_is_hex = 0;
		}
		
		if(VS_IsValidOperator(prev_op) && VS_IsValidOperator(cur_op)){
			if(!VS_IsValidOperatorPair(prev_op, cur_op, expr[i+1])){
				return 0;
			}
		}
		
		prev_op = cur_op;
    }
	
	return 1;
}

void VS_ConvertExpr(char infix[], char postfix[]){ 
   int i = 0, j = 0;
   stack[++top] = '#'; 
   while(infix[i] != '\0') {
      if(isxdigit(infix[i]) || infix[i] == 'x' || infix[i] == '$') {
         // Append full number
         while(isxdigit(infix[i]) || infix[i] == 'x' || infix[i] == '$') {
            postfix[j++] = infix[i++];
         }
         postfix[j++] = ' '; // space as delimiter
         continue;
      }

      if(infix[i] == '<' && infix[i+1] == '<') {
         while(VS_OperatorPrecedence('<') <= VS_OperatorPrecedence(stack[top])) {
            postfix[j++] = VS_PopItemFromExprStack(); postfix[j++] = ' ';
         }
         VS_PushItemToExprStack('<'); i += 2; continue;
      }
      if(infix[i] == '>' && infix[i+1] == '>') {
         while(VS_OperatorPrecedence('>') <= VS_OperatorPrecedence(stack[top])) {
            postfix[j++] = VS_PopItemFromExprStack(); postfix[j++] = ' ';
         }
         VS_PushItemToExprStack('>'); i += 2; continue;
      }

      char symbol = infix[i];
      if(symbol == '(') {
         VS_PushItemToExprStack(symbol);
      } else if(symbol == ')') {
         while(stack[top] != '(') {
            postfix[j++] = VS_PopItemFromExprStack(); postfix[j++] = ' ';
         }
         VS_PopItemFromExprStack(); // VS_PopItemFromExprStack '('
      } else if (VS_IsValidOperator(symbol)) {
         while(VS_OperatorPrecedence(symbol) <= VS_OperatorPrecedence(stack[top])) {
            postfix[j++] = VS_PopItemFromExprStack(); postfix[j++] = ' ';
         }
         VS_PushItemToExprStack(symbol);
      }
      i++;
   }
   while(stack[top] != '#') {
      postfix[j++] = VS_PopItemFromExprStack(); postfix[j++] = ' ';
   }
   postfix[j] = '\0';
}

int VS_LineContainsOperator(char* line){
	int i, len = strlen(line);
	for(i = 0; i < len; i++){
		if(VS_IsValidOperator(line[i])){
			return 1;
		}
	}
	return 0;
}

long VS_EvaluateExpr(char *input, VS_SYNTAX syntax){
	char output[VS_STACK_SIZE];
	
	if(VS_IsValidExpression(input,syntax)){
		if(strlen(input) >= VS_STACK_SIZE){
			char in[VS_STACK_SIZE];
			memset(in,'\0',VS_STACK_SIZE);
			strncpy(in,input,VS_STACK_SIZE);
			VS_ConvertExpr(in,output);
			printf("Warning: Expression length is larger than max expression stack size. Truncating expression\n");
		}
		else{
			VS_ConvertExpr(input,output);
		}

		int i = 0, len = strlen(output);
		for(i = 0; i < len; i++){
			if(isspace(output[i])){
				i++;
				continue;
			}
			
			if((output[i] == '0' && output[i+1] == 'x') || (output[i] == '$')){
				if(output[i] == '$'){
					i++;
				}
				else{
					i += 2;
				}
				
				char hexnum[20];
				int k = 0;

				while (isxdigit(output[i]) && k < 20) {
					hexnum[k++] = output[i++];
				}
				hexnum[k] = '\0';

				long value = (long)strtoul(hexnum, NULL, 16);

				VS_PushItemToExprStack_long(value);
			}
			else if(isdigit(output[i])){
				long num = 0;
				while(isdigit(output[i])){
					num = num * 10 + (output[i++] - '0');
				}
				VS_PushItemToExprStack_long(num);
			} else {
				long op2 = VS_PopItemFromExprStack_long();
				long op1 = VS_PopItemFromExprStack_long();
				
				switch(output[i]){
					case '+': VS_PushItemToExprStack_long(op1 + op2); break;
					case '-': VS_PushItemToExprStack_long(op1 - op2); break;
					case '*': VS_PushItemToExprStack_long(op1 * op2); break;
					case '/': VS_PushItemToExprStack_long(op1 / op2); break;
					case '<': VS_PushItemToExprStack_long(op1 << op2); break;
					case '>': VS_PushItemToExprStack_long(op1 >> op2); break;
					case '&': VS_PushItemToExprStack_long(op1 & op2); break;
					case '|': VS_PushItemToExprStack_long(op1 | op2); break;
				}
				
				i++;
			}
		}
	}

	return stack_int[top_int];
}