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

    #include "parser.h"
%}

%option noyywrap
%option noinput
%option nounput
/* %option nounistd */

%%

\(                 { return L_PAREN; }
\)                 { return R_PAREN; }
\{                 { return L_BRACE; }
\}                 { return R_BRACE; }
\+                 { return PLUS; }
\-                 { return MINUS; }
\\cdot             { return CDOT; }
\\frac             { return FRAC; }
0[bB][01]+         { yylval.pascal_str.str = yytext + 2; yylval.pascal_str.len = yyleng - 2; return BIN_INT; }
[0-9]+             { yylval.pascal_str.str = yytext; yylval.pascal_str.len = yyleng; return DEC_INT; }
0[xX][0-9A-Fa-f]+  { yylval.pascal_str.str = yytext + 2; yylval.pascal_str.len = yyleng - 2; return HEX_INT; }
\\\\               { return END; }
[ \t\n]            { /* skip whitespaces */ }
.                  { fprintf(stderr, "Parser error: invalid character: %c\n", *yytext); }

%%
