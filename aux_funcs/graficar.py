import matplotlib.pyplot as plt

with open("genetico_paralelo_funcion_76.txt", 'r') as file:
    numbers = [float(line.strip()) for line in file.readlines()]

# Generate x-axis values
iterations = list(range(1, len(numbers) + 1))

#numbers=numbers[0:150000]
#iterations=iterations[0:150000]
# Plot the data
plt.figure(figsize=(10, 5))
plt.plot(iterations, numbers, marker='o', linestyle='-', label='Values')
plt.xlabel('Iteration')
plt.ylabel('TSP Value   ')
plt.title('Iterations vs Optimal')
plt.legend()
plt.grid(True)
plt.show()