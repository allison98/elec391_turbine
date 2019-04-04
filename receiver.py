import numpy as np
import pyqtgraph as pg
from pyqtgraph.Qt import QtGui, QtCore
from pyqtgraph.ptime import time
import time
import serial 
import os

port_name = "COM5"
baudrate = 9600
ser = serial.Serial(port_name,baudrate)

# Initialization
app = QtGui.QApplication([])
graphic = pg.GraphicsLayout()
view = pg.GraphicsView()
view.resize(800,500)
view.setCentralItem(graphic)
view.show()
view.setWindowTitle('Live plots of turbine data')

graphic.addLabel('Raw signal', colspan=4)

timestring = time.strftime("%Y%m%d-%H%M%S")

datafile = open( timestring + '.txt', "w+")
datafile.write("power after, duty, load voltage, prev load voltage, current, power, voltage, position, rps fan, rps turbine \n")

# First row of plots
graphic.nextRow()
p1 = graphic.addPlot(title="Duty", labels={'left':'%', 'bottom':'Samples'})
p2 = graphic.addPlot(title="Voltage at Load", labels={'left':'Voltage', 'bottom':'Samples'})
p5 = graphic.addPlot(title="Current", labels={'left':'Amps', 'bottom':'Samples'})

# Second row of plots 
graphic.nextRow()
p3 = graphic.addPlot(title="Power ", labels={'left':'Watts', 'bottom':'Samples'}, colspan = 2)

p1.showGrid(x=None, y=True, alpha=None)
p2.showGrid(x=None, y=True, alpha=None)
p3.showGrid(x=None, y=True, alpha=None)
p5.showGrid(x=None, y=True, alpha=None)

p1.setRange(yRange=[20,80])
p2.setRange(yRange=[0,18])
p3.setRange(yRange=[0,4])
p5.setRange(yRange=[0,0.8])


curve1 = p1.plot()
curve2 = p2.plot()
curve3 = p3.plot()
curve5 = p5.plot()

windowWidth = 500                      
Xn1 = np.linspace(0,0,windowWidth)
Xn2 = np.linspace(0,0,windowWidth)
Xn3 = np.linspace(0,0,windowWidth)
Xn5 = np.linspace(0,0,windowWidth)   

ptr = -windowWidth
data_array = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]

power_average = []

# Reads string from serial and converts it to list of ints
def read_data():
	data = ser.readline().decode()
	while data.isspace(): # if faulty reading (whitespace), keep trying
		data = ser.readline().decode()
	datafile.write(data)
	return list(map(float, data.split(",")))

# Update values on graph
def update():
	global ptr, data_array, Xn1, Xn2, Xn3, Xn4, Xn5, curve1, curve2, curve3, curve4, curve5
	data_array = read_data()
	power_average.append(data_array[0])
		
	Xn1[:-1] = Xn1[1:]
	Xn2[:-1] = Xn2[1:]
	Xn3[:-1] = Xn3[1:]
	Xn5[:-1] = Xn5[1:]

	value1 =  data_array[1] 
	value2 = data_array[2] 
	value3 = data_array[5] 
	value5 = data_array[4] 

	try: 
		Xn1[-1] = float(value1)
		Xn2[-1] = float(value2)
		Xn3[-1] = float(value3)
		Xn5[-1] = float(value5)
	except ValueError:
		pass

	ptr += 1
	curve1.setData(Xn1) 
	curve2.setData(Xn2)
	curve3.setData(Xn3)
	curve5.setData(Xn5)

	curve1.setPos(ptr,0)
	curve2.setPos(ptr,0)
	curve3.setPos(ptr,0)
	curve5.setPos(ptr,0)
	QtGui.QApplication.processEvents()

timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(0)

def print_values():
	print("Current Before:", str.format('{0:.3f}', data_array[4]))
	print("Voltage Before:", str.format('{0:.3f}',data_array[6]))
	print("Direction: ", str.format('{0:.3f}',data_array[7]))
	print("Voltage After: ", str.format('{0:.3f}',data_array[2]))
	print("Power After: ", str.format('{0:.3f}',data_array[0]))
	print("Power Before:", str.format('{0:.3f}',data_array[5]))
	print("Duty: ",data_array[1])
	speed_v = 3.626 * data_array[8] + 4.8
	print("Wind Speed: ", speed_v)
	speed_t = data_array[9] * 60* 0.10472 * 0.2
	print("RPM of Turbine:", str.format('{0:.3f}',data_array[9]*60))
	print("Speed of Turbine: ",str.format('{0:.3f}',speed_t))	
	cp = data_array[0]/(0.5*1.225*3.14*0.2**2*speed_v**3)
	print("CP: ", str.format('{0:.3f}',cp))
	print("Lambda:", str.format('{0:.3f}',speed_t/speed_v))
	
# Main program: executes the update function and updates the graph and calculates variables
while True: 
	update()
	print_values()
pg.QtGui.QApplication.exec_()


