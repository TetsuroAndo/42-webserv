# Methods チェックリスト

GET
- [ ] 200 text: Content-Type/Length/本文一致
- [ ] 200 html: Content-Type/本文一致
- [ ] 404 not found
- [ ] 403 読み取り不可
- [ ] index 適用
- [ ] autoindex on/off
- [ ] ディレクトリ末尾スラなし→301 スラ付へ
- [ ] HEAD: ボディなしでヘッダ一致

POST (uploadStore)
- [ ] 201 作成、ファイル実体保存
- [ ] 413 サイズ超過、ファイル未作成
- [ ] 405 不許可location
- [ ] chunked 送信で正しく受理

DELETE
- [ ] 204/200 削除成功、実体消去
- [ ] 404 不在
- [ ] 405 不許可location
