import matplotlib.pyplot as plt
import numpy as np
import csv

x_0 = []
x_1 = []

y_0 = []
y_1 = []

itr = 0.3

with open('analysis2.csv') as csvfile:
    readCSV = csv.reader(csvfile, delimiter=',')
    for row in readCSV:
    	if(row[3] == 'time_taken'):
    		continue
    	if(row[1] == '0'):
    		x_0.append(row[0])
    		y_0.append(float(row[3]))
    	else:
    		x_1.append(row[0])
    		y_1.append(float(row[3]))
    	itr = itr + 0.3

plt.plot(x_0,y_0, 'b')
plt.plot(x_1,y_1, 'r')
plt.legend(['Fast WMF', 'Naive WMF'], loc=0)
plt.xlabel('Scale factor')
plt.ylabel('Time Taken (in s)')
plt.title('Time Vs Image size')

plt.show()