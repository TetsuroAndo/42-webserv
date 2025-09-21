#include "PipelineContext.hpp"

PipelineContext::PipelineContext(HttpRequest &r, HttpResponse &s, const Config &c)
	: req(r), res(s), conf(c), session(NULL) {}
