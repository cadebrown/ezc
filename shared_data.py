var_names = set()

def get_var_init(var):
    return "mpfr_t %s; mpfr_init(%s);" % (var, var)

def get_var_free(var):
    return "mpfr_clear(%s);" % (var)

def is_const(str):
    str = clean_line(str)
    return str[0].isdigit() or str[0] == "." or str[0] == "-"

def is_pos_int(str):
    return str[0].isdigit()

def clean_line(v):
    v = v.lstrip()
    v = v.rstrip()
    return v

def split_l(aa):
    r = []
    for a in aa:
        a = clean_line(a)
        if a and a != "":
            r.append(a)
    return r

def parse_function(l):
    l = clean_line(l)
    if "=" in l:
        assign, args = l.split("=")
        return clean_line(assign), split_l(args.split(" "))
    else:
        args = split_l(l.split(" "))
        return "", args


class Statement():
    def __init__(self, assign, args):
        if assign and assign != "":
            var_names.add(assign)
        self.assign = assign
        self.args = args

class Operator():
    def __init__(self, assign, a, b):
        self.assign = assign
        self.a = a
        self.b = b


libs = """#include <stdio.h>
#include <stdlib.h>
#include <mpfr.h>
#include <math.h>
"""

license = """/* 
   Copyright (C) 2016 ChemicalDevelopment
   EZC transpiled code.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
"""

start="""
int main(int argc, char *argv[]) {
    int _prec, _pprec;
"""

end="""
}
"""