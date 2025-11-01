package test_req_1

import (
	"net/http"
	lib "test_subject/lib"
	"testing"
)

func TestGetMethod(t *testing.T) {
	baseURL, cleanup := lib.ManagedServer(t, "subject_test/get_basic.yaml")
	defer cleanup()

	resp, err := http.Get(baseURL + "/")
	if err != nil {
		t.Fatalf("failed request: %v", err)
	}
	except := 200
	if resp.StatusCode != except {
		t.Fatalf("failed with status %v, excepted %v", resp.StatusCode, except)
	}
}
