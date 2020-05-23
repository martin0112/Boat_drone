# -*- coding: utf-8 -*-
"""
Created on Sun May 17 16:05:29 2020

@author: Martin
"""

import pandas as pd
import matplotlib.pyplot as plt


data_boat = pd.read_csv("170520.csv",sep=";")



plt.figure(0)
plt.plot(data_boat.heading_sp,"--")
plt.plot(data_boat.heading,"-")

plt.figure(1)
plt.plot(data_boat.error,data_boat.cv,"x")

plt.figure(2)
plt.plot(data_boat.heading_sp-data_boat.heading,"x")