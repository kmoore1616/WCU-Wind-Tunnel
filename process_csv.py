
import pandas as pd
import sys
import os

def process_file(file_path):
    # Read CSV with no headers
    df = pd.read_csv(file_path, header=None)

    # Assign known column names
    column_names = [
        'Time', 'P1', 'P2', 'P3', 'p4', 'V_AS0(P5)', 'P6', 'P7', 'P8', 'P9',
        'V_AS1(P10)', 'FRQ', 'LoadCell_0', 'LoadCell_1', 'LoadCell_2',
        'LC0', 'LC1', 'LC2', 'Velocity', 'Tare'
    ]
    df.columns = column_names

    # KEEP time as raw float (no conversion)

    # Process in 100-sample blocks
    samples_per_block = 100
    num_blocks = len(df) // samples_per_block
    df_trimmed = df.iloc[:num_blocks * samples_per_block]
    grouped = df_trimmed.groupby(df_trimmed.index // samples_per_block)

    # Calculate means for raw data
    raw_cols = ['P1', 'P2', 'P3', 'p4', 'V_AS0(P5)', 'P6', 'P7', 'P8', 'P9',
                'V_AS1(P10)', 'FRQ', 'LoadCell_0', 'LoadCell_1', 'LoadCell_2']
    raw_means = grouped[raw_cols].mean()

    # Get first available processed values
    processed_cols = ['LC0', 'LC1', 'LC2', 'Velocity', 'Tare']
    processed_vals = grouped[processed_cols].first()

    # First raw float timestamp in each block
    timestamps = grouped['Time'].first()

    # Combine everything
    result_df = pd.concat([timestamps, raw_means, processed_vals], axis=1)
    result_df.reset_index(drop=True, inplace=True)

    # Write output CSV
    out_file = os.path.splitext(file_path)[0] + "_processed.csv"
    result_df.to_csv(out_file, index=False)
    print(f"Processed file saved to: {out_file}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Drag and drop a CSV file onto this script.")
    else:
        process_file(sys.argv[1])
