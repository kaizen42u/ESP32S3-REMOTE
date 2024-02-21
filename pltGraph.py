from tkinter import Tk
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

matplotlib.use("Agg")


class pltGraph:
    def __init__(
        self,
        root: Tk,
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
        self.figure.clear()
        ax = self.figure.add_subplot(111)
        ax.plot(self.times, self.data)
        ax.set_title(self.title)
        if self.do_ylim:
            ax.set_ylim(self.low_ylim, self.high_ylim)
        ax.grid()
        self.canvas.draw()
