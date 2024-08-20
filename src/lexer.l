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

    #include "parser.h"
%}

%option noyywrap
%option noinput
%option nounput
/* %option nounistd */

%%

"+"          { return ADD; }
"*"          { return MUL; }
0[bB][0-9]+  { yylval.pascal_str.str = yytext; yylval.pascal_str.len = yyleng; return BIN_NUM_N; }
[0-9]+       { yylval.pascal_str.str = yytext; yylval.pascal_str.len = yyleng; return DEC_NUM_N; }
0[xX}[0-9]+  { yylval.pascal_str.str = yytext; yylval.pascal_str.len = yyleng; return HEX_NUM_N; }
\\\\         { return END; }
[ \t\n]      { /* skip whitespaces */ }
.            { printf("invalid character: %c\n", *yytext); }

%%
