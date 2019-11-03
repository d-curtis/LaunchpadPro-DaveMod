import matplotlib.pyplot as plt
from matplotlib import colors
from scipy import ndimage
import numpy as np
import math
import os

TABLE_SIZE = 128


headerheader = """
/*
This file was auto-generated.
If there's something wrong, fix tools/velocitylut.py
*/

#include "app.h"
#ifndef CURVES_H
#define CURVES_H

"""

headerfooter = "\n#endif\n"



def make_array(var, arr):
    res = f"u8 {var}[{len(arr)}] =\n{{\n"
    colcount = 0
    for i in range(len(arr)):
        if colcount > 12:
            colcount = 0
            res += "\n"
        res += "\t" + str(arr[i])
        if i < len(arr)-1:
            res += ","
        
        # It was bugging me that the tabs wouldn't align properly... Pad to 4 characters.
        if i < 10:
            res += "   "
        elif i < 100:
            res += "  "
        elif i < 1000:
            res += " "

        colcount += 1
    res += "\n};\n\n"
    return res

#   --      --      --      --      Generate curves

lin_hi = range(0, TABLE_SIZE)
log_hi = np.log10([i+1 for i in range(TABLE_SIZE)])
log_hi = np.around(log_hi/(log_hi.max()/TABLE_SIZE)).astype(int)
inv_hi = np.invert(np.flip(log_hi)) + TABLE_SIZE + 1

lin_lo = range(0, 8)
log_lo = np.log10([i+1 for i in range(8)])
log_lo = np.around(log_lo/(log_lo.max()/8)).astype(int)
inv_lo = np.invert(np.flip(log_lo)) + 9

curves_hi = []
curves_hi.append(["lut_vel1_hi", lin_hi])
curves_hi.append(["lut_vel2_hi", log_hi])
curves_hi.append(["lut_vel3_hi", inv_hi])

curves_lo = []
curves_lo.append(["lut_vel1_alias", lin_lo])
curves_lo.append(["lut_vel2_alias", log_lo])
curves_lo.append(["lut_vel3_alias", inv_lo])




#   --      --      --      --      Write the header file

with open("../src/velocity.h", "w+") as f:
    headerstring = headerheader
    for i in curves_hi:
        headerstring += make_array(i[0], i[1])
    for i in curves_lo:
        headerstring += make_array(i[0], i[1])
    headerstring += headerfooter
    f.write(headerstring)
    print(f"Wrote {len(headerstring.encode('utf-8'))} bytes to ../src/velocity.h")


#   --      --      --      --      Plot them for visual joy

space_hi = np.linspace(1, TABLE_SIZE, TABLE_SIZE)
space_lo = np.linspace(1, 8, 8)

plt.subplot(2, 1, 1)
legend = []
for i in curves_hi:
    plt.plot(space_hi, i[1])
    legend.append(i[0])
plt.title("Velocity curve")
plt.legend(legend)

plt.subplot(2, 1, 2)
legend = []
for i in curves_lo:
    plt.scatter(space_lo, i[1], marker="s", s=200)
legend.append(i[0])
plt.title("Grid display")
plt.grid()
plt.legend(legend)

# space = np.linspace(0, 8, 8)
# legend = []
# for i in curves_lo:
#     plt.plot(space, i[1])
#     legend.append(i[0])



# plt.legend(legend)
# plt.xticks(range(math.floor(min(space)), math.ceil(max(space))+1))

plt.show()