import sys
import matplotlib.pyplot as plt
import numpy as np

def open_csv(path):
    result = []

    inter = ""
    with open(path, 'r') as f:
        inter = f.read()

    inter = inter.splitlines()
    inter = inter[1:]

    for i in inter:
        result.append(i.split(','))

    return result

def get_columns(csv, column):
    inter = []
    for i in range(6):
        inter.append([])

    for c in csv:
        inter[(int)(c[0])].append([(float)(c[column])])

    return inter

def main(args):

    if len(args) != 2:
        print('Provide path to csv file.')
        return

    csv = open_csv(args[1])

    abs_tols = get_columns(csv, 1)
    sig_bits = get_columns(csv, 2)
    ratios = get_columns(csv, 3)

    print(abs_tols)
    print(sig_bits)
    print(ratios)

    index = np.arange(5)
    bar_width = 0.35

    plt.subplot(2, 1, 1)
    plt.plot(sig_bits[3], ratios[3], color='b', label='Sigbits')

    plt.xlabel('Significant Bits')
    plt.ylabel('Compression factor')
    plt.title('Compression factor by relative error tolerance')

    plt.grid(True)
    plt.legend()

    plt.subplot(2, 1, 0)
    plt.gca().invert_xaxis()
    plt.semilogx(abs_tols[1], ratios[1], basex=2, color='b', label='Abstol')

    #plt.plot(abs_tols[1], ratios[1], color='b')
    plt.plot(abs_tols[2], ratios[2], color='g', label='GZip')
    #plt.plot(abs_tols[3], ratios[3], color='b')
    plt.plot(abs_tols[4], ratios[4], color='r', label='FPZip')
    plt.plot(abs_tols[5], ratios[5], color='purple', label='ZFP')

    plt.xlabel('Absolute error tolerance')
    plt.ylabel('Compression factor')
    plt.title('Compression factor by absolute error tolerance')
    #plt.xticks(index + bar_width, ('Abstol', 'GZIP', 'Sigbits', 'FPZip', 'ZFP'))
    plt.grid(True)
    plt.legend()

    plt.tight_layout()
    plt.show()

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
