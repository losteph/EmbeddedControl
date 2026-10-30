import matplotlib.pyplot as plt
import pandas as pd


df = pd.read_csv("perGrafico.csv", header=None)

# ha letto tutto in un'unica colonna, separiamo noi con ';'
if df.shape[1] == 1:
  df = df[0].str.split(";", expand=True)

# Estraiamo setpoint e distance dalle prime due colonne
setpoint = pd.to_numeric(df[0], errors="coerce")
distance = pd.to_numeric(df[1], errors="coerce")

# sse temporale (26.6 ms = 0.0266 s, i nostri 37Hz)
dt = 0.0266
time = [i * dt for i in range(len(df))]

# Plot finale
plt.figure(figsize=(12, 6))
plt.plot(time, distance, label="Distance", color="blue", linewidth=1.5)
plt.plot(time, setpoint, label="Setpoint", color="red", linestyle="--", linewidth=2)

plt.xlabel("Tempo (s)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()