import time
import truck_telemetry as tt

POLL_HZ = 10
DELAY = 1.0 / POLL_HZ

tt.init()

time.sleep(1.0)

first = True
try:
    while True:
        data = tt.get_data()   # <-- chiamata aggiornata ad ogni iterazione
        if not data:
            # nessun dato disponibile: il plugin potrebbe non essere caricato o non sei in gioco
            print("No telemetry data yet (plugin not loaded or not in-game).")
            time.sleep(0.5)
            continue

        # la prima volta, stampa l'intero dict per verificare i nomi delle chiavi
        if first:
            print("Telemetry keys available:", list(data.keys()))
            first = False

        # usa get con default per evitare KeyError
        gear = data.get("gear", None)
        gear_dashboard = data.get("gearDashboard", None)

        # stampa in modo leggibile
        print(f"gear: {gear}    gearDashboard: {gear_dashboard}")

        time.sleep(DELAY)

except KeyboardInterrupt:
    print("Stopped by user")