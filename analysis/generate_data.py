import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import os

rng = np.random.default_rng(42)
MINUTES = 1440  # one day, one reading per minute

def baseline(n):
    """Compressor cycling: slow wave around 5°C plus sensor noise."""
    t = np.arange(n)
    cycle = 1.2 * np.sin(2 * np.pi * t / 40)  # 40-minute cycle
    noise = rng.normal(0, 0.1, n)
    return 5.0 + cycle + noise

def add_door_open(temp, labels, start, duration=5, rise=4.0, recovery=15):
    """Temp climbs while the door is open, then falls back."""
    for i in range(duration + recovery):
        idx = start + i
        if idx >= len(temp):
            break
        if i < duration:
            temp[idx] += rise * (i + 1) / duration
        else:
            temp[idx] += rise * (1 - (i - duration + 1) / recovery)
        labels[idx] = "door_open"

def add_compressor_failure(temp, labels, start, duration=120, peak=7.0):
    """Slow upward drift as the compressor dies."""
    for i in range(duration):
        idx = start + i
        if idx >= len(temp):
            break
        temp[idx] += peak * (i + 1) / duration
        labels[idx] = "compressor_failure"

def add_freeze(temp, labels, start, duration=30, drop=4.5):
    """Temperature pushed below the safe range."""
    for i in range(duration):
        idx = start + i
        if idx >= len(temp):
            break
        temp[idx] -= drop
        labels[idx] = "freeze"

temp = baseline(MINUTES)
labels = np.array(["normal"] * MINUTES, dtype=object)

add_door_open(temp, labels, start=200)
add_freeze(temp, labels, start=500)
add_door_open(temp, labels, start=700, duration=3)  # a brief one
add_compressor_failure(temp, labels, start=950)

status = np.where(temp > 8, "TOO_HOT", np.where(temp < 2, "TOO_COLD", "OK"))

df = pd.DataFrame({
    "minute": np.arange(MINUTES),
    "temp_c": temp.round(2),
    "status": status,
    "event_label": labels,
})

os.makedirs("data", exist_ok=True)
os.makedirs("images", exist_ok=True)
df.to_csv("data/simulated_day.csv", index=False)

# quick look at what happened during each event
print(df.groupby("event_label")["status"].value_counts())

# plot with the safe band shaded
plt.figure(figsize=(12, 4))
plt.plot(df["minute"], df["temp_c"], linewidth=0.8)
plt.axhspan(2, 8, alpha=0.15, color="green", label="safe range (2-8°C)")
plt.xlabel("minute")
plt.ylabel("temp (°C)")
plt.legend()
plt.tight_layout()
plt.savefig("images/simulated_day.png", dpi=150)