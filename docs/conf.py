# Configuration file for the Sphinx documentation builder.

import os
import sys
from datetime import datetime

# Add project root and python package to path
PROJECT_ROOT = os.path.abspath("..")
PYTHON_DIR = os.path.join(PROJECT_ROOT, "python")
sys.path.insert(0, PROJECT_ROOT)
sys.path.insert(0, PYTHON_DIR)

project = "Ri7la - Routing Library"
author = "Sohaib Afifi"
copyright = f"2020-{datetime.now().year}, {author}"

extensions = [
    "myst_parser",
    "sphinx.ext.autodoc",
    "sphinx.ext.napoleon",
    "sphinx.ext.intersphinx",
    "sphinx.ext.viewcode",
    "sphinx_design",
    "sphinx_copybutton",
    "breathe",
    "sphinxcontrib.mermaid",
]

source_suffix = {
    ".rst": "restructuredtext",
    ".md": "markdown",
}

myst_enable_extensions = [
    "colon_fence",
    "deflist",
    "fieldlist",
    "tasklist",
    "attrs_inline",
    "attrs_block",
]

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

html_theme = "pydata_sphinx_theme"
html_title = "Ri7la: Composable VRP Library"
html_logo = "assets/logo.svg"
html_static_path = ["_static"]
html_css_files = ["custom.css"]

html_baseurl = "https://ri7la.sohaibafifi.com/"

html_theme_options = {
    "github_url": "https://github.com/sohaibafifi/routing",
    "use_edit_page_button": False,
    "logo": {
        "text": "Ri7la",
    },
    "navbar_start": ["navbar-logo"],
    "navbar_center": ["navbar-nav"],
    "navbar_end": ["theme-switcher", "navbar-icon-links"],
}

breathe_projects = {
    "routing": "_build/doxygen/xml",
}
breathe_default_project = "routing"

intersphinx_mapping = {
    "python": ("https://docs.python.org/3", None),
}

# Allow building docs without the compiled extension.
autodoc_mock_imports = ["routing._routing_core"]
