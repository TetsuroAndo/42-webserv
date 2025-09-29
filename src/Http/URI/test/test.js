const {execFileSync, spawnSync} = require("child_process");

function runCpp(mode, input) {
    try {
        return execFileSync("./uri_test", [mode], {
            input: Buffer.from(input, "utf8"),
            encoding: "utf8"
        }).trim();
    } catch (e) {
        return { error: true, message: e.message };
    }
}

function safeRunJS(fn, arg) {
    try {
        return fn(arg);
    } catch (e) {
        return { error: true, message: e.message };
    }
}

function testCase(input) {
    console.log("=========================================");
    console.log("Input:", JSON.stringify(input));

    // encodeURI
    const jsEnc = safeRunJS(encodeURI, input);
    const cppEnc = runCpp("encodeURI", input);
    if (typeof jsEnc === "object" && jsEnc.error) {
        console.log("[JS encodeURI ERROR]", jsEnc.message);
    }
    if (typeof cppEnc === "object" && cppEnc.error) {
        console.log("[C++ encodeURI ERROR]", cppEnc.message);
    }
    console.log("encodeURI Match:", jsEnc === cppEnc);

    // decodeURI
    const jsDec = safeRunJS(decodeURI, jsEnc.error ? input : jsEnc);
    const cppDec = runCpp("decodeURI", jsEnc.error ? input : jsEnc);
    if (typeof jsDec === "object" && jsDec.error) {
        console.log("[JS decodeURI ERROR]", jsDec.message);
    }
    if (typeof cppDec === "object" && cppDec.error) {
        console.log("[C++ decodeURI ERROR]", cppDec.message);
    }
    console.log("decodeURI Match:", jsDec === cppDec);

    // encodeURIComponent
    const jsEncC = safeRunJS(encodeURIComponent, input);
    const cppEncC = runCpp("encodeURIComponent", input);
    console.log("encodeURIComponent Match:", jsEncC === cppEncC);

    // decodeURIComponent
    const jsDecC = safeRunJS(decodeURIComponent, jsEncC.error ? input : jsEncC);
    const cppDecC = runCpp("decodeURIComponent", jsEncC.error ? input : jsEncC);
    if (typeof jsDecC === "object" && jsDecC.error) {
        console.log("[JS decodeURIComponent ERROR]", jsDecC.message);
    }
    if (typeof cppDecC === "object" && cppDecC.error) {
        console.log("[C++ decodeURIComponent ERROR]", cppDecC.message);
    }
    console.log("decodeURIComponent Match:", jsDecC === cppDecC);
    console.log();
}

// 網羅的テストケース
const testCases = [
    "", // 空文字
    "simpleASCII123", // 英数字
    "hello world", // スペース
    "a+b=c&d=e", // + や記号
    "https://example.com/こんにちは?foo=bar&baz=qux", // 日本語 URL
    "スペースと記号!@#$%^&*()", // 記号混在
    "emoji: 😃🚀", // 絵文字
    "multi byte: 漢字カタカナひらがな", // 日本語マルチバイト
    "reserved;/?:@&=+$,", // 予約文字
    "unsafe<>#%{}|\\^~[]`", // 非推奨文字
    "percent%25sign", // % を含む
    "foo%20bar", // すでにエンコード済み
    "foo%ZZbar", // 不正なエンコード
    "control\u0000\u0001\u0002", // 制御文字
    "newline\ncarriage\rreturn", // 改行
    "surrogate pair: \uD83D\uDE00", // サロゲートペア
];

for (const input of testCases) {
    testCase(input);
}
