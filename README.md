# Psy-Q Symbol Extractor

This is a tool to extract symbols from a PSY-Q generated symbol file.

## Usage

    extract-psyq-symbols -i [input] -o [output] <-f [symbol]> <-x [symbol]> <-p [prefix]> <-xp [prefix]> <-s [suffix]> <-xs [suffix]>
    -i [input]     - Input symbol file
    -o [output]    - Output file
    <-f [symbol]>  - Force include symbol
    <-x [symbol]>  - Exclude symbol
    <-p [prefix]>  - Only include symbols with prefix
    <-xp [prefix]> - Exclude symbols with prefix
    <-s [suffix]>  - Only include symbols with suffix
    <-xs [suffix]> - Exclude symbols with suffix
