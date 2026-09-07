import argparse
import os
import subprocess
import sys

from jinja2 import Environment, FileSystemLoader

# Theme directory name -> BOARD macro required to compile the theme (matches $ENV{BOARD} in CMakeLists)
THEME_BOARDS = {
    "NerdOctaxeGamma": ["NERDOCTAXEGAMMA"],
    "NerdQaxePlus": ["NERDQAXEPLUS"],
    "NerdEko": ["NERDEKO"],
    "NerdQaxePlus2": ["NERDQAXEPLUS2"],
    "NerdAxe": ["NERDAXE"],
    "NerdAxeGamma": ["NERDAXEGAMMA"],
    "NerdQX": ["NERDQX"],
    "NerdOctaxePlus": ["NERDOCTAXEPLUS"],
    "Generic": ["Q1370", "Q1373"],
    "NerdHaxeGamma": ["NERDHAXEGAMMA"],
    "BV001": ["BV001"],
    "BV002": ["BV002"],
    "BV003": ["BV003"],
}

# Function to process each theme directory and convert PNG files to C files
def process_theme(theme, rpath):
    theme_path = os.path.join("../themes", theme)
    os.chdir(theme_path)

    for screen in ["initscreen2", "miningscreen2", "portalscreen", "btcscreen", "settingsscreen", "splashscreen2" , "globalStats"]:
        image_path = f"./Raw Images/{screen}.png"
        # Run the convert_single.py script to generate the C files for each image
        #subprocess.run(["python3", f"{rpath}/convert_single.py", theme, image_path, screen], check=True)
        subprocess.run([sys.executable, os.path.join(rpath, "convert_single.py"), theme, image_path, screen], check=True)


    os.chdir(rpath)

# Function to generate a file using Jinja2 templates
def generate_file(template_file, output_file, context):
    env = Environment(
        loader=FileSystemLoader('.'),
        trim_blocks=True,
        lstrip_blocks=True
    )
    template = env.get_template(template_file)

    # Render the template with the given context
    rendered_output = template.render(context)

    # Write the rendered output to the output file
    with open(output_file, 'w') as f:
        f.write(rendered_output)

def generate_theme_headers(rpath, theme_dirs):
    screens = [
        "initscreen2",
        "miningscreen2",
        "portalscreen",
        "btcscreen",
        "settingsscreen",
        "splashscreen2",
        "globalStats",
    ]
    missing = [t for t in theme_dirs if t not in THEME_BOARDS]
    if missing:
        raise ValueError(f"Theme directory missing BOARD mapping: {missing}")

    context = {
        "themes": theme_dirs,
        "screens": screens,
        "theme_boards": THEME_BOARDS,
    }

    os.chdir(rpath)
    generate_file("themes.h.j2", "../themes/themes.h", context)
    generate_file("themes.c.j2", "../themes/themes.c", context)


# Main function to process themes and generate necessary files
def main():
    parser = argparse.ArgumentParser(description="Convert theme images and generate themes.c/h")
    parser.add_argument(
        "--headers-only",
        action="store_true",
        help="Only regenerate themes.c and themes.h, do not convert PNG",
    )
    args = parser.parse_args()

    rpath = os.path.dirname(os.path.realpath(__file__))
    themes_root = os.path.join(rpath, "../themes")
    os.chdir(themes_root)

    theme_dirs = sorted(
        d for d in os.listdir(".") if os.path.isdir(d) and d in THEME_BOARDS
    )
    unknown_dirs = sorted(
        d for d in os.listdir(".") if os.path.isdir(d) and d not in THEME_BOARDS
    )
    if unknown_dirs:
        print(f"Skipping theme directories without BOARD mapping: {unknown_dirs}")

    if not args.headers_only:
        for theme in theme_dirs:
            print(f"Processing theme: {theme}")
            process_theme(theme, rpath)

    generate_theme_headers(rpath, theme_dirs)

if __name__ == "__main__":
    main()
