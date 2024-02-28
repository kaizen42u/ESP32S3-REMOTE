import re
import tkinter as tk
from tkinter import font

# dictionaries to replace formatting code with tags
ansi_font_format = {1: "bold", 3: "italic", 4: "underline", 9: "overstrike"}
ansi_font_reset = {21: "bold", 23: "italic", 24: "underline", 29: "overstrike"}

# dictionaries to replace color code with tags
ansi_color_fg = {39: "foreground default"}
ansi_color_bg = {49: "background default"}
ansi_colors_dark = [
    "black",
    "red",
    "green",
    "yellow",
    "royal blue",
    "magenta",
    "cyan",
    "light gray",
]
ansi_colors_light = [
    "dark gray",
    "tomato",
    "light green",
    "light goldenrod",
    "light blue",
    "pink",
    "light cyan",
    "white",
]

# regular expression to find ansi codes in string
ansi_regexp = re.compile(r"\x1b\[((\d+;)*\d+)m")
ansi_escape = re.compile(r"\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])")


class tkAnsiFormatter:
    def __init__(self, text: tk.Text, font: str = "Consolas", size: int = 9) -> None:
        self.text = text
        self.font = font
        self.size = size
        self.configure_style()

    @staticmethod
    def escaped(str: str) -> str:
        return ansi_escape.sub("", str)

    def configure_style(self) -> None:
        self.text.configure(font=(self.font, self.size))
        self.text.tag_configure("bold", font=(self.font, self.size, "bold"))
        self.text.tag_configure("italic", font=(self.font, self.size, "italic"))
        self.text.tag_configure("underline", underline=True)
        self.text.tag_configure("overstrike", overstrike=True)
        self.text.tag_configure("foreground default", foreground=self.text["fg"])
        self.text.tag_configure("background default", background=self.text["bg"])

        for i, (col_dark, col_light) in enumerate(
            zip(ansi_colors_dark, ansi_colors_light)
        ):
            ansi_color_fg[30 + i] = "foreground " + col_dark
            ansi_color_fg[90 + i] = "foreground " + col_light
            ansi_color_bg[40 + i] = "background " + col_dark
            ansi_color_bg[100 + i] = "background " + col_light
            self.text.tag_configure("foreground " + col_dark, foreground=col_dark)
            self.text.tag_configure("background " + col_dark, background=col_dark)
            self.text.tag_configure("foreground " + col_light, foreground=col_light)
            self.text.tag_configure("background " + col_light, background=col_light)

    def insert_ansi(self, txt: str, index: str = "insert") -> None:
        first_line, first_char = map(int, str(self.text.index(index)).split("."))

        if index == "end":
            first_line -= 1

        lines = txt.splitlines()
        if not lines:
            return

        # insert text without ansi codes
        self.text.insert(index, ansi_regexp.sub("", txt))

        # find all ansi codes in txt and apply corresponding tags
        opened_tags: dict[str, str] = {}

        # text.tag_add(tag, start, end) when we reach a "closing" ansi code
        def apply_formatting(code: int, code_index: str) -> None:
            if code == 0:  # reset all by closing all opened tag
                for tag, start in opened_tags.items():
                    self.text.tag_add(tag, start, code_index)
                opened_tags.clear()

            elif code in ansi_font_format:  # open font formatting tag
                tag = ansi_font_format[code]
                opened_tags[tag] = code_index

            elif code in ansi_font_reset:  # close font formatting tag
                tag = ansi_font_reset[code]
                if tag in opened_tags:
                    self.text.tag_add(tag, opened_tags[tag], code_index)
                    del opened_tags[tag]

            elif (
                code in ansi_color_fg
            ):  # open foreground color tag (and close previously opened one if any)
                for tag in tuple(opened_tags):
                    if tag.startswith("foreground"):
                        self.text.tag_add(tag, opened_tags[tag], code_index)
                        del opened_tags[tag]
                opened_tags[ansi_color_fg[code]] = code_index

            elif (
                code in ansi_color_bg
            ):  # open background color tag (and close previously opened one if any)
                for tag in tuple(opened_tags):
                    if tag.startswith("background"):
                        self.text.tag_add(tag, opened_tags[tag], code_index)
                        del opened_tags[tag]
                opened_tags[ansi_color_bg[code]] = code_index

        def find_ansi(line_txt: str, line_nb: int, char_offset: int) -> None:
            # difference between the character position in the original line and in the text widget
            # (initial offset due to insertion position if first line + extra offset due to deletion of ansi codes)
            delta = -char_offset

            for match in ansi_regexp.finditer(line_txt):
                codes = [int(c) for c in match.groups()[0].split(";")]
                start, end = match.span()
                for code in codes:
                    apply_formatting(code, "{}.{}".format(line_nb, start - delta))

                # take into account offset due to deletion of ansi code
                delta += end - start

        # first line, with initial offset due to insertion position
        find_ansi(lines[0], first_line, first_char)

        for line_nb, line in enumerate(lines[1:], first_line + 1):
            find_ansi(line, line_nb, 0)  # next lines, no offset

        # close still opened tag
        for tag, start in opened_tags.items():
            self.text.tag_add(tag, start, "end")


if __name__ == "__main__":

    # example for the kind of output you can get with "ls --color"
    output = "file.pdf\nfile.txt\n\x1b[0m\x1b[01;34mfolder\x1b[0m\n\x1b[01;32mscript.py\x1b[0m\ntest\n"

    root = tk.Tk()
    terminal = tk.Text(root, width=160)
    terminal.pack()
    formatter = tkAnsiFormatter(terminal)
    formatter.insert_ansi(output, "end")

    print(font.families())
    root.mainloop()
