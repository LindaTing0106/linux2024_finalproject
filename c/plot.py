#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import os

def collect(binary):
    binary_path = f"./{binary}"
    t = os.popen(binary_path).read()
    t = np.fromstring(t, dtype=int, sep=',')
    t = t[:-1] # remove the last seperator
    return t

os.system("make")
t7 = collect("test_linux")
t1 = collect("test_checking2")
# t2 = collect("test_checking2")
# t3 = collect("test_checking3")
# t4 = collect("test_checking4")
# t5 = collect("test_checking5")
# t6 = collect("test_checking6")
# t2 = collect("test_pthread")


x = np.arange(len(t1))

plt.plot()
plt.scatter(x, t7, label="futex")
plt.scatter(x, t1, label="drepper2_1_with_trylock")
# plt.scatter(x, t2, label="drepper2")
# plt.scatter(x, t3, label="drepper3")
# plt.scatter(x, t4, label="drepper3b")
# plt.scatter(x, t5, label="gusted1")
# plt.scatter(x, t6, label="gusted2")

# plt.scatter(x, t2, label="pthread")

plt.legend()
plt.ylabel('Latency(ns)')
plt.xlabel('The i-th Experiment')
plt.show()