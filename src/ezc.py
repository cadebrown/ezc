import argparse
import ezlogging

parser = argparse.ArgumentParser(description='Compile EZC Language. v{0}'.format(ezlogging.version))

parser.add_argument('files', metavar='files', type=str, nargs='*', default=[], help='files to compile')
parser.add_argument('-o', default="a.out", help='Output file')

parser.add_argument('-W', default="tmp.c", type=str, help='Tmp file')




