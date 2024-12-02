import matplotlib.pyplot as plt

# Data for Trivial Parallel
n_trivial = [29, 52, 76, 100, 131]
t_trivial = [12.5, 41.2, 380, 1650, 8200]

# Data for Hybrid Parallel
n_hybrid = [29, 52, 76, 100, 131]
t_hybrid = [13.93, 44.6, 221, 840, 3527]

# Plotting the data
plt.figure(figsize=(8, 6))

# Plot Trivial Parallel
plt.plot(n_trivial, t_trivial, marker='o', label='Trivial Parallel')

# Plot Hybrid Parallel
plt.plot(n_hybrid, t_hybrid, marker='s', label='Hybrid Parallel')

# Adding labels, legend, and title
plt.xlabel('Number of Cities (N)', fontsize=12)
plt.ylabel('Time (T) in seconds', fontsize=12)
plt.title('Performance Comparison: Trivial vs Hybrid Parallel', fontsize=14)
plt.legend()
plt.grid(True)

# Display the plot
plt.show()
