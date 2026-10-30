import pandas as pd
from pathlib import Path 

DATA_PATH = Path(__file__).resolve().parent.parent / "data" / "drug_info.csv"
df = pd.read_csv(DATA_PATH)

print("=== Full table ===")
print(df[["name", "drug_class", "used_for"]].to_string(index=False))

print("\n=== Drugs per body system ===")
print(df["body_system"].value_counts().to_string())

print("\n=== Drugs grouped by body system ===")
for system, group in df.groupby("body_system"):
    print(f"{system}: {', '.join(group['name'])}")

print("\n=== Used in children ===")
kids = df[df["typical_patients"].str.contains("children")]
print(kids[["name", "typical_patients"]].to_string(index=False))

print("\n=== Pregnancy caution ===")
preg = df[df["key_caution"].str.contains("pregnancy")]
print(preg[["name", "drug_class", "key_caution"]].to_string(index=False))