# Routing/Directives チェックリスト

location/root
- [ ] root 継承: server.root が location に適用
- [ ] root 上書き: location.root 優先
- [ ] root とURL結合の挙動確認

index/autoindex
- [ ] indexFile があれば優先
- [ ] autoindex on: 一覧HTML
- [ ] autoindex off: 403

allowedMethods
- [ ] GETのみ許可: POST/DELETEは405
- [ ] 未指定(default): GET/POST/DELETE が設定に応じて動作

redirect
- [ ] 301 恒久: Location 正しい
- [ ] 302 一時: Location 正しい
- [ ] 外部 302: http://example.com に転送
- [ ] クエリ保持(必要に応じて 307/308)

error_pages
- [ ] カスタム404: 本文差し替え、コード保持
- [ ] デフォルト404
