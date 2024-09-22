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
    #include "integers.h"

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

%token <pascal_str> BIN_INT DEC_INT HEX_INT
%token PLUS
%token MINUS
%token CDOT
%token L_PAREN
%token R_PAREN
%token END

%type <node> sum prod signd paren int

%start expr

%%

expr:
  /* empty */
| expr sum END  { txc_node_simplify_and_print($2); }
;

sum:
  prod
| sum PLUS prod   { $$ = txc_node_create_bin_op(TXC_ADD, $1, $3); }
| sum MINUS prod  { $$ = txc_node_create_bin_op(TXC_ADD, $1, txc_node_create_un_op(TXC_NEG, $3)); }
;

prod:
  signd
| prod CDOT signd  { $$ = txc_node_create_bin_op(TXC_MUL, $1, $3); }
;

signd:
  paren
| int
| MINUS signd  { $$ = txc_node_create_un_op(TXC_NEG, $2); }
| PLUS signd  { $$ = $2; }
;

paren:
  L_PAREN sum R_PAREN  { $$ = $2; }
;

int:
  BIN_INT { $$ = txc_int_create_int($1.str, $1.len, 2); }
| DEC_INT { $$ = txc_int_create_int($1.str, $1.len, 10); }
| HEX_INT { $$ = txc_int_create_int($1.str, $1.len, 16); }
;

%%

void yyerror(char const *s)
{
    fprintf(stderr, "Parser error: %s\n", s);
}
