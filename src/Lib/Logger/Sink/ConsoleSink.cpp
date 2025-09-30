#include "ConsoleSink.hpp"
#include "../Form/JsonForm.hpp"
#include <iostream>

ConsoleSink::ConsoleSink(LogForm *form)
	: LogSink(form) {}

void ConsoleSink::log(const LogMessage &msg) { _form->format(msg, std::cout); }
void ConsoleSink::logAccess(const AccessLogContext& ctx) { _form->formatAccess(ctx, std::cout); }