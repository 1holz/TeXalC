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

    #include "numbers.h"

    extern int yylex(void);
    extern void yyerror(const char *);
%}

%header

%token <pascal_str> BIN_NUM_N DEC_NUM_N HEX_NUM_N
%token ADD
%token MUL
%token EXP
%token L_PAREN R_PAREN
%token END

%code requires
{
    struct pascal_str
    {
        char *str;
        size_t len;
    };
}

%union
{
    struct pascal_str pascal_str;
}

%%

num: %empty
    | num BIN_NUM_N END  { printf("= %s\n\n", txc_num_to_str(txc_create_natural_num_or_zero($2.str, $2.len))); }
    ;

/*
expr: %empty
    | expr sum END  { printf("= %d\n\n", $2); }
    ;

sum: prod
    | sum ADD BIN_NUM_N  { $$ = $1 + $3; }
    ;

prod: BIN_NUM_N
    | prod MUL BIN_NUM_N  { $$ = $1 * $3; }
    ;
*/

%%

void yyerror(char const *s)
{
    fprintf(stderr, "Error: %s\n", s);
}
