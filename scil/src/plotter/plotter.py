import sys, os
import matplotlib.pyplot as plt
import numpy as np
import glob as g

algos = {
    '0' : 'memcpy',
    '1' : 'abstol',
    '2' : 'gzip',
    '3' : 'sigbits',
    '4' : 'fpzip',
    '5' : 'zfp-abs',
    '6' : 'zfp-rel'
}

metrics = {
    'id' : 0,
    'abs-tol' : 1,
    'sig-bits' : 4,
    'u-size' : 5,
    'c-size' : 6,
    'c-ratio' : 7,
    'c-time' : 8
}

colors = ['#0000ff', '#00aaaa', '#00ff00', '#aaaa00', '#ff0000', '#aa00aa', '#aaaaaa']

def open_csv(path):
    result = []

    inter = ""
    with open(path, 'r') as f:
        inter = f.read()

    inter = inter.splitlines()

    for i in inter:
        result.append(i.split(','))

    return result

def get_all_of_metric(metric, csv):
    if not metrics.has_key(metric):
        return None

    col = metrics[metric]

    result = {}
    for v in algos.values():
        result[v] = []

    for c in csv:
        result[algos[c[0]]].append(c[col])

    return result

def plot(algo, xaxis, yaxis, color):
    plt.plot(xaxis.get(algo), yaxis.get(algo), color=color, label=algo)

def plot_line_chart(algos, xaxis, yaxis, xlabel=None, ylabel=None, title=None):

    c = 0
    for a in algos:
        plot(a, xaxis, yaxis, color=colors[c])
        c += 1

    plt.gca().set_ylim([0, 35])
    plt.plot()

    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.title(title)

    plt.grid(True)
    plt.legend()

def plot_bar_chart(algos, xaxis, yaxis, xlabel=None, ylabel=None, title=None):
    return None

def plot_sigs(path):
    csv = open_csv(path)

    sigbits = get_all_of_metric('sig-bits', csv)
    ratios = get_all_of_metric('c-ratio', csv)

    plot_line_chart(['sigbits', 'zfp-rel'],
                    sigbits,
                    ratios,
                    'Significant bits',
                    '',
                    '')

    plt.gca().set_xlim([1, 15])
    plt.grid(True)
    plt.legend()

    #plt.savefig(args[1] + '.' + args[2] + '.png')

def plot_abs(path):
    csv = open_csv(path)

    abstols = get_all_of_metric('abs-tol', csv)
    ratios = get_all_of_metric('c-ratio', csv)

    plt.gca().invert_xaxis()

    plot_line_chart(['abstol', 'zfp-abs'],
                    abstols,
                    ratios,
                    'Absolute tolerance',
                    'Compression factor',
                    '')

    plt.semilogx(basex=2)

    plt.grid(True)
    plt.legend()

    #plt.savefig(args[1] + '.' + args[2] + '.png')

def plot_lole():
    paths = g.glob("*.csv")
    bar_width = 1.0 / (2 * 3)
    ind = np.arange(len(paths))

    # fetch data -------------
    datas = {
        'memcpy' : [],
        'gzip' : [],
        'fpzip' : []
    }

    for path in paths:
        ccsv = open_csv(path)
        ratios = get_all_of_metric('c-ratio', ccsv)

        for a in datas.keys():
            datas[a].append(float(ratios.get(a)[0]))
    # ------------------------

    plt.gca().set_ylim([0.9, 1.4])

    plt.bar(ind, datas['memcpy'], width=bar_width, color='#00aaaa', label='memcpy')
    plt.bar(ind + bar_width, datas['gzip'], width=bar_width, color='#aaaa00', label='gzip')
    plt.bar(ind + 2 * bar_width, datas['fpzip'], width=bar_width, color='#aa00aa', label='fpzip')

    plt.xlabel('Data smoothness')
    plt.ylabel('Compression factor')
    plt.title('Compression rates of lossless methods')
    plt.xticks(ind + len(datas.keys()) * 0.5 * bar_width, np.arange(0, 1.2, 0.25))


    plt.grid(True)
    plt.legend()

    plt.show()

def plot_times():

    paths = g.glob("*.csv")
    bar_width = 1.0 / (2 * len(paths))
    ind = np.arange(len(paths))

    # fetch data -------------
    datas = {}
    for a in algos.values():
        datas[a] = []

    for path in paths:
        ccsv = open_csv(path)
        times = get_all_of_metric('c-time', ccsv)
        u_sizes = get_all_of_metric('u-size', ccsv)

        for a in algos.values():
            datas[a].append(float(u_sizes.get(a)[0]) / (1000000 * float(times.get(a)[0])))
    # ------------------------

    for e,a in enumerate(algos.values()):
        plt.bar(ind + e * bar_width, datas[a], width=bar_width, color=colors[e], label=a)

    plt.xlabel('Data smoothness')
    plt.ylabel('Throughput (MB/s)')
    plt.title('Compression throughputs')
    plt.xticks(ind + len(algos.keys()) * 0.5 * bar_width, np.arange(0, 1.2, 0.25))


    plt.grid(True)
    plt.legend()

    plt.show()

def main(args):

    plt.figure(figsize=(15, 5))

    plt.subplot(1, 2, 1)

    plot_abs(args[1])

    plt.subplot(1, 2, 0)

    plot_sigs(args[1])

    #plt.show()
    plt.savefig('075_lossy_rates.png')

    """
    hstr = 'Provide path to csv file and stuff.'

    plt.subplot(3, 5, 2)

    bar_width = 0.25

    ind = np.arange(5)
    rats = [ratios.get('gzip'), ratios.get('fpzip')]

    print times.get('gzip')[0]

    h4 = [4,4,4,4,4]
    h5 = [5,5,5,5,5]

    plt.bar(ind, h4, width=bar_width, color='#0000ff')
    plt.bar(ind + bar_width, h5, width=bar_width, color='#ff0000')

    plt.ylabel('Compression factor')
    plt.title('Compression factors of lossless methods')
    plt.xticks(ind + bar_width, np.arange(0, 1.2, 0.25))

    plt.grid(True)
    plt.legend()

    plt.tight_layout()
    plt.show()
"""

if __name__ == '__main__':
    main(sys.argv)

"""
Thoughputs:
    buffer of sizes 1MB to 512MB
    hints.absolute_tolerance = 0.005;
	hints.relative_tolerance_percent = 1.0;
	hints.significant_bits = 5;

Ratios:
    buffer of sizes 8MB
"""
