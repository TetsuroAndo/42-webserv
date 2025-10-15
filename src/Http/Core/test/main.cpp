#include "HttpRequestTest.hpp"
#include "HttpResponseTest.hpp"
#include "HttpStatusTest.hpp"
#include <iostream>

int main() {
	std::cout << "Running HttpRequest tests...\n";
	registerHttpRequestTests();

	std::cout << "\nRunning HttpResponse tests...\n";
	registerHttpResponseTests();

	std::cout << "\nRunning HttpStatus tests...\n";
	runHttpStatusTests();

	return 0;
}
