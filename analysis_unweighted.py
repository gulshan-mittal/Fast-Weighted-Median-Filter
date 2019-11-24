import matplotlib.pyplot as plt
import numpy as np
import csv

x_0 = []
x_1 = []

y_0 = []
y_1 = []

wt_type = 0

with open('analysis1.csv') as csvfile:
	readCSV = csv.reader(csvfile, delimiter=',')
	for row in readCSV:
		if(row[3] == 'time_taken'):
			continue
		if(row[1] == '0'):
			x_0.append(float(row[0]))
			y_0.append(float(row[3]))
		else:
			x_1.append(float(row[0]))
			y_1.append(float(row[3]))
		wt_type = row[2]

plt.plot(x_0,y_0, 'b')
plt.plot(x_1,y_1, 'r')
plt.legend(['Fast WMF', 'Naive WMF'], loc=0)
plt.xlabel('Kernel Size')
plt.ylabel('Time Taken (in s)')
plt.title('Time Vs Kernel Size')
plt.show()