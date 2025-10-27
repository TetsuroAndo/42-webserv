# CGI チェックリスト

実行
- [ ] GET 実行: 200/本文一致
- [ ] POST 実行: 200/エコー
- [ ] スクリプトエラーで500
- [ ] 実行CWD が location.root と一致

環境変数
- [ ] REQUEST_METHOD (GET/POST)
- [ ] QUERY_STRING
- [ ] CONTENT_LENGTH/TYPE (POST)
- [ ] PATH_INFO/SCRIPT_NAME/SERVER_NAME

データ転送
- [ ] chunked POST → サーバーでunchunkされstdinへ
- [ ] CGI出力 EOF をボディ終端として扱う
