from tkinter import Misc, Tk
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

matplotlib.use("Agg")


class pltGraph:
    def __init__(
        self,
        root: Misc,
        figsize: tuple[int, int] = (5, 4),
        dpi: int = 80,
        timespan: int = 5000,
        title: str = "Graph",
    ) -> None:

        # Create a figure and a canvas to draw on
        self.figure = plt.figure(figsize=figsize, dpi=dpi)
        self.canvas = FigureCanvasTkAgg(self.figure, master=root)
        self.timespan = timespan
        self.title = title

        self.data = []
        self.times = []
        self.do_ylim = False

        # Configure Axes object
        self.ax = self.figure.add_subplot(111)
        self.ax.set_title(self.title)
        if self.do_ylim:
            self.ax.set_ylim(self.low_ylim, self.high_ylim)
        self.ax.grid()
        self.line, = self.ax.plot(self.times, self.data)

    # Partial function of tk.grid()
    def grid(self, row: int = 2, column: int = 0) -> None:
        self.canvas.get_tk_widget().grid(row=row, column=column)

    # Clears graph data
    def reset(self) -> None:
        self.data.clear()
        self.times.clear()

    # Appends timestamp and data to the list, also clears old data
    def append(self, time, data) -> None:
        self.data.append(data)
        self.times.append(time)

        # Remove data older than x milliseconds
        while self.times and self.times[0] < time - self.timespan:
            self.times.pop(0)
            self.data.pop(0)

    # Set graph y-axis limit, default is automatic
    def set_ylim(self, low: float, high: float):
        self.do_ylim = True
        self.low_ylim = low
        self.high_ylim = high

    # Draw graph on canvas
    def draw(self) -> None:
        # Update the graph
        self.line.set_data(self.times, self.data)

        # Rescale the x-axis to fit the new data
        if len(self.times) >= 2:
            self.ax.set_xlim(self.times[0], self.times[-1])
        
        # Rescale the y-axis to fit the new data
        if self.do_ylim:
            self.ax.set_ylim(self.low_ylim, self.high_ylim)
        else:
            self.ax.relim()
            self.ax.autoscale_view()

        self.canvas.draw()
