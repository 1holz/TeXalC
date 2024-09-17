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
    #include "node.h"
    #include "numbers.h"

    struct pascal_str
    {
        const char *str;
        size_t len;
    };
}

%union
{
    struct pascal_str pascal_str;
    txc_node *node;
}

%token <pascal_str> BIN_NUM_N DEC_NUM_N HEX_NUM_N
%token ADD
%token MUL
%token END

%type <node> sum prod num_n

%start expr

%%

expr:
  %empty
| expr sum END  { txc_node_simplify_and_print($2); }
;

sum:
  prod
| sum ADD prod  { $$ = txc_create_bin_op(TXC_ADD, $1, $3); }
;

prod:
  num_n
| prod MUL num_n  { $$ = txc_create_bin_op(TXC_MUL, $1, $3); }
;

num_n:
  BIN_NUM_N { $$ = txc_create_natural_num_or_zero($1.str, $1.len, 2); }
| DEC_NUM_N { $$ = txc_create_natural_num_or_zero($1.str, $1.len, 10); }
| HEX_NUM_N { $$ = txc_create_natural_num_or_zero($1.str, $1.len, 16); }
;

%%

void yyerror(char const *s)
{
    fprintf(stderr, "Parser error: %s\n", s);
}
