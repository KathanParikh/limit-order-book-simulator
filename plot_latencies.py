import matplotlib.pyplot as plt
import pandas as pd
import sys

try:
    # Load the CSV
    data = pd.read_csv('latencies.csv')
    
    # Plotting
    plt.figure(figsize=(10, 6))
    plt.plot(data['Latency_Microseconds'], label='Processing Latency')
    
    # Add a moving average (trend line) to see performance stability
    plt.plot(data['Latency_Microseconds'].rolling(window=50).mean(), 
             label='50-Order Moving Avg', color='red', linewidth=2)

    plt.title('Engine Latency Performance')
    plt.xlabel('Order Sequence Number')
    plt.ylabel('Latency (microseconds)')
    plt.legend()
    plt.grid(True)
    
    # Save and Show
    plt.savefig('latency_plot.png')
    print("Plot saved as latency_plot.png")
    plt.show()

except Exception as e:
    print(f"Error plotting: {e}")
    print("Make sure you have pandas and matplotlib installed: pip install pandas matplotlib")