const { execSync } = require("child_process");

function runCpp(mode, input) {
  try {
    // C++バイナリを実行
    return execSync(`./uri_test ${mode} '${input}'`).toString().trim();
  } catch (e) {
    return null;
  }
}

function testEncodeDecode(input) {
  const jsEncoded = encodeURI(input);
  const cppEncoded = runCpp("encodeURI", input);
  const jsDecoded = decodeURI(jsEncoded);
  const cppDecoded = runCpp("decodeURI", jsEncoded);
  const jsEncodedComp = encodeURIComponent(input);
  const cppEncodedComp = runCpp("encodeURIComponent", input);
  const jsDecodedComp = decodeURIComponent(jsEncodedComp);
  const cppDecodedComp = runCpp("decodeURIComponent", jsEncodedComp);

  console.log(`Input:        ${input}`);
  console.log(`JS Encoded:   ${jsEncoded}`);
  console.log(`C++ Encoded:  ${cppEncoded}`);
  console.log(`JS Decoded:   ${jsDecoded}`);
  console.log(`C++ Decoded:  ${cppDecoded}`);
  console.log(`Encode Match: ${jsEncoded === cppEncoded}`);
  console.log(`Decode Match: ${jsDecoded === cppDecoded}`);
  console.log("-----------------------------");
  console.log(`JS Encoded Comp:   ${jsEncodedComp}`);
  console.log(`C++ Encoded Comp:  ${cppEncodedComp}`);
  console.log(`JS Decoded Comp:   ${jsDecodedComp}`);
  console.log(`C++ Decoded Comp:  ${cppDecodedComp}`);
  console.log(`Encode Comp Match: ${jsEncodedComp === cppEncodedComp}`);
  console.log(`Decode Comp Match: ${jsDecodedComp === cppDecodedComp}`);
  console.log("=============================\n");
}

// テストケース
const testCases = [
  "https://example.com/こんにちは?foo=bar&baz=qux",
  "abc def/ghi?jkl=mno&pqr=stu",
  "a+b=c&d=e",
  "スペースと記号!@#$%^&*()",
  "emoji: 😃🚀",
  "", // 空文字
  "simpleASCII123", // 英数字のみ
  "foo%20bar", // すでにエンコード済み
  "foo%ZZbar", // 不正なエンコード
  "foo=bar&baz=qux", // クエリパラメータ
  "path/to/resource", // パス
  "email@example.com", // メールアドレス
  "reserved;/?:@&=+$,", // 予約文字
  "unsafe<>#%{}|\\^~[]`", // 非推奨文字
  "multi byte: 漢字カタカナひらがな", // マルチバイト文字
  "control\u0000\u0001\u0002", // 制御文字
  "surrogate pair: \uD83D\uDE00", // サロゲートペア（絵文字）
  "percent%25sign", // パーセント記号
  "space and tab\t", // スペースとタブ
  "newline\ncarriage\rreturn", // 改行・復帰
];

for (const input of testCases) {
  testEncodeDecode(input);
}
