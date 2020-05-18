#!/usr/bin/python3

import json
import numpy as np
import matplotlib.pyplot as plt
import sys
import getopt

def print_usage():
    print('Usage\n./plåtz [--json-data <json data file>]')

def main(argv):
    js_file = 'diff_results.json';
    try:
        opts, args = getopt.getopt(argv, "h", ['help', 'json-data='])
    except getopt.GetoptError:
        print_usage()
        sys.exit(-1)
    for opt, arg in opts:
        if opt == '-h' or opt == '--help':
            print_usage()
        if opt == '--json-data':
            js_file = arg

    ff = open(js_file);
    obj = json.load(ff)
    ff.close()

    for key in obj:

        fig, ax = plt.subplots()
        ax.plot(obj[key])
        
        ax.set_ylabel(key + ' score')
        ax.set_xlabel('frame')

        plt.savefig(key + '_plot.png')
    # plt.show()


if __name__ == "__main__":
    main(sys.argv[1:])
