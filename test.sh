#!/bin/sh

#    Copyright (C) 2024  Einholz
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Affero General Public License as published
#    by the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Affero General Public License for more details.
#
#    You should have received a copy of the GNU Affero General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.

./texalc-test test_start; true
fails=0;
passes=0;
if ./texalc-test test_test; then
    ((passes++));
else
    ((fails++));
fi;
if ./texalc-test integer_one; then
    ((passes++));
else
    ((fails++));
fi;
if ./texalc-test integer_zero; then
    ((passes++));
else
    ((fails++));
fi;
if ./texalc-test integer_copy; then
    ((passes++));
else
    ((fails++));
fi;
if ./texalc-test integer_create_bin; then
    ((passes++));
else
    ((fails++));
fi;
if ./texalc-test integer_create_dec; then
    ((passes++));
else
    ((fails++));
fi;
if ./texalc-test integer_create_hex; then
    ((passes++));
else
    ((fails++));
fi;
if ./texalc-test integer_unsigned_add; then
    ((passes++));
else
    ((fails++));
fi;
if ./texalc-test integer_signed_add; then
    ((passes++));
else
    ((fails++));
fi;
if ./texalc-test integer_unsigned_mul; then
    ((passes++));
else
    ((fails++));
fi;
if ./texalc-test integer_signed_mul; then
    ((passes++));
else
    ((fails++));
fi;
if ./texalc-test integer_gcd; then
    ((passes++));
else
    ((fails++));
fi;
if ./texalc-test integer_div_invalid; then
    ((passes++));
else
    ((fails++));
fi;
if ./texalc-test integer_div; then
    ((passes++));
else
    ((fails++));
fi;
if ./texalc-test integer_to_str; then
    ((passes++));
else
    ((fails++));
fi;
echo =========;
printf "TOTAL: %2d\n" $((passes + fails));
printf "PASS:  %2d\n" $((passes));
printf "FAIL:  %2d\n" $((fails));
echo =========;
exit $fails
