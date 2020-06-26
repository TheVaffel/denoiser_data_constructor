#!/usr/bin/python3

import json
import numpy as np
import matplotlib.pyplot as plt
import sys
import os
import getopt
import random

type_str = 'bmfr'

plt.rcParams.update({'font.size': 12})

def print_usage():
    print('Usage\npython3 plåtz [--json-data <json data file>]')

def get_js(argv):
    try:
        opts, args = getopt.getopt(argv, "h", ['help', 'json-data='])
    except getopt.GetoptError:
        print_usage()
        sys.exit(-1)
    for opt, arg in opts:
        if opt == '-h' or opt == '--help':
            print_usage()
        if opt == '--json-data':
            return arg
    print_usage()

def getObj(file_name):
    try:
        ff = open(file_name);
        print('Successfully opened ' + file_name);
    except:
        print('Could not read file ' + file_name + ' stopping')
        exit(-1);
    obj = json.load(ff);
    ff.close();
    return obj;
    
def main_cross_plot():
    times1 = '/home/haakon/data/denoiser_results/evaluate_results_new/results.txt';
    times2 = '/home/haakon/data/denoiser_results/evaluate_results_original/results.txt';
    base1 = '/home/haakon/data/denoiser_results/evaluate_results_new/diff_results';
    base2 = '/home/haakon/data/denoiser_results/evaluate_results_original/diff_results';
    avs1 = []
    avs2 = []
    i = 0;
    while i < 10:
        file_name1 = base1 + str(i) + '.txt';
        obj1 = getObj(file_name1);
        file_name2 = base2 + str(i) + '.txt';
        obj2 = getObj(file_name2);

        scores1 = obj1['VMAF']
        scores2 = obj2['VMAF']

        av1 = sum(scores1) / len(scores1);
        av2 = sum(scores2) / len(scores2);

        avs1.append(av1);
        avs2.append(av2);

        i += 1

    av_times1 = []
    av_times2 = []
    
    obj1 = getObj(times1);
    obj2 = getObj(times2);

    for i in range(1, 11):
        time_data1 = obj1[str(i)];
        times1 = time_data1['total']
        av_time1 = sum(times1) / len(times1);
        av_times1.append(av_time1);

        time_data2 = obj2[str(i)];
        times2 = time_data2['total']
        av_time2 = sum(times2) / len(times2);
        av_times2.append(av_time2);

    fig, ax = plt.subplots()

    ax.plot(av_times1, avs1);
    ax.plot(av_times2, avs2);

    plt.show()
    

def main_evaluate_bmfr():
    base = '/home/haakon/data/denoiser_results/bmfr_evaluate_results/diff_results';
    base2 = '/home/haakon/BigProjects/bmfr/opencl/results/diff_results';

    out_file_name = 'bmfr_evaluate_new.png'
    i = 0;

    # fig = plt.figure(figsize=(8.0, 8.0))
    fig = plt.figure()
    ax = fig.add_subplot(111)
    # fig, ax = plt.subplots();
    # plt.xticks([0, 20, 40]);
    avs1 = []
    avs2 = []
    
    while i < 10:
        file_name = base + str(i) + '.txt';
        file_name2 = base2 + str(i) + '.txt';
        try:
            ff = open(file_name);
            ff2 = open(file_name2);
            print('Successfully opened ' + file_name);
        except:
            print('Could not read file ' + file_name + ' or ' + file_name2 + ', stopping')
            break;
        obj = json.load(ff);
        obj2 = json.load(ff2);
        ff.close();
        ff2.close();
        
        scores = obj['VMAF'];
        scores2 = obj2['VMAF'];
        av = sum(scores) / len(scores);
        avs1.append(av);
        av2 = sum(scores2) / len(scores2);
        avs2.append(av2);

        # ax.set_xlim([0, 59])
        s = ''
        if i > 0:
            s = 's'
        # ax.plot(scores, label=(str(i + 1) + ' buffer' + s));
        i += 1;
    print(avs1);
    print(avs2);
    ax.plot(range(1, 11), avs1, label='New buffers');
    ax.plot(range(1, 11), avs2, label='Original buffers');
    ax.legend()
    ax.set_ylabel('Average VMAF score')
    ax.set_xlabel('Number of buffers')
    plt.tight_layout();
    out = 'buffers_av_comp.png';
    plt.savefig(out);
    plt.show()

    print('Saved image to ', out);
    return;

    plt.tight_layout()
    plt.savefig(out_file_name);
    print('Wrote output to ' + out_file_name);
    
    plt.show()

def main_evaluate_single():
    
    base = '/home/haakon/data/denoiser_results/bmfr_evaluate_results/diff_results';
    fig, ax = plt.subplots(figsize=(8,8));
    
    ax.set_xlim([0, 59]);
    plt.xticks([0, 20, 40]);
    i = 0
    while i < 10:
        file_name = base + str(i) + '.txt'
        obj = getObj(file_name);

        scores = obj['VMAF'];
        av = sum(scores) / len(scores);
        # print('Av ' + str(i) + ': ' + str(av));
    
        s = ''
        if i > 0:
            s = 's'
        ax.plot(scores, label=(str(i + 1) + ' buffer' + s));
        i += 1
        # ax.set_aspect(0.8);
    ax.legend(ncol=2);
    ax.set_ylabel('VMAF');
    ax.set_xlabel('Frame number');
    out = 'bmfr_evaluate_new.png';
    plt.tight_layout();
    plt.savefig(out, dpi=250);
    print('Saved to ' + out);
    plt.show();
        
def main_diff(scenes, algs, score_names):
    base = '/home/haakon/Documents/NTNU/TDT4900/dataconstruction/denoise_util/diffcal/diff_results_';
    
    fig, ax = plt.subplots(1, len(scenes), sharey=True);
    fig.subplots_adjust(wspace=0.1)
    
    plt.xticks([0, 20, 40]);
    for scenei in range(len(scenes)):

        scene = scenes[scenei]
        # plt.subplot(210 + scenei + 1)
        for alg in algs:
            for score_name in score_names:
                file_name = base + scene + '_' + alg + '.json';
                print("Using json file " + file_name)
                ff = open(file_name);
                obj = json.load(ff);
                ff.close;
                
                scores = obj[score_name];
                
                # ax[scenei].set_xlabel(scene);

                # ax[scenei].set_xlim([0, 59])
                ax.set_xlim([0, 59])
                # ax[scenei].set_ylim([0.0, 100])
                ax.set_ylim([0.0, 0.02]);

                # for l in range(1,len(nr)):
                # ax.plot_func(nr[l], bottoms=nr[l - 1]);
                label = alg.upper(); # stage_names[l-1]
                plt.xticks([0, 20, 40]);
                # ax[scenei].plot(scores, label=label, linewidth=5.0);
                ax.plot(scores, label=label, linewidth=5.0);
    
                # ax.fill_between(list(range(len(nr[0]))), nr[l - 1], nr[l], label=label);
                # plo = ax.plot(list(range(len(nr[0]))), nr[l], label=label)
                # plo.set_label(label);
                
                # ax[scenei].set_aspect(0.67)
                # ax.set_aspect(0.67);
        
        # for line in leg.get_lines():
        #     line.set_linewidth(10);
    
    # plt.xticks([0, 20, 40]);
    # ax[-1].legend();
    ax.legend()
    # ax[0].set_ylabel(score_name);
    ax.set_ylabel(score_name);
    ax.set_xlabel('Frame number');

    file_name = 'plot_' + '_'.join(algs) + '_' + '_'.join(score_names) + '.png';
    print('Saving to ' + file_name);
    
    plt.tight_layout();
    plt.savefig(file_name, dpi=200);
    plt.show();
    # plt.show();

def plot_generic(file_name, stage_names):
    
    print("Using json file " + file_name);
    ff = open(file_name);
    obj = json.load(ff);
    ff.close();
    
    r = [obj[name] for name in stage_names];
    
    num = min([len(l) for l in r]) 
    acc = [0.0] * num;
    nr = [acc]
    for l in r:
        acc = [acc[i] + l[i] for i in range(num)]
        nr.append(acc);
        
    fig, ax = plt.subplots()
    ax.set_ylabel('Time (ms)');
    ax.set_xlabel('Frame number');

    ax.set_xlim([0, 60])
    ax.set_ylim([0.0, 9.0])

    for l in range(1,len(nr)):
        # ax.plot_func(nr[l], bottoms=nr[l - 1]);
        label = stage_names[l-1]
        
        ax.fill_between(list(range(len(nr[0]))), nr[l - 1], nr[l], label=label);
        # plo = ax.plot(list(range(len(nr[0]))), nr[l], label=label)
        # plo.set_label(label);
        
    ax.legend();

    file_name = os.path.basename(file_name); # Take json filename
    file_name = file_name[:file_name.rfind('.')]; # Strip extension
    file_name += "_" + type_str + ".png";  # add png extension to it

    plt.savefig(file_name, dpi=300);
    print("Plot saved to file " + file_name)
    # plt.show();
    

def main_svgf_times(file_name):
    stage_names = ['reproject', 'variance', 'atrous']
    plot_generic(file_name, stage_names);
    
def main_bmfr_times(file_name):
    
    stage_names = ['accum_noisy', 'fitter', 'weighted_sum', 'accum_filtered', 'taa'];
    plot_generic(file_name, stage_names);
    
def main2(argv):
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
    # global type_str;
    
    js_file = 'diff_results.json';

    type_str = 'bmfr';
    
    try:
        opts, args = getopt.getopt(sys.argv[1:], "ht", ['help', 'json-data=', 'type='])
    except getopt.GetoptError:
        print_usage()
        sys.exit(-1)
    for opt, arg in opts:
        if opt == '-h' or opt == '--help':
            print_usage()
        if opt == '--json-data':
            js_file = arg
        if opt == '--type' or opt == '-t':
            type_str = arg;

    if type_str == 'bmfr':
        main_bmfr_times(js_file);
    elif type_str == 'svgf':
        main_svgf_times(js_file);
    elif type_str == 'vmaf':
        main_diff(['sponza', 'living_room', 'san_miguel'], ['bmfr', 'svgf'], ['VMAF']);
    elif type_str == 'temporal':
        main_diff(['static'], ['svgf', 'bmfr', 'reference'], ['Temporal error']);
    elif type_str == 'evaluate_single':
        main_evaluate_single();
    elif type_str == 'evaluate':
        main_evaluate_bmfr();
    elif type_str == 'cross':
        main_cross_plot();
    else:
        print("Type didn't match anything, was " + type_str)
    # main_bmfr_times(sys.argv[1:])
