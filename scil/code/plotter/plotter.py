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

def get_throughputs(csv):
    inter = np.arange(6)
    for c in csv:
        inter[(int)(c[0])] += (int)(c[1]) / (float)(c[3])

    for i in range(6):
        inter[i] /= 7000000

    return inter

def main(args):

    if len(args) != 2:
        print('Provide path to csv file.')
        return

    csv = open_csv(args[1])

    throughs = get_throughputs(csv)
    print(throughs)

    index = np.arange(5)
    bar_width = 0.35

    rects = plt.bar(index + bar_width/2, throughs[1:], bar_width, color='b')

    plt.xlabel('Algorithms')
    plt.ylabel('Throughput (MB/s)')
    plt.title('Throughput by algorithm')
    plt.xticks(index + bar_width, ('Abstol', 'GZIP', 'Sigbits', 'FPZip', 'ZFP'))
    plt.grid(True)

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
