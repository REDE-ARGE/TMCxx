# TMCxx Documentation

This directory contains the Doxygen configuration for generating API documentation.

## Generate Locally

1. Install Doxygen:
   ```bash
   # Ubuntu/Debian
   sudo apt install doxygen graphviz
   
   # macOS
   brew install doxygen graphviz
   
   # Windows
   choco install doxygen.install graphviz
   ```

2. Generate docs:
   ```bash
   cd docs
   doxygen Doxyfile
   ```

3. Open `docs/html/index.html` in your browser.

## Online Documentation

Documentation is automatically deployed to GitHub Pages on every push to `main`.

Visit: https://REDE-ARGE.github.io/TMCxx/
