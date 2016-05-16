#!/usr/bin/env python3
import plotly.graph_objs as go
from plotly.offline import plot

import sys
import pandas as pd

if ( len(sys.argv) < 2):
	print("Synopsis %s <filename>\n" % sys.argv[0])
	sys.exit(1)
filename = sys.argv[1]

z_data = pd.read_csv(filename, skiprows=[0], header=None)

data = [
    go.Surface(
        z=z_data.as_matrix()
    )
]

layout = go.Layout(
    title=filename,
    width=1000,
    height=1000,
    margin=dict(  l=50,  r=50,   b=50,   t=50   ),
    autosize=True
)
fig = go.Figure(data=data, layout=layout)
plot(fig, filename=filename +  ".html")

