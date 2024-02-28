from tkinter import END, Misc, Scrollbar, Text

from tkAnsiFormatter import tkAnsiFormatter


class tkTerminal:
    def __init__(
        self, root: Misc, width: int = 80, lines: int = 200, autoscroll: bool = True
    ) -> None:

        self.lines = lines
        self.autoscroll = autoscroll

        # Create a scrollbar
        self.scrollbar = Scrollbar(root)
        self.scrollbar.grid(row=1, column=3, sticky="ns")

        # Create a text widget for the terminal
        self.terminal = Text(root, width=width, yscrollcommand=self.scrollbar.set, background="#E7FCF6")
        self.terminal.grid(row=1, column=0, columnspan=3)
        self.ansi_formatter = tkAnsiFormatter(self.terminal)

        # Configure the scrollbar to scroll the text widget
        self.scrollbar.config(command=self.terminal.yview)

    # Partial function of tk.grid()
    def grid(self, row: int, column: int, columnspan: int) -> None:
        self.scrollbar.grid(row=row, column=columnspan, sticky="ns")
        self.terminal.grid(row=row, column=column, columnspan=columnspan)

    # Writes text on screen
    def write(self, data: str) -> None:
        if self.ansi_formatter:
            self.ansi_formatter.insert_ansi(txt=data, index=END)
        else:
            self.terminal.insert(chars=data, index=END)

        # Scroll to the END
        if self.autoscroll:
            self.terminal.see(index=END)

        # Limit the number of lines in the terminal
        if int(self.terminal.index("end-1c").split(".")[0]) > self.lines:
            self.terminal.delete("1.0", "2.0")

    def set_autoscroll(self, autoscroll: bool) -> None:
        self.autoscroll = autoscroll
