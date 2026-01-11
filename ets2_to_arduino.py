#!/usr/bin/env python3
import time, serial, threading, pystray
import truck_telemetry as tt
import xml.etree.ElementTree as ET
from PIL import Image

# ------------------------------------------------------------
# Reading the XML configuration
# ------------------------------------------------------------
tree = ET.parse("config.xml")
root = tree.getroot()

SERIAL_PORT = root.find("port").text
BAUD = int(root.find("baud").text)

POLL_HZ = 10
RECONNECT_DELAY = 2.0
SEND_ON_CHANGE_ONLY = False
DELAY = 1.0 / POLL_HZ

running = True  # controlled by tray icon

# ------------------------------------------------------------
# Serial port opening
# ------------------------------------------------------------
def open_serial(port, baud):
    try:
        ser = serial.Serial(port, baud, timeout=0.5)
        try:
            ser.setDTR(False)
        except Exception:
            pass
        time.sleep(0.05)
        return ser
    except Exception as e:
        print(f"[serial] Impossibile aprire {port} : {e}")
        return None

# ------------------------------------------------------------
# CSV line preparation
# ------------------------------------------------------------
def format_line(rpm, gear, gear_dashboard, speed_kmh,
                truckBrand, truckName, engineRpmMax,
                fuel, gears, gears_reverse):

    return (
        f"{int(rpm or 0)},"
        f"{int(gear or 0)},"
        f"{int(gear_dashboard or gear)},"
        f"{float(speed_kmh):.1f},"
        f"{truckBrand},"
        f"{truckName},"
        f"{int(engineRpmMax)},"
        f"{float(fuel):.1f},"
        f"{int(gears)},"
        f"{int(gears_reverse)},\n"
    )

# ------------------------------------------------------------
# MAIN LOOP
# ------------------------------------------------------------
def main():
    global running

    try:
        tt.init()
    except Exception as e:
        print(f"[telemetry] Errore init wrapper: {e}")

    ser = None
    last_sent = None

    try:
        while running:
            if ser is None or not ser.is_open:
                ser = open_serial(SERIAL_PORT, BAUD)
                if ser is None:
                    time.sleep(RECONNECT_DELAY)
                    continue
                print(f"[serial] Connesso a {SERIAL_PORT} @ {BAUD} baud")

            data = tt.get_data()
            if not data:
                time.sleep(0.5)
                continue

            rpm = data.get('engineRpm', 0)
            gear = data.get('gear', 0)
            gear_dashboard = data.get('gearDashboard', gear)
            speed = data.get('speed', 0.0)
            truckName = data.get('truckName', "")
            truckBrand = data.get('truckBrand', "")
            engineRpmMax = data.get('engineRpmMax', 0)
            fuel = data.get('fuel', 0.0)
            gears = data.get('gears', 0)
            gears_reverse = data.get('gears_reverse', 0)

            line = format_line(
                rpm, gear, gear_dashboard, speed,
                truckBrand, truckName, engineRpmMax,
                fuel, gears, gears_reverse
            )

            try:
                if SEND_ON_CHANGE_ONLY:
                    if line != last_sent:
                        ser.write(line.encode('ascii'))
                        ser.flush()
                        last_sent = line
                        print(f"[send] {line.strip()}")
                else:
                    ser.write(line.encode('ascii'))
                    ser.flush()
                    last_sent = line
                    print(f"[send] {line.strip()}")

            except Exception as e:
                print(f"[serial] Errore scrittura: {e}")
                try:
                    ser.close()
                except:
                    pass
                ser = None

            time.sleep(DELAY)

    except KeyboardInterrupt:
        running = False

    finally:
        if ser:
            try:
                ser.close()
            except:
                pass
        print("Chiusura script.")

# ------------------------------------------------------------
if __name__ == '__main__':
    main()
