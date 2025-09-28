## C++98 Logger 詳細仕様書

### 1. 概要

本ライブラリは、C++98標準に準拠した、柔軟性と拡張性に優れたシングルトンベースのロギング機能を提供するものです。複数の出力先（シンク）と出力形式（フォーマッタ）を動的に組み合わせることで、ログレベルによるフィルタリング、構造化ロギング、ログローテーションなどの高度な機能をサポートします。

**主な特徴**:

* **言語準拠**: `C++98` に完全準拠。
* **デザインパターン**: **シングルトン**パターンによる単一の `Logger` インスタンス管理。**RAII (Resource Acquisition Is
  Initialization)** パターンを利用した `LogBuilder` による安全で直感的なログ記録。
* **モジュール性**: ログの**出力先 (Sink)** と**書式 (Form)** が分離しており、高い拡張性を持ちます。
* **柔軟な設定**: 出力先ごとにログレベルとフィルタリングモードを個別に設定可能。
* **構造化**: `attr("key", value)` 形式で、ログに機械処理可能な属性情報を付与できます。
* **ファイル管理**: ファイルサイズとバックアップ数に基づく自動ログローテーション機能を内蔵。

***

### 2. 設計と主要概念

本ライブラリは、以下のコンポーネント間の協調によって動作します。

#### **`Logger` (中央管理クラス)**

アプリケーション全体で唯一のインスタンスとして存在するシングルトンクラスです。すべてのシンク（`LogSink`）オブジェクトをコンテナで保持し、
`log()` メソッドを通じて外部から渡されたログメッセージ（`LogMessage`）を、各シンクのフィルタリング条件に基づいて適切に振り分ける責務を持ちます。

#### **`LogSink` (出力先)**

ログの出力先を抽象化したインターフェース（抽象基底クラス）です。`ConsoleSink`（コンソール出力）と`FileSink`
（ファイル出力）という2つの具象クラスが提供されます。各シンクは以下の3つの要素を保持します。

1. **フォーマッタ (`LogForm*`)**: ログメッセージを文字列に変換するオブジェクト。
2. **ログレベル (`LogLevel`)**: フィルタリングの基準となるログレベル。
3. **フィルターモード (`LogFilterMode`)**: ログレベルの比較方法。

#### **`LogForm` (書式)**

ログメッセージの書式を定義するインターフェース（抽象基底クラス）です。`JsonForm` と `ElfForm` の2つの具象クラスが提供され、
`LogMessage` オブジェクトを受け取り、指定された書式で `std::ostream` に書き出します。

#### **`LOG()` マクロと `LogBuilder` (RAIIによるログ記録)**

ログ記録のインターフェースは `LOG(level)` マクロによって提供されます。このマクロは、スタック上に一時的な `LogBuilder`
オブジェクトを生成します。利用者は `<<` 演算子を用いてメッセージや属性をこのオブジェクトにストリーミングします。
この式が完了し、`LogBuilder` オブジェクトがスコープを抜ける際、その**デストラクタ**が自動的に呼び出されます。デストラクタは、ストリームされた内容から最終的な
`LogMessage` を構築し、`Logger::getInstance().log()` を呼び出してメッセージを中央ロガーに渡します。このRAIIパターンにより、リソース管理が自動化され、簡潔な構文が実現されています。

#### **`LogMessage` (データ構造)**

ログ一件分の全ての情報（タイムスタンプ、レベル、メッセージ、ソース位置、属性）を保持する構造体です。`LogBuilder` によって生成され、
`Logger` を経由して各 `LogSink` および `LogForm` へと値渡しで渡されます。

***

### 3. 主要なクラスと構造体

#### **`struct LogMessage`**

ログ情報を格納するデータ転送用構造体です。
| メンバ変数 | 型 | 説明 |
| ------------------ | ------------------------------------ | ---------------------------------------- |
| `timestamp`        | `time_t`                             | ログのタイムスタンプ (Unix時間)          |
| `level`            | `LogLevel`                           | ログレベル |
| `message`          | `std::string`                        | ログの本文 |
| `file`             | `const char*`                        | ログが出力されたソースファイル名 |
| `line`             | `int`                                | ログが出力されたソースファイルの行番号 |
| `function`         | `const char*`                        | ログが出力された関数名 |
| `attributes`       | `std::map<std::string, std::string>` | キーと値のペアで構成される属性情報 |

#### **`class Logger`**

シングルトンで実装されたロギングシステムの中核です。
| 主要メソッド | 説明 |
| -------------------------------------------------------------- | ------------------------------------------------------------------------ |
| `static Logger &getInstance()`                                 | `Logger` のシングルトンインスタンスを返します。 |
| `void setLogDir(const std::string &)`                          |
ログファイルのデフォルト保存ディレクトリを設定します。 |
| `void setSinkConsole(...)`                                     | コンソール出力シンクを設定・追加します。 |
| `void setSinkFile(...)`                                        | ファイル出力シンクを設定・追加します。 |
| `void log(const LogMessage &)`                                 | `LogBuilder` から `LogMessage`
を受け取り、管理下の全シンクに配布します。 |
| `bool isLogLevelActive(LogLevel)`                              |
指定されたログレベルが現在アクティブかどうかを高速に判定します。 |

#### **`class LogSink` (抽象基底クラス)**

| 主要メソッド                                    | 説明                             |
|-------------------------------------------|--------------------------------|
| `virtual void log(const LogMessage&) = 0` | ログメッセージを受け取り、実際に出力処理を行う純粋仮想関数。 |
| `LogLevel getLogLevel() const`            | このシンクに設定されているログレベルを返します。       |
| `LogFilterMode getFilterMode() const`     | このシンクに設定されているフィルターモードを返します。    |

#### **`class FileSink`**

ファイルへのログ出力を担当します。ログローテーション機能も実装しています。
| メンバ変数 | 型 | 説明 |
| ------------------- | ----------------- | -------------------------------------------- |
| `_dir`              | `std::string`     | ログファイルのディレクトリ |
| `_fileName`         | `std::string`     | ログファイルのベース名 |
| `_fileStream`       | `std::ofstream`   | ファイル書き込み用のストリーム |
| `_maxFileSize`      | `size_t`          | ログローテーションを発動させる最大ファイルサイズ |
| `_maxBackupFiles`   | `size_t`          | 保持するバックアップファイルの最大数 |

#### **`class LogForm` (抽象基底クラス)**

| 主要メソッド                                                      | 説明                                          |
|-------------------------------------------------------------|---------------------------------------------|
| `virtual void format(const LogMessage&, std::ostream&) = 0` | `LogMessage` を指定の書式で `ostream` に書き出す純粋仮想関数。 |
| `std::string levelToString(LogLevel) const`                 | `LogLevel` 列挙型を文字列表現に変換します。                 |

***

### 4. APIリファレンス

#### **グローバルなマクロと関数**

| 名称                                             | 説明                                                                                           |
|------------------------------------------------|----------------------------------------------------------------------------------------------|
| `LOG(level)`                                   | ログ記録を開始するためのマクロ。`LogLevel`型の引数を取ります。返り値は `LogBuilder` の一時オブジェクトであり、`<<` 演算子でメッセージや属性を連結できます。 |
| `attr(const std::string &key, const T &value)` | 構造化ログ用の属性（キーと値のペア）を作成するテンプレート関数。`LOG()` のストリームに `<<` で渡して使用します。                              |

#### **`Logger` クラスの公開メソッド**

* `static Logger& getInstance()`
* `void setLogDir(const std::string& logDir)`
* `void setSinkConsole(LogFormat eFormat, LogLevel level, LogFilterMode mode = GREATER_OR_EQUAL)`
*

`void setSinkFile(const std::string& filename, LogFormat eFormat, LogLevel level, LogFilterMode mode = GREATER_OR_EQUAL, size_t maxFileSize = _LOG_MAX_FILE_SIZE, size_t maxBackupFiles = _LOG_MAX_BACKUPS)`

*

`void setSinkFile(const std::string& logDir, const std::string& filename, LogFormat eFormat, LogLevel level, LogFilterMode mode = GREATER_OR_EQUAL, size_t maxFileSize = _LOG_MAX_FILE_SIZE, size_t maxBackupFiles = _LOG_MAX_BACKUPS)`

***

### 5. 設定項目と列挙型

#### **`enum LogFormat`**

ログの出力フォーマットを指定します。

* `JSON`: JSON形式
* `ELF`: 拡張ログフォーマット (テキストベース)

#### **`enum LogLevel`**

ログの重要度を示すレベルです。深刻度が低い順に定義されています。

* `DEBUG`
* `INFO`
* `WARNING`
* `ERROR`
* `FATAL`

#### **`enum LogFilterMode`**

シンクごとに出力するログを絞り込む際の比較方法です。

* `GREATER_OR_EQUAL`: 設定レベル以上のログを出力 (例: `WARNING` を指定すると `WARNING`, `ERROR`, `FATAL` が対象)
* `EXACT`: 設定レベルと完全に一致するログのみ出力 (例: `WARNING` を指定すると `WARNING` のみ対象)

***

### 6. ログフォーマット仕様

#### **`JSON` フォーマット**

各ログエントリは、改行で区切られた単一のJSONオブジェクトとして出力されます。
| キー | 値の型 | 説明 |
| ------------- | ------ | ----------------------------------------------------- |
| `timestamp`   | string | `YYYY-MM-DDTHH:MM:SS` 形式のタイムスタンプ |
| `level`       | string | ログレベルの文字列 (例: "INFO")                       |
| `message`     | string | ログの本文。`"` は `\"` にエスケープされます。 |
| `source`      | string | `ファイル名:行番号` の形式 |
| `function`    | string | ログが出力された関数名 |
| `attributes`  | object | `attr()` で追加されたキーと値のペアからなるJSONオブジェクト |

#### **`ELF` (Extended Log Format) フォーマット**

スペース区切りのテキスト形式です。ファイルの先頭には `#Fields` から始まるヘッダ行が一度だけ書き込まれます。

1. **ヘッダ**: `#Fields: date time level function file:line message attributes`
2. **データ行**: ヘッダの順序に対応した値がスペース区切りで出力されます。
    * `message` と `attributes` 内の空白文字（スペース、タブ、改行）は `-` に置換（サニタイズ）されます。
    * `attributes` は `key1=value1;key2=value2` の形式で連結されます。属性がない場合は `-` が出力されます。

