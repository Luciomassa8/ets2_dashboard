import truck_telemetry as tt
tt.init()
data = tt.get_data()
print(sorted(data.keys()))
