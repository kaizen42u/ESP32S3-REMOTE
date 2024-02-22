import re
from time import sleep
import tkinter as tk
import serial.tools.list_ports
import serial
import threading
from ansiEncoding import ANSI
from pltGraph import pltGraph
from tkinter import Misc, ttk

from tkAnsiFormatter import tkAnsiFormatter

import matplotlib

matplotlib.use("Agg")


class SerialApp:
    def __init__(self, root: Misc) -> None:
        self.root = root
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

        # Create a ANSI escape sequence decoder and parser for the terminal
        self.formatter = tkAnsiFormatter(text=self.terminal)

        # Create figures and a canvas to draw on
        self.lspd_figure = pltGraph(root=root, title="Left Motor Velocity")
        self.lspd_figure.grid(row=2, column=0)
        self.lspd_figure.set_ylim(-2, 15)

        self.rspd_figure = pltGraph(root=root, title="Right Motor Velocity")
        self.rspd_figure.grid(row=2, column=1)
        self.rspd_figure.set_ylim(-2, 15)

        self.delta_figure = pltGraph(root=root, title="Delta Error")
        self.delta_figure.grid(row=2, column=2)
        # self.delta_figure.set_ylim(-10, 10)

        # Create threads to draw figures and serial port reading
        self.draw_graphs_thread = threading.Thread(target=self.draw_graphs)
        self.draw_graphs_thread.start()
        self.read_serial_thread = threading.Thread(target=self.read_from_port)
        self.read_serial_thread.start()

    def get_ports(self) -> list[str]:
        # Get a list of all available serial ports
        ports = serial.tools.list_ports.comports()
        return [port.device for port in ports]

    def close(self) -> None:
        # Flag the process as dead and close serial port
        self.killed = True
        self.draw_graphs_thread.join()
        self.read_serial_thread.join()

    def connect_toggle(self) -> None:
        # If already connected, disconnect
        if self.serial_port and self.serial_port.is_open:
            self.disconnect_serial()
            return

        # Otherwise, try to connect
        self.reset_graphs()
        try:
            self.connect_serial()

        except serial.SerialException as e:
            print(f"Could not open port {self.port_var.get()}: {e}")

    def connect_serial(self):
        self.serial_port = serial.Serial(self.port_var.get())
        self.conn_button.config(text="Disconnect")
        self.write_terminal(
            f"{ANSI.bBrightMagenta} Port [{self.port_var.get()}] Connected{ANSI.default}\n"
        )

    def disconnect_serial(self):
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
            self.conn_button.config(text="Connect")
            self.write_terminal(
                f"{ANSI.bBrightMagenta} Port [{self.port_var.get()}] Disconnected{ANSI.default}\n"
            )

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
                print("Graphing thread exited")
                return
            sleep(0.05)

            # Update graph
            try:
                self.lspd_figure.draw()
                self.rspd_figure.draw()
                self.delta_figure.draw()
            except RuntimeError:
                pass

    def read_from_port(self) -> None:
        while True:
            if self.killed:
                self.disconnect_serial()
                print("Serial Port thread exited")
                return
            sleep(0.1)

            # Reads serial port data by line
            while self.serial_port and self.serial_port.is_open:
                try:
                    line = self.serial_port.readline()

                except serial.SerialException as serr:
                    self.disconnect_serial()
                    print(f"Could not read port [{self.port_var.get()}]: {serr}")

                except TypeError as terr:
                    print(f"Bad serial data for port [{self.port_var.get()}]: {terr}")

                if not line:
                    break
                reading = line.decode("utf-8")
                self.update_graphs(reading)
                self.write_terminal(reading)

    def write_terminal(self, reading):
        self.formatter.insert_ansi(reading, "end")
        if self.auto_scroll.get():
            self.terminal.see(tk.END)


if __name__ == "__main__":

    root = tk.Tk()
    root.title("Remote Car Plotter")
    root.geometry("1280x720")
    
    tabControl = ttk.Notebook(root)
    tab1 = ttk.Frame(tabControl)
    tab2 = ttk.Frame(tabControl)
    
    tabControl.add(tab1, text="Main")
    tabControl.add(tab2, text="PID settings")
    tabControl.pack(expand=1, fill="both")
    
    app = SerialApp(tab1)
    root.mainloop()
    print("Exiting")
    app.close()
