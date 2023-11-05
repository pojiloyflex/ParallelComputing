
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import matplotlib.ticker as ticker
from matplotlib.animation import FuncAnimation

fig, ax = plt.subplots()

all_x = []
all_y = []

i = 0
with open("coordinates.txt") as fil:
    # points = fil.read()
    for coord in fil:
        if i%2==1:
            coordinates = coord.split()
            xs = list(map(float, coordinates[::2]))
            ys = list(map(float, coordinates[1::2]))
            all_x.append(xs)
            all_y.append(ys)
            
        i+=1

all_coordinates = [(all_x[i],all_y[i]) for i in range(len(all_x))]

xdata, ydata = [], []
ln, = ax.plot([], [], 'ro')

def init():
    ax.set_xlim(-1000, 1000)
    ax.set_ylim(-1000, 1000)
    return ln,

def update(coordinates_step):
    xdata=[]
    ydata=[]
    
    for i in coordinates_step[0]:
        xdata.append(i)
    for i in coordinates_step[1]:    
        ydata.append(i)
    ln.set_data(xdata, ydata)
    
    return ln,

ani = FuncAnimation(fig, update, frames=all_coordinates,
                    init_func=init, interval=10, blit=True)
plt.show()