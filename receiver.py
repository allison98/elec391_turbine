import numpy as np
import pyqtgraph as pg
from pyqtgraph.Qt import QtGui, QtCore
from pyqtgraph.ptime import time
import time
import serial 
import os

port_name = "COM7"
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
datafile.write("power after, duty, load voltage, degree, current, power, voltage, load current\n")

# First row of plots
graphic.nextRow()
p1 = graphic.addPlot(title="Duty", labels={'left':'%', 'bottom':'Time'})
p2 = graphic.addPlot(title="Voltage at Load", labels={'left':'Voltage', 'bottom':'Time'})
p3 = graphic.addPlot(title="Power ", labels={'left':'Degree', 'bottom':'Time'})

# Second row of plots 
graphic.nextRow()
p4 = graphic.addPlot(title="Power After", labels={'left':'Watts', 'bottom':'Time'})
p5 = graphic.addPlot(title="Current", labels={'left':'Watts', 'bottom':'Time'})

p1.showGrid(x=None, y=True, alpha=None)
p2.showGrid(x=None, y=True, alpha=None)
p3.showGrid(x=None, y=True, alpha=None)
p4.showGrid(x=None, y=True, alpha=None)
p5.showGrid(x=None, y=True, alpha=None)

p1.setRange(yRange=[20,80])
p2.setRange(yRange=[0,17])
p3.setRange(yRange=[0,3])
p4.setRange(yRange=[0,3])
p5.setRange(yRange=[0,0.5])


curve1 = p1.plot()
curve2 = p2.plot()
curve3 = p3.plot()
curve4 = p4.plot()
curve5 = p5.plot()

windowWidth = 500                      
Xn1 = np.linspace(0,0,windowWidth)
Xn2 = np.linspace(0,0,windowWidth)
Xn3 = np.linspace(0,0,windowWidth)
Xn4 = np.linspace(0,0,windowWidth)   
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
	# Values comming in as power, duty cycle, load voltage and degree
	# String powervals = String(power_after)  + ',' + String(dutycycle)  + ',' + String(load_voltage)+ ',' + String(degree) ',' + String(read_current)  ',' + String(power) ',' + String(read_voltage) ',' + String(read_voltage);  
# Update values on graph
def update():
	global ptr, data_array, Xn1, Xn2, Xn3, Xn4, Xn5, curve1, curve2, curve3, curve4, curve5
	data_array = read_data()
	power_average.append(data_array[0])
		
	Xn1[:-1] = Xn1[1:]
	Xn2[:-1] = Xn2[1:]
	Xn3[:-1] = Xn3[1:]
	Xn4[:-1] = Xn4[1:]
	Xn5[:-1] = Xn5[1:]

	value1 =  data_array[1] # duty
	value2 = data_array[2] # load volt
	value3 = data_array[5] # direction
	value4 = data_array[0] # power
	value5 = data_array[4] 
	#avgPower = sum(power_average/len(power_average))

	try: 
		Xn1[-1] = float(value1)
		Xn2[-1] = float(value2)
		Xn3[-1] = float(value3)
		Xn4[-1] = float(value4)
		Xn5[-1] = float(value5)
	except ValueError:
		pass

	ptr += 1
	curve1.setData(Xn1) 
	curve2.setData(Xn2)
	curve3.setData(Xn3)
	curve4.setData(Xn4)
	curve5.setData(Xn5)

	curve1.setPos(ptr,0)
	curve2.setPos(ptr,0)
	curve3.setPos(ptr,0)
	curve4.setPos(ptr,0)
	curve5.setPos(ptr,0)
	QtGui.QApplication.processEvents()

timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(0)

def print_values():
	
	print("Current Before:", str.format('{0:.3f}', data_array[4]))
	print("Voltage Before:", str.format('{0:.3f}',data_array[6]))
	print("High Enough: ", str.format('{0:.3f}',data_array[7]))
	print("Voltage After: ", str.format('{0:.3f}',data_array[2]))
	print("Power After: ", str.format('{0:.3f}',data_array[0]))
	print("Power Before:", str.format('{0:.3f}',data_array[5]))
	print("Duty: ",data_array[1])
	print("Prev Load V: ",data_array[3])	
	#os.system('cls')
	#cp = data_array[0]/(0.5*1.225*A*V^3)
	#print("CP: %f\n", cp)
	
	
# Main program: executes the update function and updates the graph

while True: 
	update()
	print_values()
pg.QtGui.QApplication.exec_()


