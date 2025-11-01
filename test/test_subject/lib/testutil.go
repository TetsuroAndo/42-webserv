package testutil

import (
    "bytes"
    "context"
    "fmt"
    "io"
    "io/ioutil"
    "net"
    "os"
    "os/exec"
    "path/filepath"
    "regexp"
    "sync"
    "syscall"
    "testing"
    "time"
)

var (
    // 実行時のカレントディレクトリ（test_subject/test_req_1）からの相対パス
    webservPath = filepath.Join("../../..", "webserv") // プロジェクトルート直下のバイナリ
    configsDir  = filepath.Join("../..", "confs")       // test ディレクトリ配下の設定群
    defaultHost = "127.0.0.1"
    once        sync.Once
)

// TestMain: セッションスコープで1回だけmakeを実行
func BuildWebserv(t *testing.T) {
    once.Do(func() {
        if _, err := os.Stat(webservPath); err == nil {
            return
        }
        cmd := exec.Command("make")
        cmd.Dir = filepath.Dir(webservPath)
        out, err := cmd.CombinedOutput()
        if err != nil {
            t.Fatalf("Failed to build webserv: %v\nOutput:\n%s", err, out)
        }
        if _, err := os.Stat(webservPath); err != nil {
            t.Fatalf("Webserv binary not found at %s", webservPath)
        }
    })
}

// findFreePort: 利用可能なポートを探す
func FindFreePort() int {
	l, _ := net.Listen("tcp", "127.0.0.1:0")
	defer l.Close()
	return l.Addr().(*net.TCPAddr).Port
}

// parseConfigPort: YAMLからportを抽出
func ParseConfigPort(path string) int {
    data, err := os.ReadFile(path)
    if err != nil {
        return 0
    }
    // シンプルにportを探す
    r, _ := regexp.Compile(`port:\s*(\d+)`)
    m := r.FindSubmatch(data)
    if len(m) > 1 {
        port := 0
		fmt.Sscanf(string(m[1]), "%d", &port)
		return port
	}
	return 0
}

// createTempConfigWithPort: YAMLを一時ファイルに書き換え
func CreateTempConfigWithPort(originalPath string, port int) string {
	data, _ := os.ReadFile(originalPath)
	re := regexp.MustCompile(`(port:\s*)\d+`)
	modified := re.ReplaceAllString(string(data), fmt.Sprintf("${1}%d", port))
	tmpfile, _ := os.CreateTemp("", "test_config_*.yaml")
	ioutil.WriteFile(tmpfile.Name(), []byte(modified), 0644)
	return tmpfile.Name()
}

func ManagedServer(t *testing.T, configName string) (baseURL string, cleanup func()) {
	BuildWebserv(t)

	// 設定ファイルの解決
	configPath := filepath.Join(configsDir, configName)
	if _, err := os.Stat(configPath); err != nil {
		t.Fatalf("config not found: %s", configPath)
	}

	port := ParseConfigPort(configPath)
	if port == 0 {
		port = FindFreePort()
	}

	tempPath := CreateTempConfigWithPort(configPath, port)
	ctx, cancel := context.WithCancel(context.Background())
	cmd := exec.CommandContext(ctx, webservPath, tempPath)
	cmd.SysProcAttr = &syscall.SysProcAttr{Setpgid: true}

	var stderr bytes.Buffer
	cmd.Stderr = &stderr
	if err := cmd.Start(); err != nil {
		t.Fatalf("failed to start webserv: %v", err)
	}

	// ポートリッスン待機
	ok := false
	for i := 0; i < 50; i++ {
		conn, err := net.DialTimeout("tcp", fmt.Sprintf("%s:%d", defaultHost, port), 100*time.Millisecond)
		if err == nil {
			conn.Close()
			ok = true
			break
		}
		time.Sleep(100 * time.Millisecond)
	}
	if !ok {
		cancel()
		out, _ := io.ReadAll(&stderr)
		t.Fatalf("webserv did not start: %s", out)
	}

	baseURL = fmt.Sprintf("http://%s:%d", defaultHost, port)
	cleanup = func() {
		cancel()
		syscall.Kill(-cmd.Process.Pid, syscall.SIGKILL)
		os.Remove(tempPath)
	}
	return
}
