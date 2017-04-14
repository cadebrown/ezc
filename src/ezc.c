/* ezc.c -- main file for ec, or EZ calc

  Copyright 2016-2017 ChemicalDevelopment

  This file is part of the EZC project.

  EZC, EC, EZC Documentation, and any other resources in this project are 
free software; you are free to redistribute it and/or modify them under 
the terms of the GNU General Public License; either version 3 of the 
license, or any later version.

  These programs are hopefully useful and reliable, but it is understood 
that these are provided WITHOUT ANY WARRANTY, or MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GPLv3 or email at 
<info@chemicaldevelopment.us> for more info on this.

  Here is a copy of the GPL v3, which this software is licensed under. You 
can also find a copy at http://www.gnu.org/licenses/.

*/

#include "ezc_impl.h"


argp_t arg_parse;

void end() {

}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    if (arg_parse.show_help) {
      show_help();
      return 0;
    }

    if (arg_parse.show_info) {
      show_info();
      return 0;
    }

    if (arg_parse.show_version) {
      show_version();
      return 0;
    }


    int i = 0;
    run_t *cur_target;
    while (i < arg_parse.targets.size) {
      cur_target = dynarray_get(arg_parse.targets, run_t *, i);
      if ((*cur_target).howto == RUN_HOWTO_EXPR) {
        printf("%d (expr): %s\n", i, (*cur_target).goal);
      } else if ((*cur_target).howto == RUN_HOWTO_FILE) {
        printf("%d (file): %s\n", i, (*cur_target).goal);
      } else {
        printf("Idk: %s\n", (*cur_target).goal);
      }
      i++;
    }
    return 0;
}


