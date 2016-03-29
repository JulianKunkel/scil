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

    fig, sp = plt.subplots()

    sp.set_title("Execution time", fontsize=20)
    sp.set_xlabel('buffer size in byte', fontsize=16)
    sp.set_ylabel('time in seconds', fontsize=16)
    sp.set_xscale('log')
    sp.set_yscale('log')
    sp.grid(True,linestyle='-',color='0.75')

    labels = ['memcpy', 'abstol']

    for g, l in zip(graphs, labels):
        sp.plot(g[0], g[1], label=l)

    sp.legend(loc='upper left')

    plt.show()

if __name__ == '__main__':
    main()
