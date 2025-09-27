# How to Use Logger

このドキュメントは、C++98 Loggerライブラリをプロジェクトに導入し、基本的なログ出力を行うための手順を解説します。

## 1\. 準備

### ヘッダーのインクルード

ロガー機能を使用するには、まずメインのヘッダーファイルをインクルードします。

```cpp
#include "Logger.hpp"
```

#### Loggerインスタンスの取得

このライブラリはシングルトンとして設計されています。以下のコードで、アプリケーション全体で共有される唯一のインスタンスへの参照を取得します。

```cpp
Logger &logger = Logger::getInstance();
```

以降の操作は、すべてこの `logger` 変数を通じて行います。

-----

## 2\. Loggerの設定方法

ログを実際にどこに、どのような形式で、どのレベルから出力するかを設定します。設定はいくつでも追加できます。

### 設定できる内容

- ログレベル
    - 5段階のログレベル
    - ログレベルフィルタ
- コンソール出力(標準出力)
- ファイルへの出力
    - PATH
    - ファイル名
    - ファイルサイズの制限
    - バックアップファイル数

### 5段階のログレベル

ログレベルはこの5段階です。

1. `DEBUG`
2. `INFO`
3. `WARNING`
4. `ERROR`
5. `FATAL`

出力先（Sink）ごとに、どのレベルのログを出力するかを**フィルタモード**で制御できます。

### ログレベルフィルタ

#### `GREATER_OR_EQUAL` モード (指定レベル以上)

指定したレベルとそのレベルより重大なログをすべて出力します。
これは**デフォルト**の動作です。

**例**: `INFO` レベル以上に設定すると、`INFO`, `WARNING`, `ERROR`, `FATAL` のログが出力される。

```cpp
// 例: 省略した場合はGREATER_OR_EQUALになる。
logger.setSinkConsole(ELF, INFO, GREATER_OR_EQUAL);
logger.setSinkFile("server.log", JSON, INFO, GREATER_OR_EQUAL);
```

#### `EXACT` モード (完全一致)

指定したレベルと完全に一致するログのみを出力します。

**例**: `WARNING` レベルのみを `WarningOnly.log` に出力する。

```cpp
logger.setSinkConsole(ELF, WARNING, EXACT);
logger.setSinkFile("WarningOnly.log", ELF, WARNING, EXACT);
```

この例では、出力されるのはWARNINGのみ

---

### コンソールへの出力設定

`setSinkConsole()` を使用して、標準出力（コンソール）にログを出力するよう設定します。引数には**フォーマット**と**ログレベル
**を指定します。

**例**: コンソールに `JSON` 形式で `DEBUG` レベル以上のログを出力する。

```cpp
logger.setSinkConsole(JSON, DEBUG);
logger.setSinkConsole(JSON, DEBUG, EXACT);
```

---

### ファイルへの出力設定

`setSinkFile()` を使用して、ファイルにログを出力します。出力先のパスやログローテーション（世代管理）も設定可能です。

#### PATHとファイル名の設定

`setLogDir()` でログを保存するデフォルトのディレクトリを指定できます。指定しない場合は `./log` ディレクトリが使用されます。

**例**: `./logs` ディレクトリに `server.log` という名前でログファイルを作成する。

```cpp
// ログファイルを保存するデフォルトディレクトリを指定
logger.setLogDir("./logs");

// ファイルシンクを設定 (フォーマット: JSON, レベル: INFO以上)
logger.setSinkFile("server.log", JSON, INFO);
```

特別にSinkごとにディレクトリを指定することも可能。

**例**: `/var/out-logs` ディレクトリに `special.log` という名前でログファイルを作成する。

```cpp
logger.setSinkFile("/var/out-logs", "special.log", JSON, DEBUG);
```

#### ファイルサイズとバックアップ数の設定 (ログローテーション)

ファイルサイズが上限に達した際に、自動でファイルをバックアップし、新しいファイルに切り替えられます。`setSinkFile()` の追加引数で
**最大ファイルサイズ (バイト)** と**最大バックアップファイル数**を指定します。

**例**: `Max.log` は最大 **100バイト** に制限し、バックアップは **3世代** まで保持する。

```cpp
// setSinkFile(ファイル名, フォーマット, レベル, モード, 最大ファイルサイズ, 最大バックアップ数)
logger.setSinkFile("Max.log", ELF, DEBUG, GREATER_OR_EQUAL, 100, 3);
```

この設定では、`Max.log` が100バイトを超えると `MaxTest.log.1` にリネームされ、新しい `MaxTest.log`
が作成されます。古いバックアップは順次繰り上げられ (`.1`→`.2`)、最大数を超えるもの (`.3`より古いもの) は削除されます。

デフォルトでは

- 最大ファイルサイズ: `10MB`
- バックアップファイル数: `8`

-----

## 3\. ログの記録方法

`LOG(level)` マクロと `<<` 演算子を使用して、`std::cout` のような直感的なスタイルでログを記録します。

### 基本的なメッセージ

```cpp
LOG(INFO) << "Server is starting...";
LOG(WARNING) << "Configuration file has a deprecated option.";
```

### 変数や値を含める

文字列だけでなく、数値や `std::string` 型の変数なども直接ストリームに渡せます。

```cpp
int port = 8080;
std::string host = "localhost";
LOG(INFO) << "Server listening on " << host << ":" << port;
```

### 構造化属性の追加

`attr("キー", 値)` ヘルパー関数を使うことで、ログに構造化されたデータを追加できます。これは特に `JSON` フォーマットで効果的です。

```cpp
std::string clientIp = "127.0.0.1";
int clientFd = 5;

LOG(INFO) << "Accepted new connection"
          << attr("client_ip", clientIp)
          << attr("fd", clientFd);

LOG(ERROR) << "Failed to process request"
           << attr("status_code", 404)
           << attr("reason", "File not found");
```

上記のログは、`JSON` フォーマットの場合、`attributes` フィールドに `{"client_ip": "127.0.0.1", "fd": "5"}`
のような情報を含んで出力されます。

-----

## 4\. 総合的なサンプルコード

以下は、これまでに説明した機能を組み合わせた完全なサンプルコードです。

```cpp
#include "Logger.hpp"
#include <iostream>
#include <stdexcept>

int main() {
    try {
        // 1. Loggerインスタンスを取得
        Logger &logger = Logger::getInstance();

        // 2. ログファイルのデフォルトの保存先ディレクトリを設定
        logger.setLogDir("./logs");

        // 3. 出力先 (シンク) を複数設定
        //    - コンソールにはJSON形式でDEBUGレベル以上を出力
        logger.setSinkConsole(JSON, DEBUG);
        //    - server.logにはJSON形式でINFOレベル以上を出力
        logger.setSinkFile("server.log", JSON, INFO);
        //    - WarningOnly.logにはELF形式でWARNINGレベルのみを出力
        logger.setSinkFile("WarningOnly.log", ELF, WARNING, EXACT);
        //    - MaxTest.logは100バイトでローテーションする
        logger.setSinkFile("MaxTest.log", ELF, DEBUG, GREATER_OR_EQUAL, 100, 3);


        // 4. ログを記録
        LOG(DEBUG) << "This is a debug message. It will appear on console and MaxTest.log";

        LOG(INFO) << "Server is starting...";

        std::string clientIp = "127.0.0.1";
        int clientFd = 5;
        LOG(INFO) << "Accepted new connection"
                  << attr("client_ip", clientIp)
                  << attr("fd", clientFd);

        LOG(WARNING) << "Configuration file has a deprecated option.";

        LOG(ERROR) << "Failed to process request for resource: /test.html"
                   << attr("status_code", 404)
                   << attr("reason", "File not found");

    } catch (const std::exception &e) {
        // ロガー自体の初期化失敗など、致命的なエラーを捕捉
        std::cerr << "A critical error occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```