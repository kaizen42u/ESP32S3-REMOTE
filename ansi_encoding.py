class ANSI:
    default = "[0m"
    reset = "[0m"
    normal = "[0m"

    bold = "[1m"
    incIntensity = "[1m"
    faint = "[2m"
    decIntensity = "[2m"
    italic = "[3m"
    underline = "[4m"
    slowBlink = "[5m"
    fastBlink = "[6m"
    reverse = "[7m"
    invert = "[7m"
    conceal = "[8m"
    hide = "[8m"
    crossedout = "[9m"
    strike = "[9m"

    primaryFont = "[10m"
    altFont1 = "[11m"
    altFont2 = "[12m"
    altFont3 = "[13m"
    altFont4 = "[14m"
    altFont5 = "[15m"
    altFont6 = "[16m"
    altFont7 = "[17m"
    altFont8 = "[18m"
    altFont9 = "[19m"

    fraktur = "[20m"
    gothic = "[20m"
    doublyUnderlined = "[21m"
    notBold = "[21m"
    normIntensity = "[22m"
    notItalic = "[23m"
    notBlackletter = "[23m"
    notUnderlined = "[24m"
    notBlinking = "[25m"
    propSpacing = "[26m"
    notReversed = "[27m"
    notInverted = "[27m"
    reveal = "[28m"
    notConcealed = "[28m"
    notCrossed = "[29m"
    notStriked = "[29m"

    fBlack = "[30m"
    fRed = "[31m"
    fGreen = "[32m"
    fYellow = "[33m"
    fBlue = "[34m"
    fMagenta = "[35m"
    fCyan = "[36m"
    fGray = "[37m"
    _fColor = "[38;5;{n}m"
    _fColorRGB = "[38;2;{r};{g};{b}m"
    fDefault = "[39m"

    bBlack = "[40m"
    bRed = "[41m"
    bGreen = "[42m"
    bYellow = "[43m"
    bBlue = "[44m"
    bMagenta = "[45m"
    bCyan = "[46m"
    bGray = "[47m"
    _bColor = "[48;5;{n}m"
    _bColorRGB = "[48;2;{r};{g};{b}m"
    bDefault = "[49m"

    noPropSpacing = "[50m"
    framed = "[51m"
    encircled = "[52m"
    overlined = "[53m"
    notFramed = "[54m"
    notEncircled = "[54m"
    notOverlined = "[55m"

    _uColor = "[58;5;{n}m"
    _uColorRGB = "[58;2;{r};{g};{b}m"
    uDefault = "[59m"

    fBrightGray = "[90m"
    fBrightRed = "[91m"
    fBrightGreen = "[92m"
    fBrightYellow = "[93m"
    fBrightBlue = "[94m"
    fBrightMagenta = "[95m"
    fBrightCyan = "[96m"
    fBrightWhite = "[97m"

    bBrightGray = "[100m"
    bBrightRed = "[101m"
    bBrightGreen = "[102m"
    bBrightYellow = "[103m"
    bBrightBlue = "[104m"
    bBrightMagenta = "[105m"
    bBrightCyan = "[106m"
    bBrightWhite = "[107m"

    highlightRed = fBrightWhite + bBrightRed
    highlightGray = fBrightWhite + bGray
    highlightBrightGray = fBrightWhite + bBrightGray
    highlightChetwodeBlue = fBrightWhite + _bColor.format(n=104)


def fColor(n: int) -> str:
    return ANSI._fColor.format(n=n)


def bColor(n: int) -> str:
    return ANSI._bColor.format(n=n)


def uColor(n: int) -> str:
    return ANSI._uColor.format(n=n)


def fColorRGB(r: int, g: int, b: int) -> str:
    return ANSI._fColorRGB.format(r=r, g=g, b=b)


def bColorRGB(r: int, g: int, b: int) -> str:
    return ANSI._bColorRGB.format(r=r, g=g, b=b)


def uColorRGB(r: int, g: int, b: int) -> str:
    return ANSI._uColorRGB.format(r=r, g=g, b=b)


if __name__ == "__main__":
    print(" - ANSI Standard Color -")
    print(f"{ANSI.fBlack} Foreground Black   {ANSI.default}", end="")
    print(f"{ANSI.bBlack} Background Black   {ANSI.default}")
    print(f"{ANSI.fRed} Foreground Red     {ANSI.default}", end="")
    print(f"{ANSI.bRed} Background Red     {ANSI.default}")
    print(f"{ANSI.fGreen} Foreground Green   {ANSI.default}", end="")
    print(f"{ANSI.bGreen} Background Green   {ANSI.default}")
    print(f"{ANSI.fYellow} Foreground Yellow  {ANSI.default}", end="")
    print(f"{ANSI.bYellow} Background Yellow  {ANSI.default}")
    print(f"{ANSI.fBlue} Foreground Blue    {ANSI.default}", end="")
    print(f"{ANSI.bBlue} Background Blue    {ANSI.default}")
    print(f"{ANSI.fMagenta} Foreground Magenta {ANSI.default}", end="")
    print(f"{ANSI.bMagenta} Background Magenta {ANSI.default}")
    print(f"{ANSI.fCyan} Foreground Cyan    {ANSI.default}", end="")
    print(f"{ANSI.bCyan} Background Cyan    {ANSI.default}")
    print(f"{ANSI.fGray} Foreground Gray    {ANSI.default}", end="")
    print(f"{ANSI.bGray} Background Gray    {ANSI.default}")

    print(" - ANSI Standard Color Bright -")
    print(f"{ANSI.fBrightWhite} Foreground Bright White   {ANSI.default}", end="")
    print(f"{ANSI.bBrightWhite} Background Bright White   {ANSI.default}")
    print(f"{ANSI.fBrightRed} Foreground Bright Red     {ANSI.default}", end="")
    print(f"{ANSI.bBrightRed} Background Bright Red     {ANSI.default}")
    print(f"{ANSI.fBrightGreen} Foreground Bright Green   {ANSI.default}", end="")
    print(f"{ANSI.bBrightGreen} Background Bright Green   {ANSI.default}")
    print(f"{ANSI.fBrightYellow} Foreground Bright Yellow  {ANSI.default}", end="")
    print(f"{ANSI.bBrightYellow} Background Bright Yellow  {ANSI.default}")
    print(f"{ANSI.fBrightBlue} Foreground Bright Blue    {ANSI.default}", end="")
    print(f"{ANSI.bBrightBlue} Background Bright Blue    {ANSI.default}")
    print(f"{ANSI.fBrightMagenta} Foreground Bright Magenta {ANSI.default}", end="")
    print(f"{ANSI.bBrightMagenta} Background Bright Magenta {ANSI.default}")
    print(f"{ANSI.fBrightCyan} Foreground Bright Cyan    {ANSI.default}", end="")
    print(f"{ANSI.bBrightCyan} Background Bright Cyan    {ANSI.default}")
    print(f"{ANSI.fBrightGray} Foreground Bright Gray    {ANSI.default}", end="")
    print(f"{ANSI.bBrightGray} Background Bright Gray    {ANSI.default}")

    print(" - ANSI Color Table -")
    for n in range(0, 255, 8):
        escaped = "\\x1b"
        print(
            f"{fColor(n)} {fColor(n).replace('', escaped).ljust(18)} {ANSI.default}",
            end="",
        )
        print(f"{bColor(n)} {bColor(n).replace('', escaped).ljust(18)} {ANSI.default}")
