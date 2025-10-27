#!/bin/bash

usage() {
	echo "Usage: $0 [options] [path]"
	echo "Options:"
	echo "  -h, --help         Show this help message"
	echo "  -t, --tree         Only show directory tree"
	echo "  -v, --view         Only show file contents"
	echo "  -nc, --no-comments Do not remove comments from files"
	echo "  -nt, --no-tests    Do not show test files"
	echo "If no path is provided, the current directory will be used."
}

# Default options
SHOW_TREE=true
SHOW_VIEW=true
REMOVE_COMMENTS=true
SHOW_TESTS=true
TARGET_PATH="."

# Parse command line arguments
while [[ $# -gt 0 ]]; do
	case $1 in
		-h|--help)
			usage
			exit 0
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
		-nc|--no-comments)
			REMOVE_COMMENTS=false
			shift
			;;
		-nt|--no-tests)
			SHOW_TESTS=false
			IGNORE_ARRAY+=(
						"tests/*"        # testsディレクトリ配下のファイル
						"test/*"         # testディレクトリ配下のファイル
						"*/tests/*"      # サブディレクトリ内のtests配下のファイル
						"*/test/*"       # サブディレクトリ内のtest配下のファイル
						"*_test.?"       # _test.cで終わるファイル
						"*_test.?pp"     # _test.cppで終わるファイル
						"test_*.?"       # test_で始まるCファイル
						"test_*.?pp"     # test_で始まるCppファイル
						"*.spec.js"      # Jasmine/Jestなどのテストファイル
						"*.test.js"      # Jestなどのテストファイル
						"*.spec.ts"      # Jasmineなどのテストファイル
						"*.test.ts"      # Jestなどのテストファイル
			)
			shift
			;;
		-*)
			echo "Unknown option: $1"
			usage
			exit 1
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

# ===== Configuration for filtering tree output =====

# 探索を無効にするPATHパターン
TREE_EXCLUDE=(
	"node_modules"
	"dist"
	".git"
	"libft"
	"minilibx"
	"tools"
	"playground"
)
# .Gitignoreからも除外PATHを取得 (ディレクトリ名のみ)
if [[ -f "$TARGET_PATH/.gitignore" ]]; then
	while IFS= read -r line; do
		[[ -z "$line" || "$line" =~ ^# ]] && continue
		pattern="${line#/}"
		pattern="${pattern%/}"
		TREE_EXCLUDE+=("$pattern")
	done < "$TARGET_PATH/.gitignore"
fi

# ===== Show tree if enabled =====

if [[ "$SHOW_TREE" == true ]]; then
	TREE_IGNORE=$(IFS='|'; echo "${TREE_EXCLUDE[*]}")
	{
		echo "Directory structure of: $TARGET_PATH"
		echo "----------------------------------------"
		if [[ -d "$TARGET_PATH" ]]; then
			(cd "$TARGET_PATH" && tree -F -I "$TREE_IGNORE" )
		else
			echo "$TARGET_PATH (file)"
		fi
		echo "----------------------------------------"
		echo
	} >> "$TMP_OUTPUT"
fi

# ===== Configuration for viewing file contents =====

# 探索を無効にするPATHパターン
EXCLUDE_PATHS=(
	"*/node_modules/*"
	"*/dist/*"
	"*/.git/*"
	"*/lib/libft/*"
	"*/lib/minilibx/*"
	"*/tools/*"
	"*/playground/*"
)
# .Gitignoreからも除外PATHを取得
if [[ -f "$TARGET_PATH/.gitignore" ]]; then
	while IFS= read -r line; do
		[[ -z "$line" || "$line" =~ ^# ]] && continue
		pattern="${line#/}"
		pattern="${pattern%/}"
		EXCLUDE_PATHS+=("$TARGET_PATH/$pattern*")
	done < "$TARGET_PATH/.gitignore"
fi

# 探索に有効なネームパターン
NAME_PATTERNS=(
	"*.c"
	"*.h"
	"*.?pp"
	"*.py"
	"*.ts"
	"*.js"
	"*.tsx"
	"*.jsx"
	"*.css"
	"*.html"
	"*.json"
	"*.yaml"
	"*.yml"
	"*.sh"
	"Makefile"
)

# ===== Show file contents if enabled =====

if [[ "$SHOW_VIEW" == true ]]; then
	remove_comments() {
		local input_file="$1"
		local relative_path="${input_file#$TARGET_PATH/}"
		local file_ext="${input_file##*.}"
		local file_name="${input_file##*/}" # Makefile, Dockerfileなど拡張子がないファイル用

		{
			echo "----------------------------------------"
			echo "File: $relative_path"
			echo "----------------------------------------"

			if [[ "$REMOVE_COMMENTS" == true ]]; then

				if [[ "$file_name" == "Makefile" || \
					"$file_name" == "Dockerfile" || \
					"$file_ext" == "sh" || \
					"$file_ext" == "rb" || \
					"$file_ext" == "pl" || \
					"$file_ext" == "yaml" || \
					"$file_ext" == "yml" ]]; then
					# 's/#.*$//' は # 以降を削除
					sed 's/#.*$//' "$input_file" | awk 'NF'
				
				elif [[ "$file_ext" == "py" ]]; then
					sed '
						# 1行の docstring/複数行文字列 を削除
						s/""".*"""//g;
						s/''''.*''''//g;
						# 複数行の docstring/複数行文字列 を削除
						/"""/,/"""/d;
						/'''/,/'''/d;
						# 通常の # コメントを削除
						s/#.*$//
					' "$input_file" | awk 'NF'

				elif [[ "$file_ext" == "c" || \
						"$file_ext" == "h" || \
						"$file_ext" == "cpp" || \
						"$file_ext" == "hpp" || \
						"$file_ext" == "cs" || \
						"$file_ext" == "go" || \
						"$file_ext" == "java" || \
						"$file_ext" == "js" || \
						"$file_ext" == "ts" || \
						"$file_ext" == "jsx" || \
						"$file_ext" == "tsx" || \
						"$file_ext" == "swift" || \
						"$file_ext" == "php" || \
						"$file_ext" == "css" || \
						"$file_ext" == "scss" || \
						"$file_ext" == "less" ]]; then
					sed '
						# 1行内で完結するブロックコメント /* ... */ を削除
						s/\/\*.*\*\///g;
						# 複数行にまたがるブロックコメントを削除
						/\/\*.*/,/.*\*\//d;
						# 行末コメント // ... を削除（コード部分は残す）
						s/\/\/.*$//
					' "$input_file" | awk 'NF'
				
				elif [[ "$file_ext" == "html" || \
						"$file_ext" == "xml" || \
						"$file_ext" == "svg" ]]; then
					sed '
						s/<!--.*-->//g;
						//d
					' "$input_file" | awk 'NF'

				else
					cat "$input_file"
				fi
			else
				cat "$input_file"
			fi

			echo "----------------------------------------"
			echo
		} >> "$TMP_OUTPUT"
	}

	EXCLUDE_ARGS=()
	for path in "${EXCLUDE_PATHS[@]}"; do
		EXCLUDE_ARGS+=(-not -path "$path")
	done

	NAME_ARGS=()
	for pattern in "${NAME_PATTERNS[@]}"; do
		NAME_ARGS+=(-name "$pattern" -o)
	done
	unset 'NAME_ARGS[${#NAME_ARGS[@]}-1]' # remove last -o

	find "$TARGET_PATH" -type f "${EXCLUDE_ARGS[@]}" \( "${NAME_ARGS[@]}" \) | while read -r file; do
		rel_path="${file#$TARGET_PATH/}"
		rel_path="${rel_path#./}"

		is_ignored=false
		if [[ "$SHOW_TESTS" == false ]]; then
				for pattern in "${IGNORE_ARRAY[@]}"; do
						if [[ "$rel_path" == $pattern ]]; then
								is_ignored=true
								break # 一致したらループを抜ける
						fi
				done
		fi

		if [[ "$is_ignored" == true ]]; then
				continue # 除外リストに一致したら次のファイルへ
		fi

		remove_comments "$file"
	done
fi

# Display the content
cat "$TMP_OUTPUT"

# Clean up
rm "$TMP_OUTPUT"
