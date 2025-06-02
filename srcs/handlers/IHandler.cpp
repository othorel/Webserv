

class IHandler
{
    protected :
    
        virtual HttpResponse handle(const HttpRequest &, const Route &) = 0;
}