import pandas as pd

RAW_PATH = "data/drug_names.csv"
CLEAN_PATH = "data/drug_names_clean.csv"

df = pd.read_csv(RAW_PATH)
total_in = len(df)

names = (
    df["name"]
    .astype(str)
    .str.strip()                           # trim whitespace
    .str.lower()                           # case-insensitive comparison
    .str.replace(r"\s+", " ", regex=True)  # collapse repeated spaces
)

names = names[names != ""]                 # drop empty rows
names = names.drop_duplicates().sort_values().reset_index(drop=True)

total_out = len(names)

names.to_frame(name="name").to_csv(CLEAN_PATH, index=False)

print(f"names in:      {total_in}")
print(f"names out:     {total_out}")
print(f"removed:       {total_in - total_out} (duplicates or empties)")
print(f"saved to:      {CLEAN_PATH}")