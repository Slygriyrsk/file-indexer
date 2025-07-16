#!/bin/bash

# Package script for distribution

VERSION="1.0.0"
ARCH=$(uname -m)
OS=$(uname -s | tr '[:upper:]' '[:lower:]')

echo "Packaging Fast Search Tool v${VERSION} for ${OS}-${ARCH}"

# Build first
./build.sh

if [ $? -ne 0 ]; then
    echo "Build failed, cannot package"
    exit 1
fi

# Create package directory
PACKAGE_DIR="fastsearch-${VERSION}-${OS}-${ARCH}"
mkdir -p ${PACKAGE_DIR}

# Copy files
cp build/fastsearch ${PACKAGE_DIR}/
cp README.md ${PACKAGE_DIR}/
cp LICENSE ${PACKAGE_DIR}/ 2>/dev/null || echo "No LICENSE file found"

# Create install script
cat > ${PACKAGE_DIR}/install.sh << 'EOF'
#!/bin/bash
echo "Installing Fast Search Tool..."
sudo cp fastsearch /usr/local/bin/
sudo chmod +x /usr/local/bin/fastsearch
echo "Installation complete!"
echo "Run 'fastsearch help' to get started"
EOF

chmod +x ${PACKAGE_DIR}/install.sh

# Create archive
tar -czf ${PACKAGE_DIR}.tar.gz ${PACKAGE_DIR}/

echo "Package created: ${PACKAGE_DIR}.tar.gz"
echo "To distribute:"
echo "1. Upload to GitHub releases"
echo "2. Share the .tar.gz file"
echo "3. Users can extract and run ./install.sh"