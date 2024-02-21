import re
from time import sleep
import tkinter as tk
import serial.tools.list_ports
import serial
import threading
from pltGraph import pltGraph

from tkAnsiFormatter import tkAnsiFormatter

import matplotlib
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

matplotlib.use("Agg")


class SerialApp:
    def __init__(self, root: tk.Tk) -> None:
        self.root = root
        self.root.title("Serial Port Opener")
        self.root.geometry("1280x720")
        self.serial_port = None
        self.killed = False
        self.auto_scroll = tk.BooleanVar(value=True)

        # Get a list of all available serial ports
        ports = self.get_ports()

        # Create a dropdown menu for available ports
        self.port_var = tk.StringVar()
        if ports:
            self.port_var.set(ports[0])  # Preselect the first available port
        self.port_dropdown = tk.OptionMenu(root, self.port_var, "", *ports)
        self.port_dropdown.config(width=20)
        self.port_dropdown.grid(row=0, column=0)

        # Create connect/disconnect button
        self.conn_button = tk.Button(root, text="Connect", command=self.connect_toggle)
        self.conn_button.config(width=20)
        self.conn_button.grid(row=0, column=1)

        # Create auto scroll checkbox
        self.scroll_check = tk.Checkbutton(
            root, text="Auto Scroll", variable=self.auto_scroll
        )
        self.scroll_check.config(width=20)
        self.scroll_check.grid(row=0, column=2)

        # Create a text widget for the terminal
        self.terminal = tk.Text(root, width=180)
        self.terminal.grid(row=1, column=0, columnspan=3)

        self.formatter = tkAnsiFormatter(text=self.terminal)

        # Create figures and a canvas to draw on
        self.lspd_figure = pltGraph(root=root, title="Left Motor Velocity")
        self.lspd_figure.grid(row=2, column=0)
        self.lspd_figure.set_ylim(-10, 60)

        self.rspd_figure = pltGraph(root=root, title="Right Motor Velocity")
        self.rspd_figure.grid(row=2, column=1)
        self.rspd_figure.set_ylim(-10, 60)

        self.delta_figure = pltGraph(root=root, title="Delta Error")
        self.delta_figure.grid(row=2, column=2)

        # Create a thread to draw figures
        threading.Thread(target=self.draw_graphs).start()

    def get_ports(self) -> list[str]:
        # Get a list of all available serial ports
        ports = serial.tools.list_ports.comports()
        return [port.device for port in ports]

    def close(self) -> None:
        self.killed = True
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()

    def connect_toggle(self) -> None:
        # If already connected, disconnect
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
            self.conn_button.config(text="Connect")
            return

        # Otherwise, try to connect
        self.reset_graphs()
        try:
            self.serial_port = serial.Serial(self.port_var.get())
            self.conn_button.config(text="Disconnect")
            threading.Thread(target=self.read_from_port).start()
        except serial.SerialException as e:
            print(f"Could not open port {self.port_var.get()}: {e}")

    def update_graphs(self, reading: str) -> None:
        escaped = self.formatter.escaped(reading)
        match = re.search(
            r"I \((\d+)\).*Lspd:( *[-\d.]+).*Rspd:( *[-\d.]+).*Δd:( *[-\d.]+)", escaped
        )
        if match:
            time, lspd, rspd, errdis = map(float, match.groups())
            self.lspd_figure.append(time, lspd)
            self.rspd_figure.append(time, rspd)
            self.delta_figure.append(time, errdis)

    def reset_graphs(self) -> None:
        self.lspd_figure.reset()
        self.rspd_figure.reset()
        self.delta_figure.reset()

    def draw_graphs(self) -> None:
        while True:
            if self.killed:
                return
            # Update the graph
            self.lspd_figure.draw()
            self.rspd_figure.draw()
            self.delta_figure.draw()
            sleep(0.05)

    def read_from_port(self) -> None:
        while self.serial_port:
            if self.killed:
                return
            while self.serial_port.is_open:
                line = self.serial_port.readline()
                if not line:
                    break
                reading = line.decode("utf-8")
                self.update_graphs(reading)

                self.formatter.insert_ansi(reading, "end")
                if self.auto_scroll.get():
                    self.terminal.see(tk.END)
            sleep(0.1)


if __name__ == "__main__":

    root = tk.Tk()
    app = SerialApp(root)
    root.mainloop()
    app.close()
