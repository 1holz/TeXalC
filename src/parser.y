/*
 *     Copyright (C) 2024  Einholz
 *
 *     This program is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published
 *     by the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 *
 *     You should have received a copy of the GNU Affero General Public License
 *     along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

%{
    #include <stdio.h>

    extern int yylex(void);
    extern void yyerror(const char *);
%}

%code requires
{
    #include "numbers.h"

    struct pascal_str
    {
        const char *str;
        size_t len;
    };
}

%token <pascal_str> BIN_NUM_N DEC_NUM_N HEX_NUM_N
%token ADD
%token MUL
%token END

%type <num> sum prod num_n

%union
{
    struct pascal_str pascal_str;
    txc_num *num;
}

%start expr

%%

expr:
  %empty
| expr sum END  { printf("= %s\\\\\n\n", txc_num_to_str($2)); }
;

sum:
  prod
| sum ADD num_n  { txc_num *summands[2] = {$1, $3}; $$ = txc_num_add(summands, 2); }
;

prod:
  num_n
| prod MUL num_n  { $$ = (txc_num *)&TXC_NAN_ERROR_NYI; }
;

num_n:
  BIN_NUM_N { $$ = txc_create_natural_num_or_zero($1.str, $1.len); }
| DEC_NUM_N { $$ = txc_create_natural_num_or_zero($1.str, $1.len); }
| HEX_NUM_N { $$ = txc_create_natural_num_or_zero($1.str, $1.len); }
;

%%

void yyerror(char const *s)
{
    fprintf(stderr, "Parser error: %s\n", s);
}
