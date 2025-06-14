#!/bin/bash

# 42header_tool.sh
# 42のヘッダーを削除または挿入するスクリプト
# 使用法: ./42header_tool.sh [オプション] [ディレクトリ]

# 色の定義
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

show_help() {
    echo "42のソースコードファイルのヘッダーを削除または挿入します。"
    echo ""
    echo "usage: $0 [option] [directory]"
    echo ""
    echo "option:"
    echo "  -c, --check          ヘッダーの存在を確認します"
    echo "  -d, --delete         ヘッダーを削除します"
    echo "  -a, --add            ヘッダーを追加します"
	echo "  -r, --replace        ヘッダーを置き換えます"
    echo "  -h, --help           このヘルプを表示します"
    echo "  -u, --username USER  ユーザー名を指定します（デフォルト: 環境変数USERから取得）"
    echo "  -e, --email EMAIL    メールアドレスを指定します（デフォルト: username@student.42tokyo.jp）"
    echo ""
}

# デフォルト値
ACTION="check" # デフォルトはチェックモード
TARGET_DIR="."
TARGET_FILE=""
IS_FILE=0 # 0=ディレクトリ、1=ファイル
USERNAME="${USER}"
EMAIL=""

# 引数解析
while [[ $# -gt 0 ]]; do
    case $1 in
		-c|--check)
            ACTION="check"
            shift
            ;;
        -d|--delete)
            ACTION="delete"
            shift
            ;;
        -a|--add)
            ACTION="add"
            shift
            ;;
		-r|--replace)
            ACTION="replace"
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        -u|--username)
            USERNAME="$2"
            shift 2
            ;;
        -e|--email)
            EMAIL="$2"
            shift 2
            ;;
        *)
            if [[ -d "$1" ]]; then
                # ディレクトリの場合
                TARGET_DIR="$1"
                IS_FILE=0
            elif [[ -f "$1" ]]; then
                # ファイルの場合
                TARGET_FILE="$1"
                IS_FILE=1
            else
                echo -e "${RED}error: invalid argument or non-existent file/directory: $1${NC}"
                show_help
                exit 1
            fi
            shift
            ;;
    esac
done

# メールアドレスが指定されていない場合はデフォルト値を設定
if [[ -z "$EMAIL" ]]; then
    EMAIL="${USERNAME}@student.42tokyo.jp"
fi

# ヘッダーが存在するか確認する関数
check_header() {
    local file="$1"
    
    # ファイルの先頭13行を取得
    local header_lines=$(head -n 13 "$file" 2>/dev/null)
    
    # 42ヘッダーの特徴をチェック
    if echo "$header_lines" | grep -q "\* © \|By:\|Created:\|Updated:\|\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*"; then
        echo -e "${GREEN}header exists: $file${NC}"
        return 0
    else
        echo -e "${YELLOW}header not found: $file${NC}"
        return 1
    fi
}

# ヘッダーを削除する関数
delete_header() {
    local file="$1"
    local temp_file=$(mktemp)
    
    # check_header関数を使用してヘッダーの存在を確認
    if check_header "$file" > /dev/null; then
        # ヘッダー（13行）を削除
        tail -n +13 "$file" > "$temp_file"
        mv "$temp_file" "$file"
        echo -e "${GREEN}header deleted: $file${NC}"
        return 0
    else
        echo -e "${YELLOW}header not found: $file${NC}"
        rm "$temp_file"
        return 1
    fi
}

# 1行を 80 文字に整形するヘルパ
#   $1: 行頭固定部分   $2: 可変部分   $3: 行末固定部分
padline() {
    local head="$1" body="$2" tail="$3"
    local fill=$(( 80 - ${#head} - ${#body} - ${#tail} ))
    (( fill < 0 )) && fill=0       # はみ出したら 0 に潰す
    printf '%s%s%*s%s\n' "$head" "$body" "$fill" '' "$tail"
}

# ヘッダーを追加する関数
add_header() {
    local file="$1"
    local fname="$(basename "$file")"
    local tmp="$(mktemp)"
    local now="$(date '+%Y/%m/%d %H:%M:%S')"

    # 既存チェック
    if check_header "$file" >/dev/null; then
        echo -e "${YELLOW}header already exists: $file${NC}"
        rm "$tmp"; return 1
    fi

    # 各行生成 -------------------------------------------------------------
    {
    printf  '/* ************************************************************************** */\n'
    printf  '/*                                                                            */\n'
    printf  '/*                                                        :::      ::::::::   */\n'
    padline '/*   '  "$fname"              ':+:      :+:    :+:   */'
    printf  '/*                                                    +:+ +:+         +:+     */\n'
    padline '/*   By: '  "${USERNAME} <${EMAIL}>"  '+#+  +:+       +#+        */'
    printf  '/*                                                +#+#+#+#+#+   +#+           */\n'
    padline '/*   Created: '  "$now by $USERNAME"  '#+#    #+#             */'
    padline '/*   Updated: '  "$now by $USERNAME"  '###   ########.fr       */'
    printf  '/*                                                                            */\n'
    printf  '/* ************************************************************************** */\n\n'
    } >"$tmp"

    # 本文連結
    cat "$file" >>"$tmp"
    mv "$tmp" "$file"
    echo -e "${GREEN}header added: $file${NC}"
}

replace_header() {
    local file="$1"

    # ヘッダーがあるかチェック
    if check_header "$file" >/dev/null; then
        # 既存を削除してから再挿入
        if delete_header "$file" 1>/dev/null && add_header "$file" 1>/dev/null; then
            echo -e "${GREEN}header replaced: $file${NC}"
            return 0
        else
            echo -e "${RED}header replace failed: $file${NC}"
            return 1
        fi
    else
        # 無ければ単純に追加
        if add_header "$file"; then
            return 0
        else
            echo -e "${RED}header add failed: $file${NC}"
            return 1
        fi
    fi
}

# 対象ファイルを検索して処理
process_files() {
    local target="$1"
    local action="$2"
    local is_file="$3"
    local count_success=0
    local count_skipped=0
    local count_total=0
    local count_with_header=0
    local count_without_header=0
    local files=
    
    if [[ "$is_file" -eq 1 ]]; then
        # 単一ファイルの場合
        files="$target"
    else
        # ディレクトリの場合、C/C++ソースファイルとヘッダーファイルを検索
        files=$(find "$target" -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) | sort)
    fi
    
    # 各ファイルを処理
    while IFS= read -r file; do
        # ファイルが空でない場合のみ処理
        if [[ -n "$file" ]]; then
            count_total=$((count_total + 1))

            if [[ "$action" == "check" ]]; then
                if check_header "$file"; then
                    count_with_header=$((count_with_header + 1))
                else
                    count_without_header=$((count_without_header + 1))
                fi
            elif [[ "$action" == "delete" ]]; then
                if delete_header "$file"; then
                    count_success=$((count_success + 1))
                else
                    count_skipped=$((count_skipped + 1))
                fi
            elif [[ "$action" == "add" ]]; then
                if add_header "$file"; then
                    count_success=$((count_success + 1))
                else
                    count_skipped=$((count_skipped + 1))
                fi
			elif [[ "$action" == "replace" ]]; then
                if replace_header "$file"; then
                    count_success=$((count_success + 1))
                else
                    count_skipped=$((count_skipped + 1))
                fi
            fi	
        fi
    done <<< "$files"
    
    echo ""
    echo -e "${BLUE}process completed:${NC}"
    echo "  processed files: $count_total"
    
    if [[ "$action" == "check" ]]; then
        echo "  files with header: $count_with_header"
        echo "  files without header: $count_without_header"
    else
        echo "  success: $count_success"
        echo "  skipped: $count_skipped"
    fi
}

# メイン処理
echo -e "${BLUE}42header_tool${NC}"

if [[ "$IS_FILE" -eq 1 ]]; then
    echo "file: $TARGET_FILE"
else
    echo "directory: $TARGET_DIR"
fi

echo "username: $USERNAME"
echo "email: $EMAIL"
echo "action: $ACTION"
echo ""

# 処理実行
if [[ "$IS_FILE" -eq 1 ]]; then
    process_files "$TARGET_FILE" "$ACTION" "$IS_FILE"
else
    process_files "$TARGET_DIR" "$ACTION" "$IS_FILE"
fi

exit 0
