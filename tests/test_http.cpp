#include "../fatdog/http/http.h"
#include "../fatdog/log.h"

void test_request()
{
    fatdog::http::HttpRequest::ptr req(new fatdog::http::HttpRequest);
    req->setHeader("host", "www.baidu.top");
    req->setBody("hello fatdog");
    req->dump(std::cout) << std::endl;
}

void test_response()
{
    fatdog::http::HttpResponse::ptr rsp(new fatdog::http::HttpResponse);
    rsp->setHeader("X-X", "fatdog");
    rsp->setBody("hello fatdog");
    rsp->setStatus((fatdog::http::HttpStatus)400);
    rsp->setClose(false);

    rsp->dump(std::cout) << std::endl;
}

int main(int argc, char **argv)
{
    test_request();
    test_response();
    return 0;
}
