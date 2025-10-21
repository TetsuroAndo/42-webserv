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

	bool isCgi = false;
	for (std::map< std::string, std::string >::const_iterator it =
			 loc.cgiConf.begin();
		 it != loc.cgiConf.end(); ++it) {
		const std::string &ext = it->first;
		if (path.size() >= ext.size() &&
			path.compare(path.size() - ext.size(), ext.size(), ext) == 0) {
			isCgi = true;
			break;
		}
	}

	if (isCgi) {
		std::auto_ptr< ISubHandler > handler(new CgiHandler(_cgiManager));
		try {
			ctx.res = handler->handle(ctx);
		} catch (const std::exception &e) {
			ctx.res.statusCode = HttpStatus::INTERNAL_SERVER_ERROR;
		}
		return;
	}

	next->next(ctx);
}
