#include "PostHandler.hpp"
#include "../Config/Config.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Lib/Info/App.hpp"
#include "../Lib/Logger/Log.hpp"
#include "HandlerUtil.hpp"
#include <cstring>
#include <iostream>

PostHandler::PostHandler()
{
}

PostHandler::~PostHandler()
{
}

HttpResponse PostHandler::handle(const HttpRequest& req, const Config& config)
{
	HttpResponse response(SERVER_NAME);
	LOG(INFO) << "PostHandler processing request"
			  << attr("method", req.getMethod())
			  << attr("uri", req.getPath());

	const std::string filePath = HandlerUtil::resolvePath(req.getPath(), config);
	//工事現場はこちらです。
	// std::cout << "Called!: \n"
	// <<  "    req head : " << (req.getHeader("Content-Type").empty() ? "empty": req.getHeader("Content-Type")) << std::endl
	// << "    req body : " << (req.getBody().empty() ? 0: req.getBody().size()) << std::endl;

	// TODO:POST METHODの処理を書く
	// リクエストのContent Typeで処理をswitchするのが良さげ？
	// とりあえず以下のような入力をパースできてる。
	// curl -X POST -H "Content-Type: application/xml" -d '<person><name>太郎</name><age>30</age></person>' 0.0.0.0:8080
	// これもできてた(tmp/img.pngは任意の画像に置き換えて)
	//  curl -v -i 0.0.0.0:8080/uploads -H "Expect:" -F 'file=@./tmp/img.png'
	return response;
}
