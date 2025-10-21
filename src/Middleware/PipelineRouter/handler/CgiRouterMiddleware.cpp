#include "CgiRouterMiddleware.hpp"
#include "../../../Cgi/CgiManager.hpp"
#include "../../../Config/Config.hpp"
#include "../../../Handler/CgiHandler.hpp"
#include "../../../Handler/ISubHandler.hpp"
#include "../../../Http/Core/HttpStatus.hpp"
#include "../../../Lib/Logger/Log.hpp"
#include <memory>

CgiRouterMiddleware::CgiRouterMiddleware(CgiManager *cgiManager)
	: _cgiManager(cgiManager) {}

void CgiRouterMiddleware::handle(PipelineContext &ctx,
								 MiddlewareProcessor *next) {
	const Location &loc = ctx.conf.getLocation(ctx.req.getPath());
	const std::string &path = ctx.req.getPath();

	LOG(DEBUG) << "Checking for CGI" << attr("path", path);

	bool isCgi = false;
	for (std::map< std::string, std::string >::const_iterator it =
			 loc.cgiConf.begin();
		 it != loc.cgiConf.end(); ++it) {
		const std::string &ext = it->first;
		if (path.size() >= ext.size() &&
			path.compare(path.size() - ext.size(), ext.size(), ext) == 0) {
			LOG(DEBUG) << "Path matches CGI configuration" << attr("ext", ext);
			isCgi = true;
			break;
		}
	}

	if (isCgi) {
		LOG(DEBUG) << "Routing to CgiHandler" << attr("path", path);
		std::auto_ptr< ISubHandler > handler(new CgiHandler(_cgiManager));
		try {
			ctx.res = handler->handle(ctx);
		} catch (const std::exception &e) {
			LOG(ERROR) << "CgiHandler threw an exception"
					   << attr("error", e.what()) << attr("path", path);
			ctx.res.statusCode = HttpStatus::INTERNAL_SERVER_ERROR;
		}
		return;
	}

	LOG(DEBUG) << "Path is not CGI, passing to next middleware"
			   << attr("path", path);
	next->next(ctx);
}
