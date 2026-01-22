# -- Import setup -------------------------------------------------------------

from os import path

# -- Project information ------------------------------------------------------

repository = 'scopy'
project = 'Scopy'
copyright = '2024, Analog Devices, Inc.'
author = 'Analog Devices, Inc.'

# -- General configuration ----------------------------------------------------

extensions = [
    "sphinx.ext.todo",
    "sphinx.ext.autosectionlabel",
    "adi_doctools"
]

needs_extensions = {
    'adi_doctools': '0.3'
}

exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']
source_suffix = '.rst'

# -- External docs configuration ----------------------------------------------

interref_repos = ['doctools']

# -- Custom extensions configuration ------------------------------------------

hide_collapsible_content = True
validate_links = False

# -- todo configuration -------------------------------------------------------

todo_include_todos = True
todo_emit_warnings = True

# -- autosectionlabel prefix configuration -------------------------------------------------------
# Use filename prefix to avoid section name collisions across different files
autosectionlabel_prefix_document = True
# Maximum depth for auto-labeling sections
autosectionlabel_maxdepth = 3

# -- Options for HTML output --------------------------------------------------

html_theme = 'cosmic'
html_static_path = ['sources']
html_css_files = ["custom.css"]
html_favicon = path.join("sources", "scopy.png")

html_theme_options = {
    "light_logo": "logo.svg",
}

