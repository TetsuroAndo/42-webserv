#!/bin/bash

usage() {
    echo "Usage: $0 [options] [path]"
    echo "Options:"
    echo "  -h, --help     Show this help message"
    echo "  -t, --tree     Only show directory tree"
    echo "  -v, --view     Only show file contents"
    echo "If no path is provided, the current directory will be used."
    exit 1
}

# Default options
SHOW_TREE=true
SHOW_VIEW=true
TARGET_PATH="."

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            usage
            ;;
        -t|--tree)
            SHOW_TREE=true
            SHOW_VIEW=false
            shift
            ;;
        -v|--view)
            SHOW_TREE=false
            SHOW_VIEW=true
            shift
            ;;
        -*)
            echo "Unknown option: $1"
            usage
            ;;
        *)
            TARGET_PATH="$1"
            shift
            ;;
    esac
done

# Check if target path exists
if [[ ! -d "$TARGET_PATH" && ! -f "$TARGET_PATH" ]]; then
    echo "Error: Path '$TARGET_PATH' does not exist"
    exit 1
fi

TMP_OUTPUT=$(mktemp)

remove_comments() {
    local input_file="$1"
    local relative_path="${input_file#$TARGET_PATH/}"
    local file_ext="${input_file##*.}"
    
    # libftディレクトリはスキップ
    if [[ "$input_file" == *"/lib/libft/"* || "$input_file" == *"/lib/minilibx/"* ]]; then
        return
    fi
    
    {
        echo "----------------------------------------"
        echo "File: $relative_path"
        echo "----------------------------------------"
        
        if [[ "$relative_path" == *"Makefile"* ]]; then
            sed 's/#.*$//' "$input_file" | awk 'NF'
        else
            sed '
                # 1行内で完結するブロックコメント /* ... */ を削除
                s/\/\*.*\*\///g; 
                # 複数行にまたがるブロックコメントを削除
                /\/\*.*/,/.*\*\//d; 
                # 行末コメント // ... を削除（コード部分は残す）
                s/\/\/.*$//
            ' "$input_file" | awk 'NF'
        fi
        
        echo "----------------------------------------"
        echo
    } >> "$TMP_OUTPUT"
}

# Show tree if enabled
if [[ "$SHOW_TREE" == true ]]; then
    {
        echo "Directory structure of: $TARGET_PATH"
        echo "----------------------------------------"
        if [[ -d "$TARGET_PATH" ]]; then
            (cd "$TARGET_PATH" && tree -F -I ".git|node_modules|dist" | sed -e '/ft_[a-z].*\.[ch]/d' -e '/type_put/d' -e '/get_next_line/d')
        else
            echo "$TARGET_PATH (file)"
        fi
        echo "----------------------------------------"
        echo
    } >> "$TMP_OUTPUT"
fi

# Show file contents if enabled
if [[ "$SHOW_VIEW" == true ]]; then
    if [[ -d "$TARGET_PATH" ]]; then
        # Find files in the target directory
        find "$TARGET_PATH" -type f -not -path "*/node_modules/*" -not -path "*/dist/*" -not -path "*/.git/*" \( -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.ts" -o -name "*.js" -o -name "*.tsx" -o -name "*.jsx" -o -name "*.css" -o -name "*.html" -o -name "*.json" -o -name "Makefile" \) | while read -r file; do
            remove_comments "$file"
        done
    elif [[ -f "$TARGET_PATH" ]]; then
        # Process a single file
        remove_comments "$TARGET_PATH"
    fi
fi

# Display the content
cat "$TMP_OUTPUT"

# Clean up
rm "$TMP_OUTPUT"
