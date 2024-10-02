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
 */

%{
    #include <stdio.h>

    extern int yylex(void);
    extern void yyerror(const char *);
%}

%code requires
{
    #include "node.h"
    #include "integer.h"

    struct pascal_str
    {
        const char *str;
        size_t len;
    };
}

%union
{
    struct pascal_str pascal_str;
    const txc_node *node;
}

%token <pascal_str> BIN_INT DEC_INT HEX_INT
%left PLUS MINUS
%left CDOT FRAC
%token L_PAREN R_PAREN L_BRACE R_BRACE
%token END

%type <node> expr int

%start input

%%

input:
  /* empty */
| input line
;

line:
  END
| expr END     { txc_node_simplify_and_print($1); }
;

expr:
  int
| expr PLUS expr                                 { $$ = txc_node_create_bin_op(TXC_ADD, $1, $3); }
| expr MINUS expr                                { $$ = txc_node_create_bin_op(TXC_ADD, $1, txc_node_create_un_op(TXC_NEG, $3)); }
| expr CDOT expr                                 { $$ = txc_node_create_bin_op(TXC_MUL, $1, $3); }
| FRAC L_BRACE expr R_BRACE L_BRACE expr R_BRACE { $$ = txc_node_create_bin_op(TXC_FRAC, $6, $3); }
| L_PAREN expr R_PAREN                           { $$ = $2; }
;

int:
  BIN_INT   { $$ = txc_int_create_int_node($1.str, $1.len, 2); }
| DEC_INT   { $$ = txc_int_create_int_node($1.str, $1.len, 10); }
| HEX_INT   { $$ = txc_int_create_int_node($1.str, $1.len, 16); }
| MINUS int { $$ = txc_node_create_un_op(TXC_NEG, $2); }
| PLUS int  { $$ = $2; }
;

%%

void yyerror(char const *s)
{
    fprintf(stderr, "Parser error: %s\n", s);
}
