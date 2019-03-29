import numpy as np
import pyqtgraph as pg
from pyqtgraph.Qt import QtGui, QtCore
from pyqtgraph.ptime import time
import time
import serial 

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

# First row of plots
graphic.nextRow()
p1 = graphic.addPlot(title="Degree", labels={'left':'Degrees', 'bottom':'Time'})
p2 = graphic.addPlot(title="Duty", labels={'left':'%', 'bottom':'Time'})
p3 = graphic.addPlot(title="Voltage", labels={'left':'Voltage', 'bottom':'Time'})

# Second row of plots 
graphic.nextRow()
p4 = graphic.addPlot(title="Power", labels={'left':'Watts', 'bottom':'Time'}, colspan = 2)
p5 = graphic.addPlot(title="Running Average", labels={'left':'Watts', 'bottom':'Time'}, colspan = 2)


p1.showGrid(x=None, y=True, alpha=None)
p2.showGrid(x=None, y=True, alpha=None)
p3.showGrid(x=None, y=True, alpha=None)
p4.showGrid(x=None, y=True, alpha=None)
p5.showGrid(x=None, y=True, alpha=None)

curve1 = p1.plot()
curve2 = p2.plot()
curve3 = p3.plot()
curve4 = p4.plot()
curve5 = p5.plot()

windowWidth = 100                      
Xm1 = np.linspace(0,0,windowWidth)
Xm2 = np.linspace(0,0,windowWidth)
Xm3 = np.linspace(0,0,windowWidth)
Xm4 = np.linspace(0,0,windowWidth)   
Xm5 = np.linspace(0,0,windowWidth)   

ptr = -windowWidth
data_array = [0.0, 0.0, 0.0, 0.0]

power_average = []

# Reads string from serial and converts it to list of ints
def read_data():
	data = ser.readline().decode()
	while data.isspace(): # if faulty reading (whitespace), keep trying
		data = ser.readline().decode()
	return list(map(float, data.split(",")))
	# Values comming in as power, duty cycle, load voltage and degree

# Update values on graph
def update():
	global curve1, curve2, curve3, curve3, ptr, Xm1, Xm2, Xm3, Xm4

	data_array = read_data()
	power_average.append(data_array[0])

	Xm1[:-1] = Xm1[1:]
	Xm2[:-1] = Xm2[1:]
	Xm3[:-1] = Xm3[1:]
	Xm4[:-1] = Xm4[1:]

	value1 = sum(power_average)/len(power_average) # data_array[0]
	value2 = data_array[1]
	value3 = data_array[2]
	value4 = data_array[3]
	#avgPower = sum(power_average/len(power_average))

	try: 
		Xm1[-1] = float(value1)
		Xm2[-1] = float(value2)
		Xm3[-1] = float(value3)
		Xm4[-1] = float(value4)
	except ValueError:
		pass

	ptr += 1
	curve1.setData(Xm1)
	curve2.setData(Xm2)
	curve3.setData(Xm3)
	curve4.setData(Xm4)

	curve1.setPos(ptr,0)
	curve2.setPos(ptr,0)
	curve3.setPos(ptr,0)
	curve4.setPos(ptr,0)
	QtGui.QApplication.processEvents()

timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(0)

# Main program: executes the update function and updates the graph
while True: update()
pg.QtGui.QApplication.exec_()