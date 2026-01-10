import serial
import serial.tools.list_ports

ports = list(serial.tools.list_ports.comports())
print("Available ports:")
for p in ports:
    print(f"  {p.device} - {p.description}")

test_port = ports[0].device if ports else None
if not test_port:
    print("No serial ports found.")
else:
    try:
        ser = serial.Serial(test_port, 115200, timeout=1)
        print(f"Opened {test_port} OK")
        ser.close()
    except Exception as e:
        print(f"Could not open {test_port}: {e}")
