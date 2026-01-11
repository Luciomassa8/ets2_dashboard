#!/usr/bin/env python3
import time
import serial
import truck_telemetry as tt
import xml.etree.ElementTree as ET

tree = ET.parse("config.xml")
root = tree.getroot()

port = root.find("port").text
baud = int(root.find("baud").text)

# SERIAL_PORT = 'COM9'
# BAUD = 115200
SERIAL_PORT = port
BAUD = baud
POLL_HZ = 10
RECONNECT_DELAY = 2.0
SEND_ON_CHANGE_ONLY = False

DELAY = 1.0 / POLL_HZ

def open_serial(port, baud):
    try:
        ser = serial.Serial(port, baud, timeout=0.5)
        # evita reset automatico su alcune board
        try:
            ser.setDTR(False)
        except Exception:
            pass
        time.sleep(0.05)
        return ser
    except Exception as e:
        print(f"[serial] Impossibile aprire {port} : {e}")
        return None

def format_line(rpm, gear, gear_dashboard, speed, truckBrand, truckName, engineRpmMax, fuel, gears, gears_reverse):
    rpm_v = int(rpm or 0)
    gear_v = int(gear or 0)
    gear_dash_v = int(gear_dashboard or gear_v)
    speed_v = float(speed or 0.0)
    truckBrand_v = str(truckBrand)
    truckName_v = str(truckName)
    engineRpmMax_v = int(engineRpmMax)
    fuel_v = float(fuel)
    gears_v = int(gears)
    gears_reverse_v = int(gears_reverse)
    return f"{rpm_v},{gear_v},{gear_dash_v},{speed_v:.1f},{truckBrand_v},{truckName_v},{engineRpmMax_v},{fuel_v:.1f},{gears_v},{gears_reverse_v},\n"

def main():
    try:
        tt.init()
    except Exception as e:
        print(f"[telemetry] Errore init wrapper: {e}")

    ser = None
    last_sent = None

    try:
        while True:
            if ser is None or not ser.is_open:
                ser = open_serial(SERIAL_PORT, BAUD)
                if ser is None:
                    time.sleep(RECONNECT_DELAY)
                    continue
                else:
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

            line = format_line(rpm, gear, gear_dashboard, speed, truckBrand, truckName, engineRpmMax, fuel, gears, gears_reverse)

            if SEND_ON_CHANGE_ONLY:
                if line != last_sent:
                    try:
                        ser.write(line.encode('ascii'))
                        ser.flush()
                        last_sent = line
                        print(f"[send] {line.strip()}")
                    except Exception as e:
                        print(f"[serial] Errore scrittura: {e}")
                        ser.close(); ser = None
            else:
                try:
                    ser.write(line.encode('ascii'))
                    ser.flush()
                    last_sent = line
                    print(f"[send] {line.strip()}")
                except Exception as e:
                    print(f"[serial] Errore scrittura: {e}")
                    try: ser.close()
                    except: pass
                    ser = None

            time.sleep(DELAY)

    except KeyboardInterrupt:
        print("Interrotto dall'utente.")
    finally:
        if ser:
            try: ser.close()
            except: pass
        print("Chiusura script.")

if __name__ == '__main__':
    main()