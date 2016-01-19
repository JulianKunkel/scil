import matplotlib.pyplot as plt
from glob import glob

def main():
    graphs = []
    for file in glob('*.csv'):

        with open(file) as f:
            lines = f.read().splitlines()

        xs = []
        ys = []

        for l in lines:

            s = l.split(',')
            xs.append(s[0])
            ys.append(s[1])

        graphs.append([xs, ys])

    for g in graphs:
        plt.plot(g[0], g[1])

    plt.xscale('log')
    plt.xlabel('buffer size in byte')

    plt.yscale('log')
    plt.ylabel('time in seconds')
    plt.show()

    print(graphs)

if __name__ == '__main__':
    main()
