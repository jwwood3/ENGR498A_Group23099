import pyudev
import subprocess
import os
import autoProcess
def main():
	context = pyudev.Context()
	monitor = pyudev.Monitor.from_netlink(context)
	monitor.filter_by(subsystem='usb')
	monitor.start()
	for device in iter(monitor.poll, None):
		print("New drive added")
		while not os.path.exists('/media/usb/display'):
			""
		autoProcess.processAll('/media/usb/display');
if __name__ == '__main__':
	main()

