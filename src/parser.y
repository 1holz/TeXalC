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

%token INT
%token ADD MUL
%token END

%%

expr:
    | expr sum END  { printf("= %d\n\n", $2); };

sum: prod
    | sum ADD sum  { $$ = $1 + $3; }
    ;

prod: INT
    | INT MUL INT   { $$ = $1 * $3; }
    | prod MUL INT  { $$ = $1 * $3; }
    ;

%%

void yyerror(char const *s)
{
    fprintf(stderr, "Error: %s\n", s);
}
