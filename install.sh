#!/bin/sh
# Registers this Tiko editor folder for the current user, without copying it:
#   ~/.local/bin/tiko-editor                      command (link to ./tiko)
#   ~/.local/share/applications/tiko-editor.desktop  menu entry
#   ~/.local/share/icons/hicolor/64x64/apps/tiko-editor.png
# Run it again after moving the folder.  ./install.sh --uninstall removes the three files.
set -e

BIN_DIR="$HOME/.local/bin"
APP_DIR="${XDG_DATA_HOME:-$HOME/.local/share}/applications"
ICON_DIR="${XDG_DATA_HOME:-$HOME/.local/share}/icons/hicolor/64x64/apps"
DESKTOP="$APP_DIR/tiko-editor.desktop"

refresh() {
    command -v update-desktop-database >/dev/null 2>&1 && update-desktop-database -q "$APP_DIR" || true
    command -v gtk-update-icon-cache >/dev/null 2>&1 && \
        gtk-update-icon-cache -q -t "${XDG_DATA_HOME:-$HOME/.local/share}/icons/hicolor" || true
}

if [ "$1" = "--uninstall" ]; then
    rm -f "$BIN_DIR/tiko-editor" "$DESKTOP" "$ICON_DIR/tiko-editor.png"
    refresh
    echo "Tiko editor removed from the menu and from $BIN_DIR."
    exit 0
fi

DIR=$(cd "$(dirname "$0")" && pwd -P)
if [ ! -x "$DIR/tiko" ]; then
    echo "$DIR/tiko not found: run 'make' first (or run this script from the extracted release)." >&2
    exit 1
fi

mkdir -p "$BIN_DIR" "$APP_DIR" "$ICON_DIR"
ln -sfn "$DIR/tiko" "$BIN_DIR/tiko-editor"
cp "$DIR/bin/tiko.png" "$ICON_DIR/tiko-editor.png"

cat > "$DESKTOP" <<EOF
[Desktop Entry]
Type=Application
Name=Tiko Editor
GenericName=Code Editor
Comment=Code editor for FreeBASIC and the tiko language
Exec="$DIR/tiko" %F
Icon=tiko-editor
Terminal=false
Categories=Development;IDE;TextEditor;
Keywords=FreeBASIC;BASIC;tiko;fbc;IDE;
MimeType=text/x-basic;
StartupNotify=true
StartupWMClass=Tiko
EOF

refresh
echo "Installed: tiko-editor -> $DIR/tiko, menu entry 'Tiko Editor'."
case ":$PATH:" in
    *":$BIN_DIR:"*) ;;
    *) echo "Note: $BIN_DIR is not on your PATH; add it to use the tiko-editor command." ;;
esac
